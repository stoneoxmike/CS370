#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"	// Sean Barrett's image loader - http://nothings.org/
#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {TexSphere, Background, NumVAOs};
enum Obj_Buffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum Textures {Earth, Moon, Space, NumTextures};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint TextureIDs[NumTextures];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint colCoords = 4;

// Model files
vector<const char *> objFiles = {"../models/uv_sphere.obj"};

// Texture files
vector<const char *> texFiles = {"../textures/earth.bmp", "../textures/moon.bmp", "../textures/space.jpg"};

// Camera
vec3 eye = {0.0f, 0.0f, 4.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};

// Shader variables
// Shader program reference
GLuint texture_program;
GLuint texture_vPos;
GLuint texture_vTex;
GLuint texture_proj_mat_loc;
GLuint texture_camera_mat_loc;
GLuint texture_model_mat_loc;
const char *texture_vertex_shader = "../texture.vert";
const char *texture_frag_shader = "../texture.frag";

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 model_matrix;

// Global sphere variables
GLfloat earth_angle = 0.0f;
GLfloat moon_angle = 0.0f;
GLdouble elTime = 0.0;
GLdouble rpm = 10.0;
vec3 axis = {0.0f, 1.0f, 0.0f};

// Global screen dimensions
GLint ww,hh;

void display( );
void render_scene( );
void build_geometry( );
void build_textures();
void build_background(GLuint obj);
void draw_background();
void load_object(GLuint obj);
void draw_tex_object(GLuint obj, GLuint texture);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Earth Moon");
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
    // Create textures
    build_textures();
    
    // Load shaders
	ShaderInfo texture_shaders[] = { {GL_VERTEX_SHADER, texture_vertex_shader},{GL_FRAGMENT_SHADER, texture_frag_shader},{GL_NONE, NULL} };
    texture_program = LoadShaders(texture_shaders);
    texture_vPos = glGetAttribLocation(texture_program, "vPosition");
    texture_vTex = glGetAttribLocation(texture_program, "vTexCoord");
    texture_proj_mat_loc = glGetUniformLocation(texture_program, "proj_matrix");
    texture_camera_mat_loc = glGetUniformLocation(texture_program, "camera_matrix");
    texture_model_mat_loc = glGetUniformLocation(texture_program, "model_matrix");

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
        earth_angle += (curTime-elTime)*(rpm/60.0)*360.0;
        moon_angle += (curTime-elTime)*(rpm/60.0)*360.0;
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

    // Render background
	draw_background();

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
	mat4 rot2_matrix = mat4().identity();


    // Set earth transformation matrix
    trans_matrix = translation(0.0f, 0.0f, 0.0f);
    rot_matrix = rotation(earth_angle, normalize(axis));
    scale_matrix = scale(1.0f, 1.0f, 1.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // Draw earth
    draw_tex_object(TexSphere, Earth);

    // Set moon transformation matrix
    rot_matrix = rotation(moon_angle, normalize(axis));
    scale_matrix = scale(0.25f, 0.25f, 0.25f);
    trans_matrix = translation(2.0f, 0.0f, 0.0f);
    model_matrix = rot_matrix*trans_matrix*rot_matrix*scale_matrix;
    // Draw moon
    draw_tex_object(TexSphere, Moon);
}

void build_geometry( )
{
    // Generate vertex arrays for objects
    glGenVertexArrays(NumVAOs, VAOs);

    // Load objects
    load_object(TexSphere);

    // Build background
    build_background(Background);
}

void build_textures( ) {
    int w, h, n;
    int force_channels = 4;
    unsigned char *image_data;

    // Create textures and activate unit 0
    glGenTextures( texFiles.size(),  TextureIDs);
    glActiveTexture( GL_TEXTURE0 );

    for (int i = 0; i < texFiles.size(); i++) {
        // Load image from file
        image_data = stbi_load(texFiles[i], &w, &h, &n, force_channels);
        if (!image_data) {
            fprintf(stderr, "ERROR: could not load %s\n", texFiles[i]);
        }
        // NPOT check for power of 2 dimensions
        if ((w & (w - 1)) != 0 || (h & (h - 1)) != 0) {
            fprintf(stderr, "WARNING: texture %s is not power-of-2 dimensions\n",
                    texFiles[i]);
        }

        // TODO: Bind current texture id
        glBindTexture(GL_TEXTURE_2D, TextureIDs[i]);
        // TODO: Load image data into texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     image_data);
        // TODO: Generate mipmaps for texture
        glGenerateMipmap( GL_TEXTURE_2D );
        // TODO: Set scaling modes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // TODO: Set wrapping modes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Set maximum anisotropic filtering for system
        GLfloat max_aniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
        // set the maximum!
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
    }
}

void build_background(GLuint obj) {
    vector<vec4> vertices;
    vector<vec2> uvCoords;

    // Define background vertices
    vertices = {
            {1.0f, 1.0f, 0.0f, 1.0f},
            {-1.0f, 1.0f, 0.0f, 1.0f},
            {-1.0f, -1.0f, 0.0f, 1.0f},
            {-1.0f, -1.0f, 0.0f, 1.0f},
            {1.0f, -1.0f, 0.0f, 1.0f},
            {1.0f, 1.0f, 0.0f, 1.0f},
    };

    // TODO: Define texture coordinates
    uvCoords = {
            {2.0f, 2.0f},
            {-1.0f, 2.0f},
            {-1.0f, -1.0f},
            {-1.0f, -1.0f},
            {2.0f, -1.0f},
            {2.0f, 2.0f},
    };

    numVertices[Background] = vertices.size();

    glBindVertexArray(VAOs[Background]);
    glGenBuffers(NumObjBuffers, ObjBuffers[Background]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Background][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[Background], vertices.data(), GL_STATIC_DRAW);
    // Bind texture coordinate buffer and load data
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Background][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texCoords*numVertices[Background], uvCoords.data(), GL_STATIC_DRAW);

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

void draw_tex_object(GLuint obj, GLuint texture){
    // Select shader program
    glUseProgram(texture_program);

    // Pass projection matrix to shader
    glUniformMatrix4fv(texture_proj_mat_loc, 1, GL_FALSE, proj_matrix);

    // Pass camera matrix to shader
    glUniformMatrix4fv(texture_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Pass model matrix to shader
    glUniformMatrix4fv(texture_model_mat_loc, 1, GL_FALSE, model_matrix);

    // TODO: Bind texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[texture]);
    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(texture_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(texture_vPos);

    // Bind texture object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glVertexAttribPointer(texture_vTex, texCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(texture_vTex);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_background(){
    // Set anisotropic scaling
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
    // TODO: Set default orthographic projection
    proj_matrix = ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    // TODO: Set default camera matrix
    camera_matrix = mat4().identity();

    // TODO: Set default model matrix
    model_matrix = mat4().identity();

    // TODO: Draw background with depth buffer writes disabled
    glDepthMask(GL_FALSE);
    draw_tex_object(Background, Space);
    glDepthMask(GL_TRUE);

}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Move hexagon with arrow keys
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