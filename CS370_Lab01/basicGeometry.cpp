#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/utils.h"
#include "../common/vmath.h"

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Square, NumVAOs};
enum Obj_Buffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of position coordinates
GLint posCoords = 2;

// Shader variables
// Shader program reference - basic objects
GLuint basic_program;
// Shader component references
GLuint basic_vPos;
// Shader source files
const char *basic_vertex_shader = "../basic.vert";
const char *basic_frag_shader = "../basic.frag";

void display( );
void render_scene( );
void build_geometry( );
void build_square(GLuint obj);
void draw_basic_object(GLuint obj);

int main(int argc, char**argv)
{
    // Create OpenGL window
    GLFWwindow* window = CreateWindow("Basic Geometry");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // Create geometry buffers
    build_geometry();

    // Load shaders and associate shader variables
    ShaderInfo basic_shaders[] = { {GL_VERTEX_SHADER, basic_vertex_shader},{GL_FRAGMENT_SHADER, basic_frag_shader},{GL_NONE, NULL} };
    basic_program = LoadShaders(basic_shaders);
    basic_vPos = glGetAttribLocation(basic_program, "vPosition");

    // Start loop
    while ( !glfwWindowShouldClose( window ) ) {
        // Draw graphics
        display();
        // Update other events like input handling
        glfwPollEvents();
        // Swap buffer onto screen
        glfwSwapBuffers( window );
    }

    // Close window
    glfwTerminate();
    return 0;
}


void display( )
{
    // TODO: Clear window
    glClear(GL_COLOR_BUFFER_BIT);
    // TODO: Render objects
    render_scene();
    // TODO: Flush pipeline
    glFlush();
}

void render_scene( ) {
    // TODO: Draw basic object
    draw_basic_object(Square);
}

void build_geometry( )
{
    // TODO: Generate vertex arrays
    glGenVertexArrays(NumVAOs, VAOs);
    // TODO: Build square object
    build_square(Square);
}

void build_square(GLuint obj) {
    vector<vec2> vertices;

    // TODO: Bind vertex array for obj
    glBindVertexArray(VAOs[obj]);
    // TODO: Define vertices (ensure proper orientation)
    vertices = {
            {-0.5f,  -0.5f},
            {0.5f,   0.5f},
            {-0.5f,  0.5f},
            {-0.50f, -0.5f},
            {0.5f,   -0.5f},
            {0.5f,   0.5f},
    };
    // TODO: Store number of vertices
    numVertices[obj] = vertices.size();
    // TODO: Generate object buffers for obj
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    // TODO: Bind and load position object buffer for obj
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
}

void draw_basic_object(GLuint obj) {
    // TODO: Select shader program
    glUseProgram(basic_program);
    // TODO: Bind vertex array
    glBindVertexArray(VAOs[obj]);
    // TODO: Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(basic_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(basic_vPos);
    // TODO: Draw geometry
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

