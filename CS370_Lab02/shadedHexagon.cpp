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
enum Color_Buffer_IDs {HexGradient, NumColorBuffers};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint ColorBuffers[NumColorBuffers];

// Number of indices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 2;
GLint colCoords = 4;

// Shader variables
// Shader program reference
GLuint color_program;
// Shader component references
GLuint color_vPos;
GLuint color_vCol;
// Shader source files
const char *color_vertex_shader = "../color.vert";
const char *color_frag_shader = "../color.frag";

void display( );
void render_scene( );
void build_geometry( );
void build_hexagon(GLuint obj);
void draw_color_object(GLuint obj, GLuint color);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Shaded Hexagon");
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
	ShaderInfo color_shaders[] = { {GL_VERTEX_SHADER, color_vertex_shader},{GL_FRAGMENT_SHADER, color_frag_shader},{GL_NONE, NULL} };
	color_program = LoadShaders(color_shaders);
	color_vPos = glGetAttribLocation(color_program, "vPosition");
    color_vCol = glGetAttribLocation(color_program, "vColor");

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
	// Clear window
	glClear(GL_COLOR_BUFFER_BIT);

	// Render objects
	render_scene();

	// Flush pipeline
	glFlush();
}

void render_scene() {
    draw_color_object(Hexagon, HexGradient);
}

void build_geometry( )
{
	// Generate vertex arrays
	glGenVertexArrays(NumVAOs, VAOs);

    // Generate color buffers
    glGenBuffers(NumColorBuffers, ColorBuffers);

    // TODO: Build hexagon with gradient color
    build_hexagon(Hexagon);
}

void build_hexagon(GLuint obj) {
    vector<vec2> vertices;
    vector<ivec3> indices;
    vector<vec4> colors;

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

    // TODO: Define face indices (ensure proper orientation)
    indices = {
            {0, 1, 2},
            {2, 3, 4},
            {4, 5, 0},
            {0, 2, 4},
    };
    int numFaces = indices.size();

    // TODO: Define colors per vertex
    colors = {
            {0.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 0.0f, 0.0f, 1.0f},
            {1.0f, 0.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f, 1.0f},
    };

    // Create object vertices from faces
    vector<vec2> obj_vertices;
    vector<vec4> obj_colors;
    for (int i = 0; i < numFaces; i++) {
        for (int j = 0; j < 3; j++) {
            obj_vertices.push_back(vertices[indices[i][j]]);
            obj_colors.push_back(colors[indices[i][j]]);
        }
    }

    // TODO: Set numVertices as total number of INDICES (3*number of faces)
    numVertices[obj] = numFaces*3;
    // Generate object buffers for obj
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);

    // Bind and load position object buffer for obj
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], obj_vertices.data(), GL_STATIC_DRAW);

    // TODO: Bind and load color buffer
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[obj]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[obj], obj_colors.data(), GL_STATIC_DRAW);
}

void draw_color_object(GLuint obj, GLuint color) {
    // Select shader program
    glUseProgram(color_program);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(color_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(color_vPos);

    // TODO: Bind color buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[obj]);
    glVertexAttribPointer(color_vCol, colCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(color_vCol);
    // Draw geometry
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}
