#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "lighting.h"

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, Sphere, NumVAOs};
enum Obj_Buffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {Brass, RedPlastic};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint LightBuffers[NumLightBuffers];
GLuint MaterialBuffers[NumMaterialBuffers];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint colCoords = 4;

// Model files
const char *objFiles[NumVAOs] = {"../models/unitcube.obj", "../models/sphere.obj"};

// Camera
vec3 eye = {4.0f, 4.0f, 4.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};

// Shader variables
// Shader program reference
GLuint gouraud_program;
// Shader component references
GLuint gouraud_vPos;
GLuint gouraud_vNorm;
GLuint gouraud_camera_mat_loc;
GLuint gouraud_model_mat_loc;
GLuint gouraud_proj_mat_loc;
GLuint gouraud_norm_mat_loc;
GLuint gouraud_lights_block_idx;
GLuint gouraud_materials_block_idx;
GLuint gouraud_material_loc;
GLuint gouraud_eye_loc;
const char *gouraud_vertex_shader = "../gouraud.vert";
const char *gouraud_frag_shader = "../gouraud.frag";

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 model_matrix;
mat4 normal_matrix;

// Global light and material variables
vector<LightProperties> Lights;
vector<MaterialProperties> Materials;

// Global cube variables
GLfloat cube_angle = 0.0;
GLdouble elTime = 0.0;
GLdouble rpm = 10.0;
vec3 axis = {0.0f, 1.0f, 0.0f};
bool anim = true;

// Global screen dimensions
GLint ww,hh;

void display( );
void render_scene( );
void build_geometry( );
void build_materials();
void build_lights();
void load_object(GLuint obj);
void draw_gouraud_mat_object(GLuint obj, GLuint material);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Gouraud Shading");
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

	// Create geometry buffers
    build_geometry();
    // Create light buffers
    build_lights();
    // Create material buffers
    build_materials();

    // Load shaders and associate shader variables
	ShaderInfo gouraud_shaders[] = { {GL_VERTEX_SHADER, gouraud_vertex_shader},{GL_FRAGMENT_SHADER, gouraud_frag_shader},{GL_NONE, NULL} };
    gouraud_program = LoadShaders(gouraud_shaders);
    gouraud_vPos = glGetAttribLocation(gouraud_program, "vPosition");
    gouraud_vNorm = glGetAttribLocation(gouraud_program, "vNormal");
    gouraud_camera_mat_loc = glGetUniformLocation(gouraud_program, "camera_matrix");
    gouraud_model_mat_loc = glGetUniformLocation(gouraud_program, "model_matrix");
    gouraud_proj_mat_loc = glGetUniformLocation(gouraud_program, "proj_matrix");
    gouraud_norm_mat_loc = glGetUniformLocation(gouraud_program, "normal_matrix");
    gouraud_lights_block_idx = glGetUniformBlockIndex(gouraud_program, "LightBuffer");
    gouraud_materials_block_idx = glGetUniformBlockIndex(gouraud_program, "MaterialBuffer");
    gouraud_material_loc = glGetUniformLocation(gouraud_program, "Material");
    gouraud_eye_loc = glGetUniformLocation(gouraud_program, "EyePosition");

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
        if (anim) {
            cube_angle += (curTime - elTime) * (rpm / 60.0) * 360.0;
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
    // Set projection matrix
    proj_matrix = frustum(-1.0f*xratio, 1.0f*xratio, -1.0f*yratio, 1.0f*yratio, 1.0f, 100.0f);

    // Set camera matrix
    camera_matrix = lookat(eye, center, up);

    // Render objects
	render_scene();

	glFlush();
}

void render_scene( ) {
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // Set cube transformation matrix
    trans_matrix = translation(0.0f, 0.0f, 0.0f);
    rot_matrix = rotation(cube_angle, normalize(axis));
    scale_matrix = scale(3.0f, 1.0f, 3.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // TODO: Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    // TODO: Draw cube object as brass
    draw_gouraud_mat_object(Cube, Brass);


    // Set sphere transformation matrix
    trans_matrix = translation(0.0f, 1.5f, 0.0f);
    rot_matrix = rotation(cube_angle, normalize(axis))*rotation(90.0f, 1.0f, 0.0f, 0.0f);
    scale_matrix = scale(1.0f, 1.0f, 1.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // TODO: Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    // TODO: Draw sphere object as red plastic
    draw_gouraud_mat_object(Sphere, RedPlastic);
}

void build_geometry( )
{
    // Generate vertex arrays for objects
    glGenVertexArrays(NumVAOs, VAOs);

    // Load objects
    load_object(Cube);
    load_object(Sphere);
}

void build_materials( ) {
    // TODO: Create brass material
    MaterialProperties brass = {
        vec4(0.33f, 0.22f, 0.03f, 1.0f), //ambient
        vec4(0.78f, 0.57f, 0.11f, 1.0f), //diffuse
        vec4(0.99f, 0.91f, 0.81f, 1.0f), //specular
        27.8f, //shininess
        {0.0f, 0.0f, 0.0f}  //pad
    };

    // TODO: Create red plastic material
    MaterialProperties redPlastic = {
        vec4(0.3f, 0.0f, 0.0f, 1.0f),
        vec4(0.6f, 0.0f, 0.0f, 1.0f),
        vec4(0.8f, 0.0f, 0.0f, 1.0f),
        32.0f,
        {0.0f, 0.0f, 0.0f}
    };

    // TODO: Add materials to Materials vector
    Materials.push_back(brass);
    Materials.push_back(redPlastic);
    // Create uniform buffer for materials
    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
}

void build_lights( ) {
    // TODO: Create white light
    LightProperties whiteLight = {
        DIRECTIONAL, //type
        {0.0f, 0.0f, 0.0f}, //pad
        vec4(0.0f, 0.0f, 0.0f, 0.0f), //ambient
        vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
        vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
        vec4(0.0f, 0.0f, 0.0f, 1.0f),  //position
        vec4(-1.0f, -1.0f, -1.0f, 0.0f), //direction
        0.0f,   //cutoff
        0.0f,  //exponent
        {0.0f, 0.0f}  //pad2
    };

    // TODO: Add lights to Lights vector
    Lights.push_back(whiteLight);
    // Create uniform buffer for lights
    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(), GL_STATIC_DRAW);
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

void draw_gouraud_mat_object(GLuint obj, GLuint material){
    // Select shader program
    glUseProgram(gouraud_program);
    // Pass projection and camera matrices to shader
    glUniformMatrix4fv(gouraud_proj_mat_loc, 1, GL_FALSE, proj_matrix);
    glUniformMatrix4fv(gouraud_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Bind lights
    glUniformBlockBinding(gouraud_program, gouraud_lights_block_idx, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size()*sizeof(LightProperties));

    // Bind materials
    glUniformBlockBinding(gouraud_program, gouraud_materials_block_idx, 1);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, MaterialBuffers[MaterialBuffer], 0, Materials.size()*sizeof(MaterialProperties));

    // Set camera position
    glUniform3fv(gouraud_eye_loc, 1, eye);

    // Pass model matrix and normal matrix to shader
    glUniformMatrix4fv(gouraud_model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(gouraud_norm_mat_loc, 1, GL_FALSE, normal_matrix);

    // Pass material index to shader
    glUniform1i(gouraud_material_loc, material);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(gouraud_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(gouraud_vPos);

    // Bind normal object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glVertexAttribPointer(gouraud_vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(gouraud_vNorm);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Move hexagon with arrow keys
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        anim = !anim;
    }
}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);

    ww = width;
    hh = height;
}