// CS370 Assignment 4 - Walking Man
// Fall 2022

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
#include "player.h"
#define DEG2RAD (M_PI/180.0)

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, Sphere, TexCube, Background, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {Skin, GreenJersey, RedAcrylic};
enum Textures {YCP, Shirt, Face, Basketball, Wood, NumTextures};

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

vector<const char *> objFiles = {"../models/unitcube.obj", "../models/uv_sphere.obj"};
vector<const char *> texFiles = {"../textures/ycp.png", "../textures/shirt_z.png", "../textures/face.png", "../textures/bball.png", "../textures/wood.png"};

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
mat4 model_matrix;

vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights = 0;
GLint lightOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Scene graph nodes
TexNode torso;
TexNode head;
MatNode left_upper_arm;
MatNode right_upper_arm;
TexNode court;
MatNode left_lower_arm;
MatNode right_lower_arm;
MatNode left_upper_leg;
MatNode right_upper_leg;
MatNode left_lower_leg;
MatNode right_lower_leg;
MatNode box_node;
TexNode ball;

// Animation variables
GLboolean animate = false;

// Global spherical coord values
GLfloat azimuth = 45.0f;
GLfloat daz = 2.0f;
GLfloat elevation = 54.7f;
GLfloat del = 2.0f;
GLfloat radius = 5.0f;

// Global screen dimensions
GLint ww,hh;

void display( );
void render_scene( );
void build_geometry( );
void build_texture_cube(GLuint obj);
void build_materials( );
void build_lights( );
void build_textures();
void build_scene_graph( );
void build_lighting_node(MatNode& node, GLuint obj, GLuint material, GLboolean transparent, mat4 base_trans);
void build_texture_node(TexNode& node, GLuint obj, GLuint texture, mat4 base_trans);
void build_background(GLuint obj);
void draw_background();
void draw_tex_object(GLuint obj, GLuint texture);
void traverse_scene_graph(BaseNode *node, mat4 baseTransform);
void load_object(GLuint obj);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Go Spartans 2022!");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // Store initial window size
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
        // TODO: Update scene graph nodes

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

	draw_background();

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
    proj_matrix = ortho(-15.0f*xratio, 15.0f*xratio, -10.0f*yratio, 20.0f*yratio, -50.0f, 50.0f);

    // Set camera matrix
    camera_matrix = lookat(eye, center, up);

    // Render objects
	render_scene();

	glFlush();
}

void render_scene( ) {
    // TODO: Render scene graph
    traverse_scene_graph(&torso, mat4().identity());
}

void build_geometry( )
{
    // Generate vertex arrays for objects
    glGenVertexArrays(NumVAOs, VAOs);

    // Load objects
    load_object(Cube);
    load_object(Sphere);

    // Build texture cube
    build_texture_cube(TexCube);

    // Build background
    build_background(Background);
}

void build_texture_cube(GLuint obj) {
    vector<vec4> vertices;
    vector<vec3> normals;
    vector<vec2> uvCoords;

    // Define 3D vertices for cube
    vertices = {
            vec4(0.5f,  -0.5f, 0.5f, 1.0f),    // front
            vec4(0.5f,  0.5f,  0.5f, 1.0f),
            vec4(-0.5f, 0.5f,  0.5f, 1.0f),
            vec4(-0.5f, 0.5f,  0.5f, 1.0f),
            vec4(-0.5f, -0.5f, 0.5f, 1.0f),
            vec4(0.5f,  -0.5f, 0.5f, 1.0f),
            vec4(-0.5f, -0.5f, -0.5f, 1.0f),   // back
            vec4(-0.5f, 0.5f,  -0.5f, 1.0f),
            vec4(0.5f,  0.5f,  -0.5f, 1.0f),
            vec4(0.5f,  0.5f,  -0.5f, 1.0f),
            vec4(0.5f,  -0.5f, -0.5f, 1.0f),
            vec4(-0.5f, -0.5f, -0.5f, 1.0f),
            vec4(0.5f, -0.5f, -0.5f, 1.0f),   // left
            vec4(0.5f, 0.5f, -0.5f, 1.0f),
            vec4(0.5f, 0.5f,  0.5f, 1.0f),
            vec4(0.5f, 0.5f,  0.5f, 1.0f),
            vec4(0.5f, -0.5f,  0.5f, 1.0f),
            vec4(0.5f, -0.5f, -0.5f, 1.0f),
            vec4(-0.5f,  -0.5f, -0.5f, 1.0f),   // right
            vec4(-0.5f,  -0.5f,  0.5f, 1.0f),
            vec4(-0.5f,  0.5f,  0.5f, 1.0f),
            vec4(-0.5f,  0.5f,  0.5f, 1.0f),
            vec4(-0.5f,  0.5f, -0.5f, 1.0f),
            vec4(-0.5f,  -0.5f, -0.5f, 1.0f),
            vec4(-0.5f, 0.5f,  -0.5f, 1.0f),   // top
            vec4(-0.5f, 0.5f,  0.5f, 1.0f),
            vec4(0.5f,  0.5f,  0.5f, 1.0f),
            vec4(0.5f,  0.5f,  0.5f, 1.0f),
            vec4(0.5f,  0.5f,  -0.5f, 1.0f),
            vec4(-0.5f, 0.5f,  -0.5f, 1.0f),
            vec4(-0.5f, -0.5f, -0.5f, 1.0f),   // bottom
            vec4(0.5f,  -0.5f, -0.5f, 1.0f),
            vec4(0.5f,  -0.5f, 0.5f, 1.0f),
            vec4(0.5f,  -0.5f, 0.5f, 1.0f),
            vec4(-0.5f, -0.5f, 0.5f, 1.0f),
            vec4(-0.5f, -0.5f, -0.5f, 1.0f)
    };

    normals = {
            vec3(0.0f,  0.0f, 1.0f),    // Front
            vec3(0.0f,  0.0f, 1.0f),
            vec3(0.0f,  0.0f, 1.0f),
            vec3(0.0f,  0.0f, 1.0f),
            vec3(0.0f,  0.0f, 1.0f),
            vec3(0.0f,  0.0f, 1.0f),
            vec3(1.0f,  0.0f, -1.0f),   // Back
            vec3(0.0f,  0.0f, -1.0f),
            vec3(0.0f,  0.0f, -1.0f),
            vec3(0.0f,  0.0f, -1.0f),
            vec3(0.0f,  0.0f, -1.0f),
            vec3(0.0f,  0.0f, -1.0f),
            vec3(1.0f,  0.0f, 0.0f),    // Left
            vec3(1.0f,  0.0f, 0.0f),
            vec3(1.0f,  0.0f, 0.0f),
            vec3(1.0f,  0.0f, 0.0f),
            vec3(1.0f,  0.0f, 0.0f),
            vec3(1.0f,  0.0f, 0.0f),
            vec3(-1.0f,  0.0f, 0.0f),    // Right
            vec3(-1.0f,  0.0f, 0.0f),
            vec3(-1.0f,  0.0f, 0.0f),
            vec3(-1.0f,  0.0f, 0.0f),
            vec3(-1.0f,  0.0f, 0.0f),
            vec3(-1.0f,  0.0f, 0.0f),
            vec3(0.0f,  1.0f, 0.0f),     // Top
            vec3(0.0f,  1.0f, 0.0f),
            vec3(0.0f,  1.0f, 0.0f),
            vec3(0.0f,  1.0f, 0.0f),
            vec3(0.0f,  1.0f, 0.0f),
            vec3(0.0f,  1.0f, 0.0f),
            vec3(0.0f,  -1.0f, 0.0f),     // Bottom
            vec3(0.0f,  -1.0f, 0.0f),
            vec3(0.0f,  -1.0f, 0.0f),
            vec3(0.0f,  -1.0f, 0.0f),
            vec3(0.0f,  -1.0f, 0.0f),
            vec3(0.0f,  -1.0f, 0.0f)
    };

    // TODO: Define texture coordinates for torso
    uvCoords = {
            {0.35f, 0.0f},
            {0.35f, 1.0f},
            {0.0f, 1.0f},
            {0.0f, 1.0f},
            {0.0f, 0.0f},
            {0.35f, 0.0f},   // Front

            {0.70f, 0.0f},
            {0.70f, 1.0f},
            {0.35f, 1.0f},
            {0.35f, 1.0f},
            {0.35f, 0.0f},
            {0.70f, 0.0f},   // Back

            {1.0f, 0.12f},
            {1.0f, 1.0f},
            {0.85f, 1.0f},
            {0.85f, 1.0f},
            {0.85f, 0.12f},
            {1.0f, 0.12f},   // Left

            {0.85f, 0.12f},
            {0.85f, 1.0f},
            {0.70f, 1.0f},
            {0.70f, 1.0f},
            {0.0f, 0.12f},
            {0.0f, 0.0f},   // Right TODO

            {},
            {},
            {},
            {},
            {},
            {},                  // Top

            {},
            {},
            {},
            {},
            {},
            {},                 // Bottom
    };

    // Set number of vertices
    numVertices[obj] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texCoords*numVertices[obj], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void build_background(GLuint obj) {
    vector<vec4> vertices;
    vector<vec3> normals;
    vector<vec2> uvCoords;

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

    // TODO: Define texture coordinates for background
    uvCoords = {
            {1.0f, 1.0f},
            {0.0f, 1.0f},
            {0.0f, 0.0f},
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
    };

    numVertices[obj] = vertices.size();

    glBindVertexArray(VAOs[obj]);
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texCoords*numVertices[obj], uvCoords.data(), GL_STATIC_DRAW);
}

void build_materials( ) {
    // Create skin material
    MaterialProperties skin = {
            vec4(0.33f, 0.22f, 0.03f, 1.0f), //ambient
            vec4(0.78f, 0.57f, 0.11f, 1.0f), //diffuse
            vec4(0.99f, 0.91f, 0.81f, 1.0f), //specular
            10.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    // TODO: Add additional materials
    MaterialProperties greenJersey = {
            vec4(0.0f, 0.22f, 0.0f, 1.0f), //ambient
            vec4(0.0f, 0.57f, 0.0f, 1.0f), //diffuse
            vec4(0.0f, 0.91f, 0.0f, 1.0f), //specular
            10.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    // TODO: Add additional materials to Materials vector
    Materials.push_back(skin);
    Materials.push_back(greenJersey);

    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
}

void build_lights( ) {
    // White directional light
    LightProperties whiteDirLight = {
            DIRECTIONAL, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(0.0f, 0.0f, 0.0f, 1.0f),  //position
            vec4(-1.0f, -1.0f, -1.0f, 0.0f), //direction
            0.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    // Green point light
    LightProperties yellowPointLight = {
            POINT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.5f, 0.5f, 0.0f, 1.0f), //diffuse
            vec4(0.7f, 0.7f, 0.0f, 1.0f), //specular
            vec4(10.0f, 10.0f, 10.0f, 1.0f),  //position
            vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
            0.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    //Red spot light
    LightProperties redSpotLight = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.0f, 1.0f, 0.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(0.0f, 6.0f, 4.0f, 1.0f),  //position
            vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
            30.0f,   //cutoff
            30.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    // Add lights to Lights vector
	Lights.push_back(whiteDirLight);
	Lights.push_back(yellowPointLight);
	Lights.push_back(redSpotLight);

    // Set numLights
	numLights = Lights.size();

    // Turn all lights on
    for (int i = 0; i < numLights; i++) {
        lightOn[i] = 1;
    }

    // Create uniform buffer for lights
    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(), GL_STATIC_DRAW);
}

void build_scene_graph( ) {
    // TODO: Add scene graph nodes
    build_texture_node(torso, TexCube, Shirt, scale(TORSO_WIDTH, TORSO_HEIGHT, TORSO_DEPTH));
    torso.set_update_transform(translate(vec3(0.0f, TORSO_HEIGHT, 0.0f)));
    torso.attach_nodes(&head, &court);

    build_texture_node(head, Sphere, Face, scale(HEAD_WIDTH, HEAD_HEIGHT, HEAD_DEPTH));
    head.set_update_transform(translate(vec3(0.0f, TORSO_HEIGHT/2 + HEAD_HEIGHT, 0.0f))*rotate(180.0f, vec3(1.0f, 0.0f, 0.0f)));
    head.attach_nodes(NULL, &left_upper_arm);

    build_lighting_node(left_upper_arm, Cube, GreenJersey, false,  scale(UPPER_ARM_WIDTH, UPPER_ARM_HEIGHT, UPPER_ARM_DEPTH));
    left_upper_arm.set_update_transform(translate((TORSO_WIDTH-UPPER_ARM_WIDTH)+0.25f, (TORSO_HEIGHT-UPPER_ARM_HEIGHT)/2, 0.0f));
    left_upper_arm.attach_nodes(&left_lower_arm, &right_upper_arm);

    build_lighting_node(right_upper_arm, Cube, GreenJersey, false,  scale(UPPER_ARM_WIDTH, UPPER_ARM_HEIGHT, UPPER_ARM_DEPTH));
    right_upper_arm.set_update_transform(translate(-(TORSO_WIDTH-UPPER_ARM_WIDTH)-0.25f, (TORSO_HEIGHT-UPPER_ARM_HEIGHT)/2, 0.0f));
    right_upper_arm.attach_nodes(&right_lower_arm, NULL);

    build_lighting_node(left_lower_arm, Cube, Skin, false, scale(LOWER_ARM_WIDTH, LOWER_ARM_HEIGHT, LOWER_ARM_DEPTH));
    left_lower_arm.set_update_transform(translate(0.0f, 0.0f, 0.0f));
    left_lower_arm.attach_nodes(NULL, NULL);

    build_lighting_node(right_lower_arm, Cube, Skin, false, scale(LOWER_ARM_WIDTH, LOWER_ARM_HEIGHT, LOWER_ARM_DEPTH));
    right_lower_arm.set_update_transform(translate(0.0f, 0.0f, 0.0f));
    right_lower_arm.attach_nodes(NULL, NULL);

    build_lighting_node(left_lower_leg, Cube, Skin, false, scale(LOWER_LEG_WIDTH, LOWER_LEG_HEIGHT, LOWER_LEG_DEPTH));
    

    build_texture_node(court, TexCube, Wood, scale(1.0f, 1.0f, 1.0f));
    court.set_update_transform(translate(0.0f, 0.0f, 0.0f));
    court.attach_nodes(NULL, &box_node);

    build_lighting_node(box_node, Cube, RedAcrylic, true, scale(BOX_WIDTH, BOX_HEIGHT, BOX_DEPTH));
    box_node.set_update_transform(translate(0.0f, 0.0f, 0.0f));
    box_node.attach_nodes(NULL, &ball);

    build_texture_node(ball, Sphere, Basketball, scale(BALL_RADIUS, BALL_RADIUS, BALL_RADIUS));
    ball.set_update_transform(translate(0.0f, 0.0f, 0.0f));
    ball.attach_nodes(NULL, NULL);
}

void draw_background(){
    // TODO: Draw background image
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
    // TODO: Set default orthographic projection
    proj_matrix = ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    // TODO: Set default camera matrix
    camera_matrix = mat4().identity();

    // TODO: Set default model matrix
    model_matrix = mat4().identity();

    // TODO: Draw background with depth buffer writes disabled
    glDepthMask(GL_FALSE);
    draw_tex_object(Background, YCP);
    glDepthMask(GL_TRUE);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Move hexagon with arrow keys
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        animate = !animate;
    }

    // Adjust azimuth
    if (key == GLFW_KEY_A) {
        azimuth += daz;
        if (azimuth > 360.0) {
            azimuth -= 360.0;
        }
    } else if (key == GLFW_KEY_D) {
        azimuth -= daz;
        if (azimuth < 0.0)
        {
            azimuth += 360.0;
        }
    }

    // Adjust elevation angle
    if (key == GLFW_KEY_W)
    {
        elevation += del;
        if (elevation > 180.0)
        {
            elevation = 179.0;
        }
    }
    else if (key == GLFW_KEY_S)
    {
        elevation -= del;
        if (elevation < 0.0)
        {
            elevation = 1.0;
        }
    }

    // Compute updated camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye = vec3(x,y,z);

}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

#include "utilfuncs.cpp"
