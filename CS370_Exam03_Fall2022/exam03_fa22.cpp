// CS370 - Exam03
// Fall 2022

/******************************************/
/*       INSERT (a) CODE HERE             */
/******************************************/
// Your name
// Michael Geyer

#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"	// Sean Barrett's image loader - http://nothings.org/
#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "lighting.h"
#include "MatNode.h"
#include "TexNode.h"
#include "exam03.h"

#define DEG2RAD (M_PI/180.0)
#define NUM_MODES 7

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, Cylinder, Cone, Octahedron, Circle, Carpet, Background, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {GreenPlastic, WhitePlastic, BlackCoal, RedAcrylic};
enum Textures {Snow, Tree, Santa, Skirt, NumTextures};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint LightBuffers[NumLightBuffers];
GLuint MaterialBuffers[NumMaterialBuffers];
GLuint TextureIDs[NumTextures];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint colCoords = 4;

// Model files
vector<const char *> objFiles = {"../models/unitcube.obj", "../models/cylinder.obj", "../models/cone.obj", "../models/octahedron.obj", "../models/circle.obj"};

// Texture files
vector<const char *> texFiles = {"../textures/snow.png", "../textures/tree.png", "../textures/santa.png", "../textures/skirt.jpg"};

// Camera
vec3 eye = {4.0f, 4.0f, 4.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};

// Shader variables
// Lighting shader program reference
GLuint lighting_program;
GLuint lighting_vPos;
GLuint lighting_vNorm;
GLuint lighting_camera_mat_loc;
GLuint lighting_model_mat_loc;
GLuint lighting_proj_mat_loc;
GLuint lighting_norm_mat_loc;
GLuint lighting_lights_block_idx;
GLuint lighting_materials_block_idx;
GLuint lighting_material_loc;
GLuint lighting_num_lights_loc;
GLuint lighting_light_on_loc;
GLuint lighting_eye_loc;
const char *lighting_vertex_shader = "../lighting.vert";
const char *lighting_frag_shader = "../lighting.frag";

// Texture shader program reference
GLuint texture_program;
GLuint texture_vPos;
GLuint texture_vTex;
GLuint texture_proj_mat_loc;
GLuint texture_camera_mat_loc;
GLuint texture_model_mat_loc;
const char *texture_vertex_shader = "../texture.vert";
const char *texture_frag_shader = "../texture.frag";

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 normal_matrix;
mat4 model_matrix;

// Global light and material variables
vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights = 0;
GLint lightOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Scene graph nodes
TexNode tree;
MatNode star;
MatNode base;
TexNode skirt;
MatNode present;
MatNode coal;

// Rotation angles
GLdouble elTime = 0.0;
GLdouble flashTime = 0.0;
GLfloat theta = 0.0f;
GLfloat rpm = 2.5f;

// Animation variables
GLboolean animate = false;

// Global spherical coord values
GLfloat azimuth = -45.0f;
GLfloat daz = 2.0f;
GLfloat elevation = 65.0f;
GLfloat del = 2.0f;
GLfloat radius = 1.4641f;

// Global screen dimensions
GLint ww,hh;
int mode = 0;

void display( );
void render_scene( );
void build_geometry( );
void build_materials( );
void build_lights( );
void build_textures();
void build_scene_graph( );
void build_lighting_node(MatNode& node, GLuint obj, GLuint material, GLboolean transparent, mat4 base_trans);
void build_texture_node(TexNode& node, GLuint obj, GLuint texture, mat4 base_trans);
void update_scene_graph(GLdouble dT);
void traverse_scene_graph(BaseNode *node, mat4 baseTransform);
void load_object(GLuint obj);
void build_carpet();
void draw_mat_object(GLuint obj);
void draw_tex_object(GLuint obj, GLuint texture);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);
void build_background();
void draw_background();
void draw_table();
void draw_carpet();

int main(int argc, char**argv)
{
    // Create OpenGL window
    GLFWwindow* window = CreateWindow("CS370 Exam 3");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }
    glfwGetFramebufferSize(window, &ww, &hh);
    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);

    // Load shaders
    // Load light shader
    ShaderInfo lighting_shaders[] = { {GL_VERTEX_SHADER, lighting_vertex_shader},{GL_FRAGMENT_SHADER, lighting_frag_shader},{GL_NONE, NULL} };
    lighting_program = LoadShaders(lighting_shaders);
    lighting_vPos = glGetAttribLocation(lighting_program, "vPosition");
    lighting_vNorm = glGetAttribLocation(lighting_program, "vNormal");
    lighting_proj_mat_loc = glGetUniformLocation(lighting_program, "proj_matrix");
    lighting_camera_mat_loc = glGetUniformLocation(lighting_program, "camera_matrix");
    lighting_norm_mat_loc = glGetUniformLocation(lighting_program, "normal_matrix");
    lighting_model_mat_loc = glGetUniformLocation(lighting_program, "model_matrix");
    lighting_lights_block_idx = glGetUniformBlockIndex(lighting_program, "LightBuffer");
    lighting_materials_block_idx = glGetUniformBlockIndex(lighting_program, "MaterialBuffer");
    lighting_material_loc = glGetUniformLocation(lighting_program, "Material");
    lighting_num_lights_loc = glGetUniformLocation(lighting_program, "NumLights");
    lighting_light_on_loc = glGetUniformLocation(lighting_program, "LightOn");
    lighting_eye_loc = glGetUniformLocation(lighting_program, "EyePosition");

    // Load texture shaders
    ShaderInfo texture_shaders[] = { {GL_VERTEX_SHADER, texture_vertex_shader},{GL_FRAGMENT_SHADER, texture_frag_shader},{GL_NONE, NULL} };
    texture_program = LoadShaders(texture_shaders);
    texture_vPos = glGetAttribLocation(texture_program, "vPosition");
    texture_vTex = glGetAttribLocation(texture_program, "vTexCoord");
    texture_proj_mat_loc = glGetUniformLocation(texture_program, "proj_matrix");
    texture_camera_mat_loc = glGetUniformLocation(texture_program, "camera_matrix");
    texture_model_mat_loc = glGetUniformLocation(texture_program, "model_matrix");

    // Create geometry buffers
    build_geometry();
    // Create material buffers
    build_materials();
    // Create light buffers
    build_lights();
    // Create textures
    build_textures();
    // Create scene graph
    build_scene_graph();

    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Set background color
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Enable blending and set blend factors
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set initial camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye = vec3(x, y, z);

    // Start loop
    while ( !glfwWindowShouldClose( window ) ) {
        // Draw graphics
        display();
        // Update other events like input handling
        glfwPollEvents();
        // Update angle based on time for fixed rpm
        GLdouble curTime = glfwGetTime();
        if (animate) {
            double dT = (curTime-elTime);
            flashTime += dT;
            /******************************************/
            /*       INSERT (j) CODE HERE             */
            /******************************************/
            // TODO: Animate tree, coal, and skirt


            if (mode == 7) {
                /******************************************/
                /*       INSERT (l) CODE HERE (BONUS)     */
                /******************************************/
                // TODO: Flash star
                
                
            }

        }
        elTime = curTime;
        // Swap buffer onto screen
        glfwSwapBuffers( window );
    }

    // Close window
    glfwTerminate();
    return 0;
}

void display( )
{
    proj_matrix = mat4().identity();
    camera_matrix = mat4().identity();

    // Clear window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (mode > 5) {
        draw_background();
    }

    // Set projection matrix
    // Set orthographic viewing volume anisotropic
    GLfloat xratio = 1.0f;
    GLfloat yratio = 1.0f;
    // If taller than wide adjust y
    if (ww <= hh)
    {
        yratio = (GLfloat)hh / (GLfloat)ww;
    }
        // If wider than tall adjust x
    else if (hh <= ww)
    {
        xratio = (GLfloat)ww / (GLfloat)hh;
    }
    proj_matrix = ortho(-10.0f*xratio, 10.0f*xratio, -10.0f*yratio, 10.0f*yratio, -10.0f, 10.0f);

    // Set camera matrix
    camera_matrix = lookat(eye, center, up);

    // Render objects
    render_scene();

    glFlush();
}

void render_scene( ) {
    draw_table();

    if (mode == 1 || mode == 2) {
        /******************************************/
        /*       INSERT (c) CODE HERE             */
        /******************************************/
        // TODO: Traverse scene graph
        traverse_scene_graph(&base, mat4().identity());

    }
    else if (mode > 2) {
        /******************************************/
        /*       INSERT (h) CODE HERE             */
        /******************************************/
        // TODO: Draw carpet then traverse scene graph
        draw_carpet();
        traverse_scene_graph(&base, mat4().identity());

    }
}

void build_scene_graph( ) {
    /******************************************/
    /*       INSERT (b) CODE HERE             */
    /******************************************/
    // TODO: Add base node (lighting)
    build_lighting_node(base, Cylinder, GreenPlastic, false, translate(TREE_X, TREE_Y + BASE_HEIGHT, TREE_Z)*scale(BASE_WIDTH, BASE_HEIGHT, BASE_DEPTH));
//    base.set_update_transform();
    base.attach_nodes(&tree, &skirt);


    /******************************************/
    /*       INSERT (d) CODE HERE             */
    /******************************************/
    // TODO: Add tree node (texture)
    build_texture_node(tree, Cone, Tree, translate(TREE_X, TREE_Y + BASE_HEIGHT + TREE_HEIGHT, TREE_Z)*scale(TREE_WIDTH, TREE_HEIGHT, TREE_DEPTH));
//    tree.set_base_transform();
    tree.attach_nodes(&star, NULL);

    /******************************************/
    /*       INSERT (e) CODE HERE             */
    /******************************************/
    // TODO: Add star node (lighting)
    build_lighting_node(star, Octahedron, WhitePlastic, false, translate(TREE_X, (TREE_Y + BASE_HEIGHT + TREE_HEIGHT + STAR_HEIGHT + 2.0f), TREE_Z)*scale(STAR_WIDTH, STAR_HEIGHT, STAR_DEPTH));
    star.attach_nodes(NULL, NULL);

    /******************************************/
    /*       INSERT (g) CODE HERE             */
    /******************************************/
    // TODO: Add skirt node (texture)
    build_texture_node(skirt, Circle, Skirt, translate(TREE_X, TREE_Y + 0.1f, TREE_Z)*scale(SKIRT_WIDTH, SKIRT_HEIGHT, SKIRT_DEPTH));
    skirt.attach_nodes(NULL, &coal);


    /******************************************/
    /*       INSERT (i) CODE HERE             */
    /******************************************/
    // TODO: Add present with coal (lighting)
    build_lighting_node(coal, Octahedron, BlackCoal, false, translate(PRESENT_X, PRESENT_Y + PRESENT_HEIGHT/2, PRESENT_Z)*scale(COAL_WIDTH, COAL_HEIGHT, COAL_DEPTH));
    coal.attach_nodes(NULL, &present);

    build_lighting_node(present, Cube, RedAcrylic, true, translate(PRESENT_X, PRESENT_Y + PRESENT_HEIGHT/2, PRESENT_Z)*scale(PRESENT_WIDTH, PRESENT_HEIGHT, PRESENT_DEPTH));
    present.attach_nodes(NULL, NULL);
}

void build_geometry( ) {
    // Generate vertex arrays for objects
    glGenVertexArrays(NumVAOs, VAOs);

    // Load objects
    load_object(Cylinder);
    load_object(Cone);
    load_object(Cube);
    load_object(Octahedron);
    load_object(Circle);

    build_background();
    build_carpet();
}

void build_carpet() {
    // Carpet geometry
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;

    vertices = {
            vec4(1.0f, 0.0f, 1.0f, 1.0f),
            vec4(1.0f, 0.0f, -1.0f, 1.0f),
            vec4(-1.0f, 0.0f, -1.0f, 1.0f),
            vec4(-1.0f, 0.0f, -1.0f, 1.0f),
            vec4(-1.0f, 0.0f, 1.0f, 1.0f),
            vec4(1.0f, 0.0f, 1.0f, 1.0f),
    };

    normals = {
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
    };

    /******************************************/
    /*       INSERT (h) CODE HERE             */
    /******************************************/
    // TODO: Add carpet texture coordinates
    uvCoords = {
//            {0.5, 1},
//            {1, 0.5},
//            {0.5, 0},
//            {0.5, 0},
//            {0, 0.5},
//            {0.5, 1}

//            {1, 0.5},
//            {0.5, 0},
//            {0, 0.5},
//            {0, 0.5},
//            {0.5, 1},
//            {1, 0.5}

            {0, 0.5},
            {0.5, 1},
            {1, 0.5},
            {1, 0.5},
            {0.5, 0},
            {0, 0.5}
    };

    // Set number of vertices
    numVertices[Carpet] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[Carpet]);
    glBindVertexArray(VAOs[Carpet]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Carpet][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * posCoords * numVertices[Carpet], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Carpet][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normCoords * numVertices[Carpet], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Carpet][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texCoords * numVertices[Carpet], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void build_background() {
    // Background geometry
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;

    vertices = {
            vec4(1.0f, 1.0f, 0.0f, 1.0f),
            vec4(-1.0f, 1.0f, 0.0f, 1.0f),
            vec4(-1.0f, -1.0f, 0.0f, 1.0f),
            vec4(-1.0f, -1.0f, 0.0f, 1.0f),
            vec4(1.0f, -1.0f, 0.0f, 1.0f),
            vec4(1.0f, 1.0f, 0.0f, 1.0f),
    };

    normals = {
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
    };

    /******************************************/
    /*       INSERT (k) CODE HERE             */
    /******************************************/
    // TODO: Add background texture coordinates
    uvCoords = {
            {1, 1},
            {0, 1},
            {0, 0},
            {0, 0},
            {1, 0},
            {1, 1}
    };

    // Set number of vertices
    numVertices[Background] = vertices.size();

    // Create and load object buffers
    glBindVertexArray(VAOs[Background]);
    glGenBuffers(NumObjBuffers, ObjBuffers[Background]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Background][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[Background], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Background][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[Background], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Background][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texCoords*numVertices[Background], uvCoords.data(), GL_STATIC_DRAW);
}


void draw_background(){
    // Set anisotropic scaling
    GLfloat xratio = 1.0f;
    GLfloat yratio = 1.0f;
    // If taller than wide adjust y
    if (ww <= hh)
    {
        yratio = (GLfloat)hh / (GLfloat)ww;
    }
        // If wider than tall adjust x
    else if (hh <= ww)
    {
        xratio = (GLfloat)ww / (GLfloat)hh;
    }
    // Set default orthographic projection
    proj_matrix = ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    // Set default camera matrix
    camera_matrix = mat4().identity();

    // Set default model matrix
    model_matrix = mat4().identity();

    /******************************************/
    /*       INSERT (k) CODE HERE             */
    /******************************************/
    // TODO: Draw background
    glDepthMask(GL_FALSE);
    draw_tex_object(Background, Snow);
    glDepthMask(GL_TRUE);

}

#include "examfunc.cpp"
