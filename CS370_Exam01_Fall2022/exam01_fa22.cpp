// CS370 - Exam 1
// Fall 2022

/******************************************/
/*       INSERT (a) CODE HERE             */
/******************************************/
// Michael Geyer

#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "exam01.h"
#define DEG2RAD (M_PI/180.0)
#define RAD2DEG (180.0f/3.14159f)
#define NUM_MODES 7

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, Sphere, Pyramid, Target, NumVAOs};
enum Obj_Buffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum Color_Buffer_IDs {PyramidGrad, SphereGreen, BarBlack, TableColorBuffer, TargetColorBuffer,  NumColorBuffers};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint ColorBuffers[NumColorBuffers];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint colCoords = 4;

// Model files
vector<const char *> objFiles = {"../models/unitcube.obj", "../models/sphere.obj"};

// Camera
vec3 eye = {3.0f, 3.0f, 3.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};

// Shader variables
// Shader program reference
GLuint default_program;
// Shader component references
GLuint default_vPos;
GLuint default_vCol;
GLuint default_proj_mat_loc;
GLuint default_cam_mat_loc;
GLuint default_model_mat_loc;
const char *default_vertex_shader = "../default.vert";
const char *default_frag_shader = "../default.frag";

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 model_matrix;

// Global state
GLfloat pyr_angle = 0.0;
GLdouble elTime = 0.0;
GLdouble rpm = 10.0;
vec3 axis = {1.0f, 1.0f, 1.0f};
// Global spin variables - DO NOT MODIFY
GLfloat spin_theta = 0.0f;
GLboolean spin_flag = false;
GLfloat rev_theta = 0.0f;
GLboolean rev_flag = false;

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
void build_solid_color_buffer(GLuint obj, vec4 color, GLuint buffer);
void load_object(GLuint obj);
void draw_color_obj(GLuint obj, GLuint color);
void build_target();
void build_pyramid();
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);
void draw_table();
void draw_target();
void draw_pyramid();

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Exam 1 Fall 2022");
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

    // Load shaders and associate variables
    ShaderInfo default_shaders[] = { {GL_VERTEX_SHADER, default_vertex_shader},{GL_FRAGMENT_SHADER, default_frag_shader},{GL_NONE, NULL} };
    default_program = LoadShaders(default_shaders);
    default_vPos = glGetAttribLocation(default_program, "vPosition");
    default_vCol = glGetAttribLocation(default_program, "vColor");
    default_proj_mat_loc = glGetUniformLocation(default_program, "proj_matrix");
    default_cam_mat_loc = glGetUniformLocation(default_program, "camera_matrix");
    default_model_mat_loc = glGetUniformLocation(default_program, "model_matrix");

    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Set background color
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

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
        if (mode > 3)
        {
            GLdouble curTime = glfwGetTime();
            if (spin_flag)
            {
                /******************************************/
                /*       INSERT (f) CODE HERE             */
                /******************************************/
                // TODO: Set spin_theta

                spin_theta += rpm * (360.0 / 60.0) * (curTime - elTime);
            }

            if (mode > 4)
            {
                if (rev_flag)
                {
                    /******************************************/
                    /*       INSERT (i) CODE HERE             */
                    /******************************************/
                    // TODO: Set rev_theta
                    rev_theta = -rpm * 360/60 * (curTime - elTime);
                }
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
    // Declare projection and camera matrices
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

    /******************************************/
    /*       INSERT (b) CODE HERE             */
    /******************************************/
    // TODO: Set projection matrix
    proj_matrix = mat4().identity();
    proj_matrix = ortho(-6.0f*xratio, 6.0f*xratio, -6.0f*yratio, 6.0f*yratio, -6.0f, 6.0f);
    // TODO: Set camera matrix
    camera_matrix = mat4().identity();
    camera_matrix = lookat(eye, center, up);

    // Render objects
	render_scene();

	// Flush pipeline
	glFlush();
}

void render_scene( ) {
    /*********************************************************/
    /* TODO: Declare additional transformation matrices here */
    /*********************************************************/
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    draw_table();
    // (c) Pyramid instance model
    if (mode == 1) {
        draw_color_obj(Pyramid, PyramidGrad);
    }
    // (d) Basic pyramid centered at origin
    else if (mode==2)
    {
        /******************************************/
        /*       INSERT (d) CODE HERE             */
        /******************************************/
        // TODO: Add pyramid transformations
        rot_matrix = rotate(90.0f, vec3(0.0f, 0.0f, 1.0f));
        scale_matrix = scale(0.5f, 1.0f, 0.5f);
        trans_matrix = translate(0.5f, 0.0f, 0.0f);
        model_matrix = trans_matrix * scale_matrix * rot_matrix;
        draw_color_obj(Pyramid, PyramidGrad);
    }
    // (e) Pyramid located at target location
    else if (mode==3)
    {
        /******************************************/
        /*       INSERT (e) CODE HERE             */
        /******************************************/
        // TODO: Add pyramid transformations
        rot_matrix = rotate(-90.0f, vec3(0.0f, 0.0f, 1.0f));
        scale_matrix = scale(0.5f, 1.0f, 0.5f);
        trans_matrix = translate(TARGET_X-0.5f, TARGET_Y+1.0f, TARGET_Z);
        model_matrix = trans_matrix * scale_matrix * rot_matrix;
        draw_color_obj(Pyramid, PyramidGrad);
    }
    // (f) Spinning pyramid at target location
    else if (mode==4)
    {
        /******************************************/
        /*       INSERT (f) CODE HERE             */
        /******************************************/
        // TODO: Add pyramid transformations
        rot_matrix = rotate(-90.0f, vec3(0.0f, 0.0f, 1.0f));
        scale_matrix = scale(0.5f, 1.0f, 0.5f);
        trans_matrix = translate(TARGET_X, TARGET_Y, TARGET_Z);
        model_matrix = trans_matrix * rotate(spin_theta, vec3(0.0f, 1.0f, 0.0f)) * translate(-0.5f, 1.0f, 0.0f) * scale_matrix * rot_matrix;
        draw_color_obj(Pyramid, PyramidGrad);
    }
    // (h) Spinning pyramid and initial sphere
    else if (mode==5)
    {
        /******************************************/
        /*       INSERT (h) CODE HERE          */
        /******************************************/
        // TODO: Add pyramid transformations
        rot_matrix = rotate(-90.0f, vec3(0.0f, 0.0f, 1.0f));
        scale_matrix = scale(0.5f, 1.0f, 0.5f);
        trans_matrix = translate(TARGET_X, TARGET_Y, TARGET_Z);
        model_matrix = trans_matrix * rotate(spin_theta, vec3(0.0f, 1.0f, 0.0f)) * translate(-0.5f, 1.0f, 0.0f) * scale_matrix * rot_matrix;
        draw_color_obj(Pyramid, PyramidGrad);

        // TODO: Draw sphere
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        model_matrix = scale_matrix;
        draw_color_obj(Sphere, SphereGreen);
    }
    // (i) Spinning pyramids and revolving sphere
    else if (mode==6)
    {
        /******************************************/
        /*       INSERT (i) CODE HERE          */
        /******************************************/
        // TODO: Add pyramid transformations
        rot_matrix = rotate(-90.0f, vec3(0.0f, 0.0f, 1.0f));
        scale_matrix = scale(0.5f, 1.0f, 0.5f);
        trans_matrix = translate(TARGET_X, TARGET_Y, TARGET_Z);
        model_matrix = trans_matrix * rotate(spin_theta, vec3(0.0f, 1.0f, 0.0f)) * translate(-0.5f, 1.0f, 0.0f) * scale_matrix * rot_matrix;
        draw_color_obj(Pyramid, PyramidGrad);

        // TODO: Draw sphere
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        trans_matrix = translate(TARGET_X, TARGET_Y, TARGET_Z);
        rot_matrix = rotate(rev_theta, vec3(0.0f, 1.0f, 0.0f));
        model_matrix = trans_matrix*rot_matrix*scale_matrix;
        draw_color_obj(Sphere, SphereGreen);
    }
    // (j) EXTRA CREDIT: Spinning pyramid and revolving sphere/bar at target location
    else if (mode==7)
    {
        /******************************************/
        /*       INSERT (j) CODE HERE          */
        /******************************************/
        // TODO: Add pyramid transformations

        draw_color_obj(Pyramid, PyramidGrad);

        // TODO: Draw spheres and bar

    }
}

void build_geometry( )
{
    // Generate vertex arrays and buffers
	glGenVertexArrays(NumVAOs, VAOs);
    glGenBuffers(NumColorBuffers, ColorBuffers);

    // Load models
    load_object(Cube);

    // Build target and pyramid geometries
    build_target();
    build_pyramid();

    // Build table and bar color buffers
    build_solid_color_buffer(numVertices[Cube], vec4(1.0f, 1.0f, 1.0f, 1.0f), TableColorBuffer);
    build_solid_color_buffer(numVertices[Cube], vec4(0.0f, 0.0f, 0.0f, 1.0f), BarBlack);

    /******************************************/
    /*       INSERT (g) CODE HERE          */
    /******************************************/
    // TODO: Load sphere model
    load_object(Sphere);
    // TODO: Build SphereGreen color buffer
    build_solid_color_buffer(numVertices[Sphere], vec4(0.0f, 1.0f, 0.0f,1.0f), SphereGreen);
}



void build_pyramid() {
    vector<vec4> vertices;
    vector<vec4> colors;
    vector<vec3> indices;

    // Bind pyramid vertex array object
    glBindVertexArray(VAOs[Pyramid]);

    // Define vertices
    vertices = {
            {0.0f, 0.0f, 1.0f, 1.0f},
            {0.0f, 2.0f, 1.0f, 1.0f},
            {0.0f, 2.0f, -1.0f, 1.0f},
            {0.0f, 0.0f, -1.0f, 1.0f},
            {1.0f, 1.0f, 0.0f, 1.0f},
    };

    /******************************************/
    /*       INSERT (c) CODE HERE             */
    /******************************************/
    // TODO: Define face colors (per vertex)
    colors = {
            {1.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 0.0f, 1.0f},
    };

    // TODO: Define pyramid face indices (ensure proper orientation)
    indices = {
            {0,1,2},
            {2,3,0},
            {4,2,1},
            {4,1,0},
            {4,3,2},
            {3,4,0},
    };
    int numFaces = indices.size();

    // Create object vertices and colors from faces - DO NOT MODIFY
    vector<vec4> obj_vertices;
    vector<vec4> obj_colors;
    for (int i = 0; i < numFaces; i++) {
        for (int j = 0; j < 3; j++) {
            obj_vertices.push_back(vertices[indices[i][j]]);
            obj_colors.push_back(colors[indices[i][j]]);
        }
    }

    // Set numVertices as 3*total number of FACES
    numVertices[Pyramid] = 3*numFaces;

    // Generate object buffers for Pyramid
    glGenBuffers(NumObjBuffers, ObjBuffers[Pyramid]);

    // Bind pyramid positions
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Pyramid][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[Pyramid], obj_vertices.data(), GL_STATIC_DRAW);

    // Bind pyramid colors
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[PyramidGrad]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[Pyramid], obj_colors.data(), GL_STATIC_DRAW);
}

#include "examfunc.cpp"