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
#include "scene.h"

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, Cylinder, Sphere, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum LightNames {WhiteDirLight, GreenPointLight, RedSpotLight};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {Brass, RedPlastic};
enum Textures {Earth, NumTextures};

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
vector<const char *> objFiles = {"../models/unitcube.obj", "../models/cylinder.obj", "../models/uv_sphere.obj"};

// Texture files
vector<const char *> texFiles = {"../textures/earth.bmp"};

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

// Global light and material variables
vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights = 0;
GLint lightOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Scene graph nodes
MatNode base;
MatNode lower_arm;
MatNode left_upper_arm;
MatNode right_upper_arm;
TexNode earth;

// Global object rotation angles
GLfloat theta = 0.0f;
GLfloat dtheta = 1.0f;
GLfloat phi = 0.0f;
GLfloat dphi = 1.0f;
GLfloat left_psi = 0.0f;
GLfloat right_psi = 0.0f;
GLfloat dpsi = 1.0f;
GLfloat earth_angle = 0.0f;
GLfloat rpm = 10.0f;
GLdouble elTime = 0.0;

// Global screen dimensions
GLint ww,hh;

void display( );
void render_scene( );
void build_geometry( );
void build_materials( );
void build_lights( );
void build_textures();
void build_scene_graph( );
void build_lighting_node(MatNode& node, GLuint obj, GLuint material, GLboolean transparent, mat4 base_trans);
void build_texture_node(TexNode& node, GLuint obj, GLuint texture, mat4 base_trans);
void traverse_scene_graph(BaseNode *node, mat4 baseTransform);
void load_object(GLuint obj);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Robot Graph");
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

	// Start loop
    while ( !glfwWindowShouldClose( window ) ) {
    	// Draw graphics
        display();
        // Update other events like input handling
        glfwPollEvents();
        // Update angle based on time for fixed rpm
        GLdouble curTime = glfwGetTime();
        earth_angle += (curTime-elTime)*(rpm/60.0)*360.0;
        elTime = curTime;
        // TODO: Update earth node transformation
        earth.set_update_transform(translate(vec3(0.0f, 3.0f, 5.0f))*rotate(earth_angle, vec3(0.0f, 1.0f, 0.0f)));
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
    // TODO: Traverse scene graph with base as root
    traverse_scene_graph(&base, mat4().identity());
}

void build_geometry( )
{
    // Generate vertex arrays for objects
    glGenVertexArrays(NumVAOs, VAOs);

    // Load objects (with both normals and texture coords)
    load_object(Cube);
    load_object(Cylinder);
    load_object(Sphere);
}

void build_materials( ) {
    // Create brass material
    MaterialProperties brass = {
            vec4(0.33f, 0.22f, 0.03f, 1.0f), //ambient
            vec4(0.78f, 0.57f, 0.11f, 1.0f), //diffuse
            vec4(0.99f, 0.91f, 0.81f, 1.0f), //specular
            27.8f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    // Create red plastic material
    MaterialProperties redPlastic = {
            vec4(0.3f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.6f, 0.0f, 0.0f, 1.0f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    // Add materials to Materials vector
    Materials.push_back(brass);
    Materials.push_back(redPlastic);

    // Create uniform buffer for materials
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
    LightProperties greenPointLight = {
            POINT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.0f, 1.0f, 0.0f, 1.0f), //diffuse
            vec4(0.0f, 1.0f, 0.0f, 1.0f), //specular
            vec4(3.0f, 3.0f, 3.0f, 1.0f),  //position
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
            vec4(0.0f, 6.0f, 0.0f, 1.0f),  //position
            vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
            30.0f,   //cutoff
            30.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    // Add lights to Lights vector
    Lights.push_back(whiteDirLight);
    Lights.push_back(greenPointLight);
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

void build_textures( ) {
    int w, h, n;
    int force_channels = 4;
    unsigned char *image_data;

    // Create textures and activate unit 0
    glGenTextures( texFiles.size(),  TextureIDs);
    glActiveTexture( GL_TEXTURE0 );

    for (int i = 0; i < texFiles.size(); i++) {
        // Load image from file
        image_data = stbi_load(texFiles[i], &w, &h, &n, force_channels);
        if (!image_data) {
            fprintf(stderr, "ERROR: could not load %s\n", texFiles[i]);
        }
        // NPOT check for power of 2 dimensions
        if ((w & (w - 1)) != 0 || (h & (h - 1)) != 0) {
            fprintf(stderr, "WARNING: texture %s is not power-of-2 dimensions\n",
                    texFiles[i]);
        }

        // Bind current texture id
        glBindTexture(GL_TEXTURE_2D, TextureIDs[i]);
        // Load image data into texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     image_data);
        // Generate mipmaps for texture
        glGenerateMipmap( GL_TEXTURE_2D );
        // Set scaling modes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // Set wrapping modes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Set maximum anisotropic filtering for system
        GLfloat max_aniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
        // set the maximum!
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
    }
}

void build_scene_graph( ) {
	// TODO: Add base lighting node
    build_lighting_node(base, Cylinder, RedPlastic, false, scale(vec3(BASE_RADIUS, BASE_HEIGHT, BASE_RADIUS)));
    base.set_update_transform(rotate(theta, vec3(0.0f, 1.0f, 0.0f)));
    base.attach_nodes(&lower_arm, &earth);

    // TODO: Add lower arm node (child of base)
    build_lighting_node(lower_arm, Cube, Brass, false, translate(vec3(0.0f, LOWER_HEIGHT/2, 0.0f))*scale(vec3(LOWER_WIDTH, LOWER_HEIGHT, LOWER_DEPTH)));
    lower_arm.set_update_transform(translate(vec3(0.0f, BASE_HEIGHT, 0.0f))*rotate(phi, vec3(1.0f, 0.0f, 0.0f)));
    lower_arm.attach_nodes(&left_upper_arm, NULL);

    // TODO: Add left upper arm node (child of lower arm)
    build_lighting_node(left_upper_arm, Cube, RedPlastic, false, translate(vec3(0.0f, UPPER_HEIGHT/2, 0.0f))*scale(vec3(UPPER_WIDTH, UPPER_HEIGHT, UPPER_DEPTH)));
    left_upper_arm.set_update_transform(translate(vec3((LOWER_WIDTH+UPPER_WIDTH)/2, LOWER_HEIGHT, 0.0f))*rotate(left_psi, vec3(1.0f, 0.0f, 0.0f)));
    left_upper_arm.attach_nodes(NULL, &right_upper_arm);

    // TODO: Add right upper arm node (sibling of left upper arm)
    build_lighting_node(right_upper_arm, Cube, RedPlastic, false, translate(vec3(0.0f, UPPER_HEIGHT/2, 0.0f))*scale(vec3(UPPER_WIDTH, UPPER_HEIGHT, UPPER_DEPTH)));
    right_upper_arm.set_update_transform(translate(vec3((-LOWER_WIDTH-UPPER_WIDTH)/2, LOWER_HEIGHT, 0.0f))*rotate(right_psi, vec3(1.0f, 0.0f, 0.0f)));
    right_upper_arm.attach_nodes(NULL, NULL);

    // TODO: Add earth node (sibling of base)
    build_texture_node(earth, Sphere, Earth, scale(1.5f, 1.5f, 1.5f));
    earth.set_update_transform(translate(vec3(0.0f, 3.0f, 5.0f))*rotate(earth_angle,vec3(0.0f, 1.0f, 0.0f)));
    earth.attach_nodes(NULL, NULL);
}

void build_lighting_node(MatNode& node, GLuint obj, GLuint material, GLboolean transparent, mat4 base_trans){
    // Set shader program and matrix references
    node.set_shader(lighting_program, lighting_proj_mat_loc, lighting_camera_mat_loc, lighting_norm_mat_loc, lighting_model_mat_loc);
    // Set object buffers
    node.set_buffers(VAOs[obj], ObjBuffers[obj][PosBuffer], lighting_vPos, posCoords, ObjBuffers[obj][NormBuffer], lighting_vNorm, normCoords, numVertices[obj]);
    // Set material buffers and material
    node.set_materials(MaterialBuffers[MaterialBuffer], lighting_materials_block_idx, Materials.size()*sizeof(MaterialProperties), lighting_material_loc, material, transparent);
    // Set light buffers
    node.set_lights(LightBuffers[LightBuffer], lighting_lights_block_idx, Lights.size()*sizeof(LightProperties), lighting_num_lights_loc, Lights.size(), lighting_light_on_loc, lightOn);
    // Set eye position
    node.set_eye(lighting_eye_loc, eye);
    // Set base transform
    node.set_base_transform(base_trans);
    // Set default update transform and nodes
    node.set_update_transform(mat4().identity());
    node.attach_nodes(NULL, NULL);
}

void build_texture_node(TexNode& node, GLuint obj, GLuint texture, mat4 base_trans){
    // Set shader program and matrix references
    node.set_shader(texture_program, texture_proj_mat_loc, texture_camera_mat_loc, texture_model_mat_loc);
    // Set object buffers
    node.set_buffers(VAOs[obj], ObjBuffers[obj][PosBuffer], texture_vPos, posCoords, ObjBuffers[obj][TexBuffer], texture_vTex, texCoords, numVertices[obj]);
    // Set texture
    node.set_texture(TextureIDs[texture]);
    // Set base transform
    node.set_base_transform(base_trans);
    // Default update transform and nodes
    node.set_update_transform(mat4().identity());
    node.attach_nodes(NULL, NULL);
}


void traverse_scene_graph(BaseNode *node, mat4 baseTransform) {
    // Depth first traversal of child/sibling tree
    mat4 model_matrix;

    // Stop when at bottom of branch
    if (node == NULL) {
        return;
    }

    // Apply local transformation and render
    model_matrix = baseTransform*node->ModelTransform;

    node->draw(proj_matrix, camera_matrix, model_matrix);

    // Recurse vertically if possible (depth-first)
    if (node->Child != NULL) {
        traverse_scene_graph(node->Child, model_matrix);
    }

    // Remove local transformation and recurse horizontal
    if (node->Sibling != NULL) {
        traverse_scene_graph(node->Sibling, baseTransform);
    }
}

void load_object(GLuint obj) {
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;

    // Load model and set number of vertices
    loadOBJ(objFiles[obj], vertices, uvCoords, normals);
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

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // ESC closes window
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    // a d rotate base
    if (key == GLFW_KEY_D)
    {
        theta += dtheta;
        if (theta > 360.0f)
        {
            theta -= 360.0f;
        }
        // TODO: Update base
        base.set_update_transform(rotate(theta, vec3(0.0f, 1.0f, 0.0f)));
    }
    else if (key == GLFW_KEY_A)
    {
        theta -= dtheta;
        if (theta < 0.0f)
        {
            theta += 360.0f;
        }
        // TODO: Update base
        base.set_update_transform(rotate(theta, vec3(0.0f, 1.0f, 0.0f)));
    }

    // w s rotates lower arm
    if (key == GLFW_KEY_S)
    {
        phi += dphi;
        if (phi > 90.0f)
        {
            phi = 90.0f;
        }
        // TODO: Update lower arm
        lower_arm.set_update_transform(translate(vec3(0.0f, BASE_HEIGHT, 0.0f))*rotate(phi, vec3(1.0f, 0.0f, 0.0f)));
    }
    else if (key == GLFW_KEY_W)
    {
        phi -= dphi;
        if (phi < -90.0f)
        {
            phi = -90.0f;
        }
        // TODO: Update lower arm
        lower_arm.set_update_transform(translate(vec3(0.0f, BASE_HEIGHT, 0.0f))*rotate(phi, vec3(1.0f, 0.0f, 0.0f)));
    }

    // m n rotates left upper arm
    if (key == GLFW_KEY_N)
    {
        left_psi += dpsi;
        if (left_psi > 180.0f)
        {
            left_psi = 180.0f;
        }
        // TODO: Update left upper arm
        left_upper_arm.set_update_transform(translate(vec3((LOWER_WIDTH+UPPER_WIDTH)/2, LOWER_HEIGHT, 0.0f))*rotate(left_psi, vec3(1.0f, 0.0f, 0.0f)));
    }
    else if (key == GLFW_KEY_M)
    {
        left_psi -= dpsi;
        if (left_psi < -180.0f)
        {
            left_psi = -180.0f;
        }
        // TODO: Update left upper arm
        left_upper_arm.set_update_transform(translate(vec3((LOWER_WIDTH+UPPER_WIDTH)/2, LOWER_HEIGHT, 0.0f))*rotate(left_psi, vec3(1.0f, 0.0f, 0.0f)));
    }

    // . , rotates right upper arm
    if (key == GLFW_KEY_COMMA)
    {
        right_psi += dpsi;
        if (right_psi > 180.0f)
        {
            right_psi = 180.0f;
        }
        // TODO: Update right upper arm
        right_upper_arm.set_update_transform(translate(vec3(-(LOWER_WIDTH+UPPER_WIDTH)/2, LOWER_HEIGHT, 0.0f))*rotate(right_psi, vec3(1.0f, 0.0f, 0.0f)));
    }
    else if (key == GLFW_KEY_PERIOD)
    {
        right_psi -= dpsi;
        if (right_psi < -180.0f)
        {
            right_psi = -180.0f;
        }
        // TODO: Update right upper arm
        right_upper_arm.set_update_transform(translate(vec3(-(LOWER_WIDTH+UPPER_WIDTH)/2, LOWER_HEIGHT, 0.0f))*rotate(right_psi, vec3(1.0f, 0.0f, 0.0f)));,
    }

    // Space toggles spotlight
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        lightOn[RedSpotLight] = (lightOn[RedSpotLight]+1)%2;
    }
}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);

    ww = width;
    hh = height;
}


