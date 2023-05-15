// CS370 Assignment 2 - Rollin Train
// Fall 2022

#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "RollinTrain.h"
#define DEG2RAD (M_PI/180.0)
#define RAD2DEG (180.0f/3.14159f)

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, Cylinder, Cone, Torus, NumVAOs};
enum Obj_Buffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum Color_Buffer_IDs {TiesColor, RailColor, BodyColor, EngColor, CylColor, ConeColor,
        WheelColor, SpokeColor, BottomColor, MiddleColor, TopColor, NumColorBuffers};

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
vector<const char *> objFiles = {"../models/unitcube.obj", "../models/cylinder.obj", "../models/cone.obj", "../models/torus.obj"};

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
GLdouble elTime = 0.0;
GLdouble speed = (RAIL_LENGTH/4.0f);
vec3 train_pos = {0.0f, 0.0f, 0.0f};
GLint train_dir = -1;
GLfloat wheel_ang = 0.0f;
GLboolean animate = false;

// Global spherical coord values
GLfloat azimuth = 132.0f;
GLfloat daz = 2.0f;
GLfloat elevation = 112.0f;
GLfloat del = 2.0f;
GLfloat radius = 6.0f;
GLfloat dr = 0.1f;
GLfloat min_radius = 2.0f;

// View modes
#define ORTHOGRAPHIC 0
#define PERSPECTIVE 1
int proj = ORTHOGRAPHIC;

// Component indices
#define X 0
#define Y 1
#define Z 2

// Global screen dimensions
GLint ww,hh;

void display( );
void render_scene( );
void build_geometry( );
void load_object(GLuint obj);
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer);
void draw_color_obj(GLuint obj, GLuint color);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Rollin Train 2022");
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

    // Load shaders and associate variables
    ShaderInfo default_shaders[] = { {GL_VERTEX_SHADER, default_vertex_shader},{GL_FRAGMENT_SHADER, default_frag_shader},{GL_NONE, NULL} };
    default_program = LoadShaders(default_shaders);
    default_vPos = glGetAttribLocation(default_program, "vPosition");
    default_vCol = glGetAttribLocation(default_program, "vColor");
    default_proj_mat_loc = glGetUniformLocation(default_program, "proj_matrix");
    default_cam_mat_loc = glGetUniformLocation(default_program, "camera_matrix");
    default_model_mat_loc = glGetUniformLocation(default_program, "model_matrix");

    // Enable depth test
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Set background color
    glClearColor(1.0f,1.0f,1.0f,1.0f);

    // Set Initial camera position
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
        GLdouble curTime = glfwGetTime();
        if (animate) {
            // TODO: Add animation
            if ((train_pos[2] >= (RAIL_LENGTH/2 - BODY_LENGTH) && train_pos[2] >= 0) || (train_pos[2] <= (-RAIL_LENGTH/2 + BODY_LENGTH)  && train_pos[2] <= 0)) {
                train_dir = train_dir * -1;
            }
            train_pos[2] += train_dir * speed * (curTime - elTime);
            wheel_ang += train_dir * speed * (360/60) * (curTime - elTime)*8;
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

    // TODO: Set projection and camera matrices
    // Position camera for orthographic projection
    if (proj == ORTHOGRAPHIC)
    {
        // TODO: Set orthographic (birds-eye) view (spherical coords)
        proj_matrix = ortho(-15.0f*xratio, 15.0f*xratio, -15.0f*yratio, 15.0f*yratio, 15.0f, -15.0f);
        camera_matrix = lookat(eye, center, up);
    }
    // Position camera for perspective projection
    else if (proj == PERSPECTIVE)
    {
        // TODO: Set dynamic perspective (first-person) view
        proj_matrix = frustum(-1.0f*xratio, 1.0f*xratio, -0.4f*yratio, 1.0f*yratio, 1.0f, 30.0f);
        camera_matrix = lookat(vec3(0.0f, ENG_Y, train_pos[2]+1.0f), vec3(0.0f, 2.0f, -25.0f), up);
    }

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

    // TODO: Draw objects
    // bottom box
    trans_matrix = translate(0.0f, BOTTOM_BLOCK_SIZE/2, -13.0f);
    scale_matrix = scale(BOTTOM_BLOCK_SIZE, BOTTOM_BLOCK_SIZE, BOTTOM_BLOCK_SIZE);
    model_matrix = trans_matrix*scale_matrix;
    draw_color_obj(Cube, BottomColor);
    // middle box
    trans_matrix = translate(0.0f, BOTTOM_BLOCK_SIZE+MIDDLE_BLOCK_SIZE/2, -13.0f);
    scale_matrix = scale(MIDDLE_BLOCK_SIZE, MIDDLE_BLOCK_SIZE, MIDDLE_BLOCK_SIZE);
    model_matrix = trans_matrix*scale_matrix;
    draw_color_obj(Cube, MiddleColor);
    // top box
    trans_matrix = translate(0.0f, (BOTTOM_BLOCK_SIZE+MIDDLE_BLOCK_SIZE)+TOP_BLOCK_SIZE/2, -13.0f);
    scale_matrix = scale(TOP_BLOCK_SIZE, TOP_BLOCK_SIZE, TOP_BLOCK_SIZE);
    model_matrix = trans_matrix*scale_matrix;
    draw_color_obj(Cube, TopColor);
    // left rail
    trans_matrix = translate(-RAIL_OFFSET_X, RAIL_OFFSET_Y, RAIL_OFFSET_Z);
    scale_matrix = scale(RAIL_WIDTH, RAIL_HEIGHT, RAIL_LENGTH);
    model_matrix = trans_matrix*scale_matrix;
    draw_color_obj(Cube, RailColor);
    // right rail
    trans_matrix = translate(RAIL_OFFSET_X, RAIL_OFFSET_Y, RAIL_OFFSET_Z);
    scale_matrix = scale(RAIL_WIDTH, RAIL_HEIGHT, RAIL_LENGTH);
    model_matrix = trans_matrix*scale_matrix;
    draw_color_obj(Cube, RailColor);
    // rail ties
    for (int i = 0; i < RAIL_LENGTH/2; i++) {
        trans_matrix = translate(RAIL_TIES_OFFSET_X, RAIL_TIES_OFFSET_Y, (i*1.0f+RAIL_TIES_OFFSET_Z));
        scale_matrix = scale(RAIL_TIES_WIDTH, RAIL_TIES_HEIGHT, RAIL_TIES_LENGTH);
        model_matrix = trans_matrix * scale_matrix;
        draw_color_obj(Cube, TiesColor);
    }
    for (int i = 0; i < RAIL_LENGTH/2; i++) {
        trans_matrix = translate(RAIL_TIES_OFFSET_X, RAIL_TIES_OFFSET_Y, (-i*1.0f+RAIL_TIES_OFFSET_Z));
        scale_matrix = scale(RAIL_TIES_WIDTH, RAIL_TIES_HEIGHT, RAIL_TIES_LENGTH);
        model_matrix = trans_matrix * scale_matrix;
        draw_color_obj(Cube, TiesColor);
    }

    // parameterize below
    // engineer's box
    trans_matrix = translate(ENG_X, ENG_Y, ENG_Z+train_pos[2]);
    scale_matrix = scale(ENG_WIDTH, ENG_HEIGHT, ENG_LENGTH);
    model_matrix = trans_matrix*scale_matrix;
    draw_color_obj(Cube, EngColor);
    // left wheels
    for (int i = 0; i < 3; i++) {
        trans_matrix = translate(-WHEEL_X, WHEEL_Y, (-i*MID_WHEEL_OFFSET+WHEEL_Z+train_pos[2]));
        scale_matrix = scale(WHEEL_WIDTH, WHEEL_RADIUS, WHEEL_RADIUS);
        rot_matrix = rotate(90.0f, vec3(0.0f, 0.0f, 1.0f));
        model_matrix = trans_matrix * scale_matrix * rot_matrix;
        draw_color_obj(Torus, WheelColor);
    }
    // right wheels
    for (int i = 0; i < 3; i++) {
        trans_matrix = translate(WHEEL_X, WHEEL_Y, (-i*MID_WHEEL_OFFSET+WHEEL_Z+train_pos[2]));
        scale_matrix = scale(WHEEL_WIDTH, WHEEL_RADIUS, WHEEL_RADIUS);
        rot_matrix = rotate(90.0f, vec3(0.0f, 0.0f, 1.0f));
        model_matrix = trans_matrix * scale_matrix * rot_matrix;
        draw_color_obj(Torus, WheelColor);
    }
    // smokestack
    trans_matrix = translate(STACK_X, STACK_Y, STACK_Z+train_pos[2]);
    scale_matrix = scale(STACK_RADIUS, STACK_HEIGHT, STACK_RADIUS);
    model_matrix = trans_matrix * scale_matrix;
    draw_color_obj(Cylinder, CylColor);
    // smokestack
    trans_matrix = translate(STACK_X, STACK_Y+STACK_HEIGHT, STACK_Z + train_pos[2]);
    scale_matrix = scale(FUNNEL_RADIUS, FUNNEL_HEIGHT, FUNNEL_RADIUS);
    rot_matrix = rotate(180.0f, vec3(0.0f, 0.0f, 1.0f));
    model_matrix = trans_matrix * scale_matrix * rot_matrix;
    draw_color_obj(Cone, ConeColor);
    // body
    trans_matrix = translate(BODY_X, BODY_Y, BODY_Z+train_pos[2]);
    scale_matrix = scale(BODY_WIDTH, BODY_HEIGHT, BODY_LENGTH);
    model_matrix = trans_matrix*scale_matrix;
    draw_color_obj(Cube, BodyColor);

    // rotate below
    // right spokes
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
        trans_matrix = translate(WHEEL_X, WHEEL_Y, (-i*MID_WHEEL_OFFSET+WHEEL_Z+train_pos[2]));
            scale_matrix = scale(SPOKE_WIDTH, SPOKE_WIDTH, SPOKE_LENGTH);
            rot_matrix = rotate(j * 120.0f + wheel_ang, vec3(1.0f, 0.0f, 0.0f));
            model_matrix = trans_matrix * rot_matrix * scale_matrix;
            draw_color_obj(Cube, SpokeColor);
        }
    }
    // left spokes
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            trans_matrix = translate(-WHEEL_X, WHEEL_Y, (-i*MID_WHEEL_OFFSET+WHEEL_Z+train_pos[2]));
            scale_matrix = scale(SPOKE_WIDTH, SPOKE_WIDTH, SPOKE_LENGTH);
            rot_matrix = rotate(j * 120.0f + wheel_ang, vec3(1.0f, 0.0f, 0.0f));
            model_matrix = trans_matrix * rot_matrix * scale_matrix;
            draw_color_obj(Cube, SpokeColor);
        }
    }
}

void build_geometry( )
{
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;
    // Generate vertex arrays and buffers
    glGenVertexArrays(NumVAOs, VAOs);

    // TODO: Load models
    load_object(Cube);
    load_object(Cylinder);
    load_object(Cone);
    load_object(Torus);
    // Generate color buffers
    glGenBuffers(NumColorBuffers, ColorBuffers);

    // TODO: Create color buffers
    build_solid_color_buffer(numVertices[Cube], rail_color, RailColor);
    build_solid_color_buffer(numVertices[Cube], bottom_color, BottomColor);
    build_solid_color_buffer(numVertices[Cube], middle_color, MiddleColor);
    build_solid_color_buffer(numVertices[Cube], top_color, TopColor);
    build_solid_color_buffer(numVertices[Cube], body_color, BodyColor);
    build_solid_color_buffer(numVertices[Cube], eng_color, EngColor);
    build_solid_color_buffer(numVertices[Cube], ties_color, TiesColor);
    build_solid_color_buffer(numVertices[Torus], wheel_color, WheelColor);
    build_solid_color_buffer(numVertices[Cube], wheel_color, SpokeColor);
    build_solid_color_buffer(numVertices[Cylinder], stack_color, CylColor);
    build_solid_color_buffer(numVertices[Cone], stack_color, ConeColor);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // ESC to quit
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    // Toggle projection mode
    if (key == GLFW_KEY_O)
    {
        proj = ORTHOGRAPHIC;
    }
    else if (key == GLFW_KEY_P)
    {
        proj = PERSPECTIVE;
    }

    // TODO: Add keypress functionality
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

    // Adjust radius (zoom)
    if (key == GLFW_KEY_X)
    {
        radius += dr;
    }
    else if (key == GLFW_KEY_Z)
    {
        radius -= dr;
        if (radius < min_radius)
        {
            radius = min_radius;
        }
    }

    if (key == GLFW_KEY_O) {
        proj = ORTHOGRAPHIC;
    } else if (key == GLFW_KEY_P) {
        proj = PERSPECTIVE;
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        animate = !animate;
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
