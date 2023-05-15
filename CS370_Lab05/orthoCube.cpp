#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/utils.h"
#include "../common/vmath.h"

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, NumVAOs};
enum Obj_Buffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum Color_Buffer_IDs {CubeGradient, NumColorBuffers};

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

// Shader variables
// Shader program reference
GLuint proj_program;
// Shader component references
GLuint proj_vPos;
GLuint proj_vCol;
GLuint proj_proj_mat_loc;
GLuint proj_model_mat_loc;
// Shader source files
const char *proj_vertex_shader = "../proj.vert";
const char *proj_frag_shader = "../proj.frag";

// Global state
mat4 proj_matrix;
mat4 model_matrix;
GLfloat cube_angle = 0.0;
GLdouble elTime = 0.0;
GLdouble rpm = 10.0;
vec3 axis = {1.0f, 1.0f, 1.0f};

// Global screen dimensions
GLint ww,hh;

void display( );
void render_scene( );
void build_geometry( );
void build_cube(GLuint obj);
void draw_color_object(GLuint obj, GLuint color);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Ortho Cube");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // TODO: Store initial window size in global variables
    glfwGetFramebufferSize(window, &ww, &hh);

    // Register callbacks
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);
    // TODO: Register resize callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // Get initial time
    elTime = glfwGetTime();

    // Create geometry buffers
    build_geometry();

    // Load shaders and associate shader variables
    ShaderInfo proj_shaders[] = { {GL_VERTEX_SHADER, proj_vertex_shader},{GL_FRAGMENT_SHADER, proj_frag_shader},{GL_NONE, NULL} };
    proj_program = LoadShaders(proj_shaders);
    proj_vPos = glGetAttribLocation(proj_program, "vPosition");
    proj_vCol = glGetAttribLocation(proj_program, "vColor");
    proj_proj_mat_loc = glGetUniformLocation(proj_program, "proj_matrix");
    proj_model_mat_loc = glGetUniformLocation(proj_program, "model_matrix");

    // TODO: Enable depth test
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
	// Start loop
    while ( !glfwWindowShouldClose( window ) ) {
    	// Draw graphics
        display();
        // Update other events like input handling
        glfwPollEvents();
        // Update angle based on time for fixed rpm
        GLdouble curTime = glfwGetTime();
        cube_angle += (curTime-elTime)*(rpm/60.0)*360.0;
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
    // Declare projection matrix
    proj_matrix = mat4().identity();

	// TODO: Clear window and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Compute anisotropic scaling
    // Adjust viewing volume (orthographic)

    GLfloat xratio = 1.0f;
    GLfloat yratio = 1.0f;
    // If taller than wide adjust y
    if(ww <= hh)
    {
        yratio = (GLfloat) hh/ (GLfloat) ww;
    }
        // If wider than tall adjust x
    else if (hh <= ww)
    {
        xratio = (GLfloat) ww/ (GLfloat) hh;
    }
    // TODO: Set projection matrix
    proj_matrix = ortho(-2.0f*xratio, 2.0f*xratio, -2.0f*yratio, 2.0f*yratio, 2.0f, -2.0f);
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

	// Draw cube
    trans_matrix = translate(vec3(0.0f, 0.0f, 0.0f));
    rot_matrix = rotate(cube_angle, normalize(axis));
    scale_matrix = scale(vec3(1.0f, 1.0f, 1.0f));
	model_matrix = trans_matrix*rot_matrix*scale_matrix;
    draw_color_object(Cube, CubeGradient);
}

void build_geometry( )
{
    // Generate and bind vertex arrays
    glGenVertexArrays(NumVAOs, VAOs);

    // Generate color buffers
    glGenBuffers(NumColorBuffers, ColorBuffers);

    // Build cube with gradient color
    build_cube(Cube);
}

void build_cube(GLuint obj) {
    vector<vec4> vertices;
    vector<ivec3> indices;
    vector<vec4> colors;

    // Bind vertex array for obj
    glBindVertexArray(VAOs[obj]);

    // Define 3D (homogeneous) vertices for cube
    vertices = {
            {-0.5f, -0.5f, -0.5f, 1.0f},
            {0.5f, -0.5f, -0.5f, 1.0f},
            {0.5f, -0.5f, 0.5f, 1.0f},
            {-0.5f, -0.5f, 0.5f, 1.0f},
            {-0.5f, 0.5f, -0.5f, 1.0f},
            {0.5f, 0.5f, -0.5f, 1.0f},
            {0.5f, 0.5f, 0.5f, 1.0f},
            {-0.5f, 0.5f, 0.5f, 1.0f}
    };

    // TODO: Define face indices (ensure proper orientation)
    indices = {
            {0, 4, 1},
            {4, 5, 1},
            {5, 4, 6},
            {6, 4, 7},
            {7, 0, 3},
            {7, 4, 0},
            {1, 6, 2},
            {5, 6, 1},
            {6, 3, 2},
            {6, 7, 3},
            {2, 3, 0},
            {2, 0, 1}
    };
    int numFaces = indices.size();

    // Define vertex colors
    colors = {
            {0.0f, 0.0f, 0.0f, 1.0f},
            {1.0f, 0.0f, 0.0f, 1.0f},
            {1.0f, 0.0f, 1.0f, 1.0f},
            {0.0f, 0.0f, 1.0f, 1.0f},
            {0.0f, 1.0f, 0.0f, 1.0f},
            {1.0f, 1.0f, 0.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f},
            {0.0f, 1.0f, 1.0f, 1.0f}
    };

    // Create object vertices and colors from faces
    vector<vec4> obj_vertices;
    vector<vec4> obj_colors_red, obj_colors_green, obj_colors_blue, obj_colors_yellow;
    for (int i = 0; i < numFaces; i++) {
        for (int j = 0; j < 3; j++) {
            obj_vertices.push_back(vertices[indices[i][j]]);
            obj_colors_red.push_back(colors[indices[i][j]]);
        }
    }

    // Set numVertices as total number of INDICES
    numVertices[obj] = 3*numFaces;

    // Generate object buffers for obj
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);

    // Bind and load position object buffer for obj
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], obj_vertices.data(), GL_STATIC_DRAW);

    // Bind and load color buffers
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[CubeGradient]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[obj], obj_colors_red.data(), GL_STATIC_DRAW);
}

void draw_color_object(GLuint obj, GLuint color) {
    // Select shader program
    glUseProgram(proj_program);

    // Pass projection matrix to shader
    glUniformMatrix4fv(proj_proj_mat_loc, 1, GL_FALSE, proj_matrix);

    // Pass model matrix to shader
    glUniformMatrix4fv(proj_model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(proj_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(proj_vPos);

    // Bind color buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[color]);
    glVertexAttribPointer(proj_vCol, colCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(proj_vCol);

    // Draw geometry
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Esc closes window
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }
}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
    // TODO: Store new window sizes in global variables
    glfwGetFramebufferSize(window, &ww, &hh);
}
