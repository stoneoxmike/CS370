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
enum VAO_IDs {Cube, Sphere, Torus, Cone, NumVAOs};
enum Obj_Buffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum LightNames {WhitePointLight, GreenSpotLight};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {Brass, RedPlastic, WhitePlastic, RedAcrylic};

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
const char *objFiles[NumVAOs] = {"../models/unitcube.obj", "../models/sphere.obj", "../models/torus.obj", "../models/cone.obj"};

// Camera
vec3 eye = {0.0f, 0.0f, 4.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};

// Shader variables
// Shader program reference
GLuint lighting_program;
// Shader component references
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

// Global state
mat4 proj_matrix;
mat4 model_matrix;
mat4 camera_matrix;
mat4 normal_matrix;

// Global cube variables
GLfloat cube_angle = 0.0;
GLdouble elTime = 0.0;
GLdouble rpm = 10.0;
vec3 axis = {0.0f, 1.0f, 0.0f};

// Global light and material variables
vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights;
GLint lightOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Global object initial positions
vec3 sphere_pos = {0.0f,0.0f,-0.2f};
vec3 torus_pos = {0.0f,-2.8f,-1.5f};
GLfloat torus_theta = 0.0f;
GLboolean bounce_sphere = true;
GLboolean roll_torus = true;
GLfloat SPHERE_STEP = 6.0f;
GLfloat SPHERE_MIN = -0.2f;
GLfloat SPHERE_MAX = 2.0f;
GLint sphere_dir = 1;
GLfloat TORUS_STEP = 90.0f;

// Global screen dimensions
GLint ww,hh;

void build_geometry( );
void build_lights();
void build_materials();
void display( );
void render_scene( );
void build_geometry( );
void build_materials();
void build_lights();
void load_object(GLuint obj);
void draw_mat_object(GLuint obj, GLuint material);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Blended Cone");
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
    // Create material buffers
    build_materials();
    // Create light buffers
    build_lights();

    // Load shaders
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

    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // TODO: Enable alpha blending and set blend factors
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Start loop
    while ( !glfwWindowShouldClose( window ) ) {
    	// Draw graphics
        display();
        // Update other events like input handling
        glfwPollEvents();
        GLdouble curTime = glfwGetTime();
        GLdouble dT = curTime - elTime;
        // Bounce sphere
        if (bounce_sphere)
        {
            sphere_pos[1] += sphere_dir*SPHERE_STEP*dT;
            if (sphere_pos[1] > SPHERE_MAX) {
                sphere_pos[1] = SPHERE_MAX - 0.05f;
                sphere_dir *= -1;
            } else if (sphere_pos[1] < SPHERE_MIN) {
                sphere_pos[1] = SPHERE_MIN + 0.05f;
                sphere_dir *= -1;
            }
        }

        // TODO: Roll torus
        if (roll_torus)
        {
            torus_theta += TORUS_STEP*dT;
            if (torus_theta > 360.0f)
            {
                torus_theta -= 360.0f;
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
    proj_matrix = frustum(-4.0f*xratio, 4.0f*xratio, -4.0f*yratio, 4.0f*yratio, 2.0f, 6.0f);

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
	mat4 tor_rot_matrix = mat4().identity();

    // Set cube transformation matrix
    trans_matrix = translate(0.0f, -2.1f, 0.0f);
    scale_matrix = scale(6.0f, 0.2f, 6.0f);
    model_matrix = trans_matrix*scale_matrix;
    // Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    // Draw object
    draw_mat_object(Cube, WhitePlastic);

    // Set sphere transformation matrix
    trans_matrix = translate(sphere_pos);
    scale_matrix = scale(1.0f, 1.0f, 1.0f);
    model_matrix = trans_matrix*scale_matrix;
    // Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    // Draw object
    draw_mat_object(Sphere, RedPlastic);

    // Set torus transformation matrix
    trans_matrix = translate(torus_pos);
    scale_matrix = scale(0.5f, 0.5f, 0.5f);
    rot_matrix = rotate(torus_theta, vec3(0.0f, 1.0f, 0.0f));
    tor_rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
    model_matrix = scale_matrix*rot_matrix*trans_matrix*tor_rot_matrix;
    // Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    // Draw object
    draw_mat_object(Torus, Brass);

    // Set (translucent) cylinder transformation matrix
    trans_matrix = translate(vec3(0.0f, -2.0f, 0.0f));
    scale_matrix = scale(1.0f, 1.0f, 1.0f);
    model_matrix = trans_matrix*scale_matrix;
    // Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    // TODO: Draw translucent object (disable depth mask)
    glDepthMask(GL_FALSE);
    draw_mat_object(Cone, RedAcrylic);
    glDepthMask(GL_TRUE);
}

void build_geometry( )
{
    // Generate vertex arrays for objects
    glGenVertexArrays(NumVAOs, VAOs);

    // Load objects
    load_object(Cube);
    load_object(Sphere);
    load_object(Torus);
    load_object(Cone);

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

    // Create white plastic material
    MaterialProperties whitePlastic = {
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    // TODO: Create red acrylic material
    MaterialProperties redAcrylic = {
            vec4(0.3f, 0.0f, 0.0f, 0.5f), //ambient
            vec4(0.6f, 0.0f, 0.0f, 0.5f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 0.5f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    // Add materials to Materials vector
    Materials.push_back(brass);
    Materials.push_back(redPlastic);
    Materials.push_back(whitePlastic);
    // TODO: Add red acrylic to Materials vector
    Materials.push_back(redAcrylic);

    // Create uniform buffer for materials
    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
}

void build_lights( ) {
    // Create white point light
    LightProperties whitePointLight = {
            POINT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(3.0f, 1.0f, 3.0f, 1.0f),  //position
            vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
            0.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    // Add lights to Lights vector
    Lights.push_back(whitePointLight);

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

void draw_mat_object(GLuint obj, GLuint material){
    // Select shader program
    glUseProgram(lighting_program);

    // Pass projection and camera matrices to shader
    glUniformMatrix4fv(lighting_proj_mat_loc, 1, GL_FALSE, proj_matrix);
    glUniformMatrix4fv(lighting_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Bind lights
    glUniformBlockBinding(lighting_program, lighting_lights_block_idx, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size()*sizeof(LightProperties));

    // Bind materials
    glUniformBlockBinding(lighting_program, lighting_materials_block_idx, 1);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, MaterialBuffers[MaterialBuffer], 0, Materials.size()*sizeof(MaterialProperties));

    // Set camera position
    glUniform3fv(lighting_eye_loc, 1, eye);

    // Set num lights and lightOn
    glUniform1i(lighting_num_lights_loc, numLights);
    glUniform1iv(lighting_light_on_loc, numLights, lightOn);

    // Pass model matrix and normal matrix to shader
    glUniformMatrix4fv(lighting_model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(lighting_norm_mat_loc, 1, GL_FALSE, normal_matrix);

    // Pass material index to shader
    glUniform1i(lighting_material_loc, material);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(lighting_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vPos);

    // Bind normal object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glVertexAttribPointer(lighting_vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vNorm);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Esc exits program
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }
}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);

    ww = width;
    hh = height;
}
