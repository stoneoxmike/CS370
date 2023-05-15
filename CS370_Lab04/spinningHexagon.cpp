#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/utils.h"
#include "../common/vmath.h"

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Hexagon, NumVAOs};
enum Obj_Buffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum Color_Buffer_IDs {HexRed, NumColorBuffers};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint ColorBuffers[NumColorBuffers];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 2;
GLint colCoords = 4;

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

// Global state variables
mat4 model_matrix;
GLfloat hex_angle = 0.0;
GLfloat hex_x = 0.0;
GLfloat hex_y = 0.0;
GLfloat delta = 0.1;
GLdouble elTime = 0.0;
GLdouble rpm = 10.0;
GLint dir = 1;

void display( );
void render_scene( );
void build_geometry( );
void build_hexagon(GLuint obj);
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer);
void draw_color_object(GLuint obj, GLuint color);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Transform Hexagon");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // TODO: Register callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);
    // TODO: Get initial time
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
        //  TODO: Update angle based on time for fixed rpm
        GLdouble curTime = glfwGetTime();
        hex_angle += dir*rpm*(360.0/60.0)*(curTime - elTime);
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
	// Clear window
	glClear(GL_COLOR_BUFFER_BIT);

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

    // Draw hexagon
    // TODO: Set translation matrix
    trans_matrix = translate(hex_x, hex_y, 0.0f);
    // TODO: Set rotation matrix
    rot_matrix = rotate(hex_angle, 0.0f, 0.0f, 1.0f);
    scale_matrix = scale(0.5f, 0.5f, 1.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    draw_color_object(Hexagon, HexRed);
}

void build_geometry( )
{
    // Generate and bind vertex arrays
    glGenVertexArrays(NumVAOs, VAOs);

    // Generate color buffers
    glGenBuffers(NumColorBuffers, ColorBuffers);

    // Build color objects
    build_hexagon(Hexagon);

    // Build color buffer
    build_solid_color_buffer(numVertices[Hexagon], vec4(1.0f, 0.0f, 0.0f, 1.0f), HexRed);
}

void build_hexagon(GLuint obj) {
    vector<vec2> vertices;
    vector<ivec3> indices;
    vector<vec4> red;

    // Bind vertex array for obj
    glBindVertexArray(VAOs[obj]);

    // Define vertices (no particular orientation)
    vertices = {
            //{0.0f, 0.0f},  // non-convex but DOES work
            {1.0f, 0.0f},
            {0.5f, 0.866f},
            {-0.5f, 0.866f},
            {-1.0f, 0.0f},
            {-0.5f, -0.866f},
            {0.5f, -0.866f},
            //{0.0f, 0.0f}   // non-convex but DOESN'T work
    };

    // Define face indices (ensure proper orientation)
    indices = {
            {0, 1, 2},
            {2, 3, 4},
            {4, 5, 0},
            {0, 2, 4}
    };
    int numFaces = indices.size();

    // Create object vertices and colors from faces
    vector<vec2> obj_vertices;
    for (int i = 0; i < numFaces; i++) {
        for (int j = 0; j < 3; j++) {
            obj_vertices.push_back(vertices[indices[i][j]]);
        }
    }

    // Set numVertices as total number of INDICES
    numVertices[obj] = 3*numFaces;

    // Generate object buffers for obj
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);

    // Bind and load position object buffer for obj
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], obj_vertices.data(), GL_STATIC_DRAW);
}

void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer) {
    // Create object colors
    vector<vec4> obj_colors;
    for (int i = 0; i < num_vertices; i++) {
        obj_colors.push_back(color);
    }

    // Bind and load color buffers
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[buffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*num_vertices, obj_colors.data(), GL_STATIC_DRAW);
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

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Esc closes window
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    // TODO: Move hexagon with arrow keys
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        hex_y = hex_y + delta;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        hex_y = hex_y - delta;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        hex_x = hex_x - delta;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        hex_x = hex_x + delta;
    }
}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){
    // TODO: Flip spin direction with mouse click
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        dir = dir * -1;
    }
}

