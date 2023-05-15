// CS370 - Exam 2
// Fall 2022

/******************************************/
/*       INSERT (a) CODE HERE             */
/******************************************/
// Your name
// Michael Geyer

#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "lighting.h"
#include "exam02.h"
#define DEG2RAD (M_PI/180.0)
#define RAD2DEG (180.0f/3.14159f)
#define NUM_MODES 8

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, Donut, Teapot, Mug, Coffee, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {GreyPlastic, Brass, Dough, YellowAcrylic, GreenAcrylic, CoffeeBlack};

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
vector<const char *> objFiles = {"../models/unitcube.obj", "../models/donut.obj", "../models/teapot.obj", "../models/mug.obj", "../models/cylinder.obj"};

// Camera
vec3 eye = {1.0f, 1.0f, 1.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};

// Shader variables
// Light shader program reference
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
vec4 whiteLightPos = {2.0f, 3.0f, 0.0f, 1.0f};

// Global spin variables - DO NOT MODIFY
GLfloat ppm = 6.0;
GLfloat pot_theta = 0.0f;
GLboolean tipping = true;
GLint pour_dir = -1.0;
GLfloat pour_ang = -30.0f;
GLboolean pouring = false;
GLfloat coffee_y = 0.0;
GLfloat coffee_fill = 1.0f;
GLdouble elTime = 0.0;

// Global spherical coord values
GLfloat azimuth = 45.0f;
GLfloat daz = 2.0f;
GLfloat elevation = 54.7f;
GLfloat del = 2.0f;
GLfloat radius = 1.732f;

// Global screen dimensions
GLint ww,hh;
int mode = 0;

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
void draw_table();

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Exam 2 Fall 2021");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // Store initial window size in global variables
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

    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Set background color
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    /******************************************/
    /*       INSERT (j) CODE HERE             */
    /******************************************/
    // TODO: Enable blending and set blend factors
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set initial camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye = vec3(x, y, z);

    // Set initial time
    elTime = glfwGetTime();

    // Start loop
    while ( !glfwWindowShouldClose( window ) ) {
    	// Draw graphics
        display();
        // Update other events like input handling
        glfwPollEvents();
        // Update angle based on time for fixed rpm
        GLdouble curTime = glfwGetTime();
        if (mode >= 7) {
            GLdouble dT = (curTime - elTime);
            /******************************************/
            /*       INSERT (l) and (m) CODE HERE     */
            /******************************************/
            // TODO: Time-based animation for pouring teapot
            pot_theta += pour_dir * ppm * dT;
            if (pot_theta < pour_ang) {
                pot_theta = pour_ang + 0.05f;
                pour_dir *= -1;
            } else if (pot_theta > 0.0f) {
                pot_theta = 0.0f - 0.05f;
                pour_dir *= -1;
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

    // Clear window and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute anisotropic scaling
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
    proj_matrix = ortho(-8.0f*xratio, 8.0f*xratio, -8.0f*yratio, 8.0f*yratio, -8.0f, 8.0f);

    // Set camera matrix
    camera_matrix = lookat(eye, center, up);

    // Render objects
	render_scene();

	// Flush pipeline
	glFlush();
}

void render_scene( ) {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();
    mat4 rot2_matrix = mat4().identity();

    // Mode 0/1: Table
    draw_table();

    // Mode 2: Brass teapot
    if (mode == 2) {
        /******************************************/
        /*       INSERT (d) CODE HERE             */
        /******************************************/
        // TODO: Render Brass teapot
        model_matrix = mat4().identity();
        scale_matrix = mat4().identity();
        rot_matrix = mat4().identity();
        trans_matrix = mat4().identity();
        rot2_matrix = mat4().identity();

        trans_matrix = translate(POT_X, 0.0f, POT_Z);
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Teapot, Brass);
    }
    // Mode 3: Brass teapot and donut
    else if (mode==3)
    {
        /******************************************/
        /*       INSERT (f) CODE HERE             */
        /******************************************/
        // TODO: Render Brass teapot and Dough donut
        model_matrix = mat4().identity();
        scale_matrix = mat4().identity();
        rot_matrix = mat4().identity();
        trans_matrix = mat4().identity();
        rot2_matrix = mat4().identity();

        trans_matrix = translate(POT_X, 0.0f, POT_Z);
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Teapot, Brass);

        trans_matrix = translate(STACK_X, 0.5f, STACK_Z);
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Donut, Dough);
    }
    // Mode 4: Brass teapot and stack of donuts
    else if (mode==4 || mode==5)
    {
        /******************************************/
        /*       INSERT (g) CODE HERE             */
        /******************************************/
        // TODO: Render Brass teapot and Dough donut stack
        model_matrix = mat4().identity();
        scale_matrix = mat4().identity();
        rot_matrix = mat4().identity();
        trans_matrix = mat4().identity();
        rot2_matrix = mat4().identity();

        trans_matrix = translate(POT_X, 0.0f, POT_Z);
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Teapot, Brass);

        trans_matrix = translate(STACK_X-1.5f, 0.5f, STACK_Z);
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Donut, Dough);

        trans_matrix = translate(STACK_X+1.5f, 0.5f, STACK_Z);
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Donut, Dough);

        trans_matrix = translate(STACK_X, 1.5f, STACK_Z);
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Donut, Dough);
    }
    // Mode 6: Render YellowAcrylic teapot and Dough donut stack with GreenAcrylic mug
    else if (mode==6)
    {
        /******************************************/
        /*       INSERT (j) CODE HERE             */
        /******************************************/
        // TODO: Render YellowAcrylic teapot and Dough donut stack with GreenAcrylic mug
        model_matrix = mat4().identity();
        scale_matrix = mat4().identity();
        rot_matrix = mat4().identity();
        trans_matrix = mat4().identity();
        rot2_matrix = mat4().identity();

        trans_matrix = translate(STACK_X-1.5f, 0.5f, STACK_Z);
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Donut, Dough);

        trans_matrix = translate(STACK_X+1.5f, 0.5f, STACK_Z);
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Donut, Dough);

        trans_matrix = translate(STACK_X, 1.5f, STACK_Z);
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Donut, Dough);

        trans_matrix = translate(POT_X, 1.2f, POT_Z);
        scale_matrix = mat4().identity();
        rot2_matrix = rotate(150.0f, vec3(0.0f, 1.0f, 0.0f));
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Teapot, YellowAcrylic);

        trans_matrix = translate(CUP_X, 0.0f, CUP_Z);
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        rot2_matrix = mat4().identity();
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Mug, GreenAcrylic);
    }
    // Mode 7: Animated yellow acrylic teapot, stack of three dough donuts, and green acrylic mug
    else if (mode==7)
    {
        /******************************************/
        /*       INSERT (l) CODE HERE             */
        /******************************************/
        // TODO: Render animated YellowAcrylic teapot and Dough donut stack with GreenAcrylic mug
        model_matrix = mat4().identity();
        scale_matrix = mat4().identity();
        rot_matrix = mat4().identity();
        trans_matrix = mat4().identity();
        rot2_matrix = mat4().identity();

        trans_matrix = translate(STACK_X-1.5f, 0.5f, STACK_Z);
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Donut, Dough);

        trans_matrix = translate(STACK_X+1.5f, 0.5f, STACK_Z);
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Donut, Dough);

        trans_matrix = translate(STACK_X, 1.5f, STACK_Z);
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Donut, Dough);

        trans_matrix = translate(POT_X, 1.2f, POT_Z);
        scale_matrix = mat4().identity();
        rot_matrix = rotate(pot_theta ,vec3(0.0f, 0.0f, 1.0f));
        rot2_matrix = rotate(150.0f, vec3(0.0f, 1.0f, 0.0f));
        model_matrix = scale_matrix*trans_matrix*rot2_matrix*rot_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Teapot, YellowAcrylic);

        trans_matrix = translate(CUP_X, 0.0f, CUP_Z);
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        rot2_matrix = mat4().identity();
        rot_matrix = mat4().identity();
        model_matrix = scale_matrix*rot_matrix*trans_matrix*rot2_matrix;
        // Compute normal matrix from model matrix
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Mug, GreenAcrylic);
    }
    // Extra Credit Mode 8: Animated yellow acrylic teapot, stack of three dough donuts, and green acrylic mug
    // filling with coffee
    else if (mode==8)
    {
        /******************************************/
        /*       INSERT (m) CODE HERE             */
        /******************************************/
        // TODO: Render animated YellowAcrylic teapot and Dough donut stack with GreenAcrylic mug and CoffeeBlack coffee

    }

}

void build_materials( ) {
    Materials.clear();
    // Create grey plastic material
    MaterialProperties greyPlastic = {
            vec4(0.1f, 0.1f, 0.1f, 1.0f), //ambient
            vec4(0.6f, 0.6f, 0.6f, 1.0f), //diffuse
            vec4(0.8f, 0.8f, 0.8f, 1.0f), //specular
            10.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    Materials.push_back(greyPlastic);

    if (mode >= 2) {
        /******************************************/
        /*       INSERT (c) CODE HERE             */
        /******************************************/
        // TODO: Create brass material
        MaterialProperties brass = {
                vec4(0.33f, 0.22f, 0.03f, 1.0f), //ambient
                vec4(0.78f, 0.57f, 0.11f, 1.0f), //diffuse
                vec4(0.99f, 0.91f, 0.81f, 1.0f), //specular
                27.8f, //shininess
                {0.0f, 0.0f, 0.0f}  //pad
        };
        Materials.push_back(brass);
    }

    if (mode >= 3) {
        /******************************************/
        /*       INSERT (e) CODE HERE             */
        /******************************************/
        // TODO: Create dough material
        MaterialProperties dough = {
                vec4(0.3f, 0.3f, 0.0f, 1.0f), //ambient
                vec4(0.6f, 0.6f, 0.0f, 1.0f), //diffuse
                vec4(0.8f, 0.8f, 0.6f, 1.0f), //specular
                32.0f, //shininess
                {0.0f, 0.0f, 0.0f}  //pad
        };
        Materials.push_back(dough);
    }

    if (mode >= 6) {
        /******************************************/
        /*       INSERT (i) CODE HERE             */
        /******************************************/
        // TODO: Create yellow acrylic (translucent) material
        MaterialProperties yellowAcrylic = {
                vec4(0.3f, 0.3f, 0.0f, 0.8f), //ambient
                vec4(0.6f, 0.6f, 0.0f, 0.8f), //diffuse
                vec4(0.8f, 0.8f, 0.6f, 0.8f), //specular
                32.0f, //shininess
                {0.0f, 0.0f, 0.0f}  //pad
        };

        // TODO: Create green acrylic (translucent) material
        MaterialProperties greenAcrylic = {
                vec4(0.0f, 0.3f, 0.0f, 0.6f), //ambient
                vec4(0.0f, 0.6f, 0.0f, 0.6f), //diffuse
                vec4(0.6f, 0.8f, 0.6f, 0.6f), //specular
                32.0f, //shininess
                {0.0f, 0.0f, 0.0f}  //pad
        };
        Materials.push_back(yellowAcrylic);
        Materials.push_back(greenAcrylic);
    }

    if (mode >= 8) {
        /******************************************/
        /*       INSERT (m) CODE HERE             */
        /******************************************/
        // TODO: Create coffeeBlack (translucent) material
        MaterialProperties coffeeBlack = {};
        Materials.push_back(coffeeBlack);
    }

    // Create uniform buffer for materials
    if (glIsBuffer(MaterialBuffers[MaterialBuffer])) {
        glDeleteBuffers(NumMaterialBuffers, MaterialBuffers);
    }
    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
}

void build_lights( ) {
    Lights.clear();
    if (mode > 0) {
        /******************************************/
        /*       INSERT (b) CODE HERE             */
        /******************************************/
        // TODO: Add white point light
        LightProperties whitePointLight = {
                POINT, //type
                {0.0f, 0.0f, 0.0f}, //pad
                vec4(0.1f, 0.1f, 0.1f, 1.0f), //ambient
                vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
                vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
                vec4(whiteLightPos[0], whiteLightPos[1], whiteLightPos[2], whiteLightPos[3]),  //position
                vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
                0.0f,   //cutoff
                0.0f,  //exponent
                {0.0f, 0.0f}  //pad2
        };
        Lights.push_back(whitePointLight);
    }
    if (mode > 4) {
        /******************************************/
        /*       INSERT (h) CODE HERE             */
        /******************************************/
        // TODO: Add red spotlight
        LightProperties redSpotLight = {
                SPOT, //type
                {0.0f, 0.0f, 0.0f}, //pad
                vec4(0.1f, 0.0f, 0.0f, 1.0f), //ambient
                vec4(1.0f, 0.0f, 0.0f, 1.0f), //diffuse
                vec4(1.0f, 0.0f, 0.0f, 1.0f), //specular
                vec4(STACK_X, 4.0f, STACK_Z, 1.0f),  //position
                vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
                30.0f,   //cutoff
                30.0f,  //exponent
                {0.0f, 0.0f}  //pad2
        };
        Lights.push_back(redSpotLight);
    }
    if (mode > 5) {
        /******************************************/
        /*       INSERT (k) CODE HERE             */
        /******************************************/
        // TODO: Add green spotlight
        LightProperties greenSpotLight = {
                SPOT, //type
                {0.0f, 0.0f, 0.0f}, //pad
                vec4(0.0f, 0.1f, 0.0f, 1.0f), //ambient
                vec4(0.0f, 1.0f, 0.0f, 1.0f), //diffuse
                vec4(0.0f, 1.0f, 0.0f, 1.0f), //specular
                vec4(CUP_X, 4.0f, CUP_Z, 1.0f),  //position
                vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
                30.0f,   //cutoff
                30.0f,  //exponent
                {0.0f, 0.0f}  //pad2
        };
        Lights.push_back(greenSpotLight);
    }


    // Set numLights
    numLights = Lights.size();

    // Turn all lights on
    for (int i = 0; i < numLights; i++) {
        lightOn[i] = 1;
    }

    // Create uniform buffer for lights
    if (glIsBuffer(LightBuffers[LightBuffer])) {
        glDeleteBuffers(NumLightBuffers, LightBuffers);
    }
    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(), GL_STATIC_DRAW);
}

#include "examfunc.cpp"