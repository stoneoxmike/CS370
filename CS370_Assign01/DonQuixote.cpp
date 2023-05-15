// CS370 Assignment 1 - Don Quixote
// Fall 2022

#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "iostream"

using namespace vmath;
using namespace std;

#define NUM_SUN 361

// Vertex array and buffer names
enum VAO_IDs {Square, Triangle, Sun, NumVAOs};
enum Obj_Buffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum Color_Buffer_IDs {SkyBlue, GrassGreen, HouseBrown, RoofRed, FanBlue, SunYellow, White, DoorBrown, PathBrown, DoorKnob, NumColorBuffers};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint ColorBuffers[NumColorBuffers];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 2;
GLint colCoords = 4;
GLint ww,hh;

// Shader variables
// Shader program reference
GLuint trans_program;
// Shader component references
GLuint trans_vPos;
GLuint trans_vCol;
GLuint trans_model_mat_loc;
// Shader source files
const char *trans_vertex_shader = "../trans.vert";
const char *trans_frag_shader = "../trans.frag";

// Global state
mat4 model_matrix;
GLboolean anim = false;
GLfloat fan_angle = 0.0;
GLdouble elTime = 0.0;
GLdouble rpm = 10.0;
GLdouble sunRPM = 1.0;
GLfloat sunAngle = 0.0;
GLint dir = 1;

void display( );
void render_scene( );
void build_geometry( );
void build_square(GLuint obj);
void build_triangle(GLuint obj);
void build_sun(GLuint obj);
void draw_color_object(GLuint obj, GLuint color);
void draw_color_fan_object(GLuint obj, GLuint color);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Don Quixote 2022");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    glfwGetFramebufferSize(window, &ww, &hh);
    // TODO: Register callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // Get initial time
    elTime = glfwGetTime();

	// Create geometry buffers
    build_geometry();
    
    // Load shaders and associate shader variables
	ShaderInfo trans_shaders[] = { {GL_VERTEX_SHADER, trans_vertex_shader},{GL_FRAGMENT_SHADER, trans_frag_shader},{GL_NONE, NULL} };
	trans_program = LoadShaders(trans_shaders);
	trans_vPos = glGetAttribLocation(trans_program, "vPosition");
    trans_vCol = glGetAttribLocation(trans_program, "vColor");
    trans_model_mat_loc = glGetUniformLocation(trans_program, "model_matrix");

	// Start loop
    while ( !glfwWindowShouldClose( window ) ) {
    	// Draw graphics
        display();
        // Update other events like input handling
        glfwPollEvents();
        //  TODO: Update angle based on time for fixed rpm when animating
        GLdouble curTime = glfwGetTime();
        sunAngle += sunRPM * (360.0 / 60.0) * (curTime - elTime);
        if (anim) {
            fan_angle += dir * rpm * (360.0 / 60.0) * (curTime - elTime);
        }
        elTime = curTime;
        // Swap buffer onto screen
        glfwSwapBuffers(window);
    }

    // Close window
    glfwTerminate();
    return 0;
}

void display( )
{
	// Clear window
	glClear(GL_COLOR_BUFFER_BIT);

    // Render objects
	render_scene();

	// Flush pipeline
	glFlush();
}

void render_scene( ) {
    model_matrix = mat4().identity();

    // TODO: Declare transformation matrices
    model_matrix = translate(0.0f, 0.25f, 0.0f)*scale(1.0f, 0.75f, 1.0f);
    // TODO: Draw sky
    draw_color_object(Square, SkyBlue);
    model_matrix = rotate(sunAngle, vec3(0.0f, 0.0f, 1.0f))*translate(-0.5f, 0.5f, 0.0f)*scale(0.1f, 0.1f, 1.0f);
    draw_color_fan_object(Sun, SunYellow);
    model_matrix = rotate(sunAngle, vec3(0.0f, 0.0f, 1.0f))*translate(0.5f, -0.5f, 0.0f)*scale(0.1f, 0.1f, 1.0f);
    draw_color_fan_object(Sun, White);
    // TODO: Draw grass
    model_matrix = translate(0.0f, -0.75f, 0.0f)*scale(1.0f, 0.25f, 1.0f);
    draw_color_object(Square, GrassGreen);
    // TODO: Draw house
    model_matrix = translate(0.0f, -0.3f, 0.0f)*scale(0.25f, 0.25f, 1.0f);
    draw_color_object(Square, HouseBrown);
    model_matrix = translate(0.0f, -0.375f, 0.0f)*scale(0.075f, 0.175f, 1.0f);
    draw_color_object(Square, DoorBrown);
    model_matrix = translate(0.0f, -0.8f, 0.0f)*scale(0.1f, 0.25f, 1.0f);
    draw_color_object(Square, PathBrown);
    model_matrix = translate(0.04f, -0.4f, 0.0f)*scale(0.02f, 0.02f, 1.0f);
    draw_color_fan_object(Sun, DoorKnob);
    // TODO: Draw roof
    model_matrix = translate(0.0f, -0.05f, 0.0f)*rotate(-45.0f, vec3(0.0f, 0.0f, 1.0f))*scale(0.177f, 0.177f, 1.0f);
    draw_color_object(Triangle, RoofRed);
    // TODO: Draw fan
    model_matrix = translate(0.0f, -0.05f, 0.0f)*scale(0.03f, 0.177f, 1.0f)*rotate(-45.0f, vec3(0.0f, 0.0f, 1.0f));
    model_matrix = translate(0.0f, 0.2f, 0.0f)*rotate(fan_angle, vec3(0.0f, 0.0f, 1.0f))*translate(0.0f, -0.2f, 0.0f)*model_matrix;
    draw_color_object(Triangle, FanBlue);
    model_matrix = translate(-0.177f, 0.3f, 0.0f)*rotate(-120.0f,vec3(0.0f, 0.0f, 1.0f))*translate(0.0f, -0.05f, 0.0f)*scale(0.03f, 0.177f, 1.0f)*rotate(-45.0f, vec3(0.0f, 0.0f, 1.0f));
    model_matrix = translate(0.0f, 0.2f, 0.0f)*rotate(fan_angle, vec3(0.0f, 0.0f, 1.0f))*translate(0.0f, -0.2f, 0.0f)*model_matrix;
    draw_color_object(Triangle, FanBlue);
    model_matrix = translate(0.177f, 0.3f, 0.0f)*rotate(-240.0f,vec3(0.0f, 0.0f, 1.0f))*translate(0.0f, -0.05f, 0.0f)*scale(0.03f, 0.177f, 1.0f)*rotate(-45.0f, vec3(0.0f, 0.0f, 1.0f));
    model_matrix = translate(0.0f, 0.2f, 0.0f)*rotate(fan_angle, vec3(0.0f, 0.0f, 1.0f))*translate(0.0f, -0.2f, 0.0f)*model_matrix;
    draw_color_object(Triangle, FanBlue);
    // TODO: Draw sun using draw_color_fan_object
}

void build_geometry( )
{
    // Generate vertex arrays
	glGenVertexArrays(NumVAOs, VAOs);

    // Generate color buffers
    glGenBuffers(NumColorBuffers, ColorBuffers);

    // Build squares
    build_square(Square);

    // Build triangles
    build_triangle(Triangle);

    // Build sun
    build_sun(Sun);
}

void build_square(GLuint obj) {
    vector<vec2> vertices;
    vector<ivec3> indices;
    vector<vec4> blue_grad, green_grad, brown, door, path;

    // Bind square
    glBindVertexArray(VAOs[obj]);

    // Define square vertices
    vertices = {
            { 1.0f, 1.0f},
            {-1.0f, 1.0f},
            {-1.0f,-1.0f},
            { 1.0f,-1.0f},
    };

    // TODO: Define square face indices (ensure proper orientation)
    indices = {
            {0, 1, 2},
            {0, 2, 3},
    };
    int numFaces = indices.size();

    // TODO: Define blue sky color
    blue_grad = {
            {0.0f, 0.0f, 1.0f, 1.0f},
            {0.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f},
    };

    // TODO: Define green grass color
    green_grad = {
            {0.0f, 0.5f, 0.0f, 1.0f},
            {0.0f, 0.5f, 0.0f, 1.0f},
            {0.0f, 1.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 0.0f, 1.0f},
    };

    // TODO: Define brown house color
    brown = {
            {0.5, 0.25, 0.0f, 1.0f},
            {0.5, 0.25, 0.0f, 1.0f},
            {0.5, 0.25, 0.0f, 1.0f},
            {0.5, 0.25, 0.0f, 1.0f},
    };

    door = {
            {0.8f, 0.2f, 0.4f, 1.0f},
            {0.8f, 0.2f, 0.4f, 1.0f},
            {0.8f, 0.2f, 0.4f, 1.0f},
            {0.8f, 0.2f, 0.4f, 1.0f},
    };

    path = {
            {0.825f, 0.4f, 0.1f, 1.0f},
            {0.825f, 0.4f, 0.1f, 1.0f},
            {0.825f, 0.4f, 0.1f, 1.0f},
            {0.825f, 0.4f, 0.1f, 1.0f},
    };

    // TODO: Create object vertices and colors from faces

    vector<vec2> obj_vertices;
    vector<vec4> obj_blue_grad, obj_green_grad, obj_brown, obj_door, obj_path;
    for (int i = 0; i < numFaces; i++) {
        for (int j = 0; j < 3; j++) {
            obj_vertices.push_back(vertices[indices[i][j]]);
            obj_blue_grad.push_back(blue_grad[indices[i][j]]);
            obj_green_grad.push_back(green_grad[indices[i][j]]);
            obj_brown.push_back(brown[indices[i][j]]);
            obj_door.push_back(door[indices[i][j]]);
            obj_path.push_back(path[indices[i][j]]);
        }
    }

    // Set numVertices as total number of INDICES
    numVertices[obj] = 3*numFaces;

    // Generate object buffers for obj
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);

    // TODO: Bind and load position object buffer for obj
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], obj_vertices.data(), GL_STATIC_DRAW);
    // TODO: Bind and load color buffers
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[SkyBlue]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[obj], obj_blue_grad.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[GrassGreen]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[obj], obj_green_grad.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[HouseBrown]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[obj], obj_brown.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[DoorBrown]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[obj], obj_door.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[PathBrown]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[obj], obj_path.data(), GL_STATIC_DRAW);
}

void build_triangle(GLuint obj) {
    vector<vec2> vertices;
    vector<ivec3> indices;
    vector<vec4> red, blue_grad;

    // Bind vertex array for obj
    glBindVertexArray(VAOs[obj]);

    // Define triangle vertices
    vertices = {
            {1.0f, 1.0f},
            {-1.0f, 1.0f},
            {-1.0f,-1.0f},
    };

    // TODO: Define triangle indices (ensure proper orientation)
    indices = {
            {0, 1, 2},
    };
    int numFaces = indices.size();

    // TODO: Define red roof color
    red = {
            {1.0f, 0.0f, 0.0f, 1.0f},
            {1.0f, 0.0f, 0.0f, 1.0f},
            {1.0f, 0.0f, 0.0f, 1.0f},
    };

    // TODO: Define blue fan color
    blue_grad = {
            {0.0f, 0.0f, 1.0f, 1.0f},
            {0.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f},
    };

    // TODO: Create object vertices and colors from faces
    vector<vec2> obj_vertices;
    vector<vec4> obj_red, obj_blue_grad;
    for (int i = 0; i < numFaces; i++) {
        for (int j = 0; j < 3; j++) {
            obj_vertices.push_back(vertices[indices[i][j]]);
            obj_red.push_back(red[indices[i][j]]);
            obj_blue_grad.push_back(blue_grad[indices[i][j]]);
        }
    }

    // Set numVertices as total number of INDICES (3*number of faces)
    numVertices[obj] = 3*numFaces;

    // Generate object buffers for obj
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);

    // TODO: Bind and load position object buffer for obj
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], obj_vertices.data(), GL_STATIC_DRAW);
    // TODO: Bind and load color buffer
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[RoofRed]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[obj], obj_red.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[FanBlue]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[obj], obj_blue_grad.data(), GL_STATIC_DRAW);
}

void build_sun(GLuint obj) {
    vector<vec2> vertices;
    vector<ivec3> indices;
    vector<vec4> yellow_grad, white, knob;

    // Bind vertex array for obj
    glBindVertexArray(VAOs[obj]);

    // TODO: Define sun vertices and colors for trianle fan
    vertices.push_back({0, 0});
    for (int i = 0; i < NUM_SUN; i++) {
        float val = 2 * M_PI * i / NUM_SUN;
        vertices.push_back({cos(val), sin(val)});
    }
    for (int i = 1; i < NUM_SUN; i++) {
        if (i == NUM_SUN - 1) {
            indices.push_back({0, i, 1});
        } else {
            indices.push_back({0, i, i + 1});
        }
    }
    int numFaces = indices.size();
    yellow_grad.push_back({1.0f, 1.0f, 1.0f, 1.0f});
    for (int i = 0; i < NUM_SUN; i++) {
        yellow_grad.push_back({1.0f, 0.8f, 0.0f, 1.0f});
    }
    for (int i = 0; i < NUM_SUN; i++) {
        white.push_back({1.0f, 1.0f, 1.0f, 1.0f});
    }
    knob.push_back({1.0f, 1.0f, 1.0f, 1.0f});
    for (int i = 0; i < NUM_SUN; i++) {
        knob.push_back({0.0f, 0.0f, 0.0f, 1.0f});
    }
    // TODO: Set numVertices for sun
    vector<vec2> obj_vertices;
    vector<vec4> obj_yellow_grad, obj_white, obj_knob;
    for (int i = 0; i < numFaces; i++) {
        for (int j = 0; j < 3; j++) {
            obj_vertices.push_back(vertices[indices[i][j]]);
            obj_yellow_grad.push_back(yellow_grad[indices[i][j]]);
            obj_white.push_back(white[indices[i][j]]);
            obj_knob.push_back(knob[indices[i][j]]);
        }
    }

    numVertices[obj] = 3*numFaces;
    // Generate object buffers for obj
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);

    // TODO: Bind and load position object buffer for obj
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], obj_vertices.data(), GL_STATIC_DRAW);
    // TODO: Bind and load color buffer
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[SunYellow]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[obj], obj_yellow_grad.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[White]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[obj], obj_white.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[DoorKnob]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[obj], obj_knob.data(), GL_STATIC_DRAW);
}

void draw_color_object(GLuint obj, GLuint color) {
    // Select shader program
    glUseProgram(trans_program);

    // Pass model matrix to shader
    glUniformMatrix4fv(trans_model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(trans_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(trans_vPos);

    // Bind color buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[color]);
    glVertexAttribPointer(trans_vCol, colCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(trans_vCol);

    // Draw geometry
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_color_fan_object(GLuint obj, GLuint color) {
    // Select shader program
    glUseProgram(trans_program);

    // Pass model matrix to shader
    glUniformMatrix4fv(trans_model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(trans_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(trans_vPos);

    // Bind color buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[color]);
    glVertexAttribPointer(trans_vCol, colCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(trans_vCol);

    // Draw geometry
    glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices[obj]);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Esc closes window
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    // TODO: Start/Stop animation with spacebar
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        anim = !anim;
    }
}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){
    // TODO: Flip spin direction with mouse click
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        dir = dir * -1;
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    // TODO: Store new window sizes in global variables
    glfwGetFramebufferSize(window, &ww, &hh);
}

