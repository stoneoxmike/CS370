// CS370 Final Project
// Fall 2022

#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"	// Sean Barrett's image loader - http://nothings.org/
#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "lighting.h"
#define DEG2RAD (M_PI/180.0)

using namespace vmath;
using namespace std;

// Vertex array and buffer names
//enum VAO_IDs {Cube, Chair, Desk, Glass, Fruit, NumVAOs};
enum VAO_IDs {Cube, Table, Chair, Fruit, Glass, Switch, Can, Sphere, Fan, Mirror, Frame, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum Color_Buffer_IDs {RedCube, Paint, SolidChair, SolidDesk, SolidFruit, SolidGlass, SolidSwitch, SolidCan, SolidSphere, NumColorBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {Orange, WhitePaint, GreyPaint, Brass, Wood, Acrylic, RedPlastic};
enum Textures {Blank, Carpet, Door, Outside, Art, MirrorTex, NumTextures};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint ColorBuffers[NumColorBuffers];
GLuint LightBuffers[NumLightBuffers];
GLuint MaterialBuffers[NumMaterialBuffers];
//GLuint TextureIDs[NumTextures];
GLuint TextureIDs[NumTextures];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint colCoords = 4;

// Model files
//vector<const char *> objFiles = {"../models/unitcube.obj", "../models/chair.obj", "../models/desk.obj", "../models/glass.obj", "../models/Avocados.obj"};
vector<const char *> objFiles = {"../models/unitcube.obj", "../models/table.obj", "../models/chair.obj", "../models/Avocados.obj", "../models/Glasses.obj", "../models/switch.obj", "../models/soda_can.obj", "../models/sphere.obj", "../models/CeilingFanLamp.obj", "../models/plane.obj"};

// Texture files
//vector<const char *> texFiles = {"../textures/blank.png", "../textures/white-texture.jpg"};
vector<const char *> texFiles = {"../textures/blank.png", "../textures/carpet.png", "../textures/door.jpg", "../textures/outside.jpg", "../textures/art.jpg"};

// Camera
vec3 eye = {0.0f, 5.0f, 0.0f};
vec3 center = {7.5f, 5.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};
vec3 mirror_eye = {0.0f, 2.5f, 7.25f};
vec3 mirror_center = {0.0f, 2.5f, 0.0f};
vec3 mirror_up = {0.0f, 1.0f, 0.0f};
GLfloat azimuth = 0.0f;
GLfloat daz = 1.0f;
GLfloat elevation = 0.0f;
GLfloat del = 0.01f;
GLfloat radius = 7.5f;
GLfloat dr = 0.1f;
GLfloat min_radius = 2.0f;
vec3 dir = center - eye;
GLfloat centerX;
GLfloat centerY;
GLfloat centerZ;

// Global object variables
GLboolean fan = true;
GLfloat fan_angle = 0.0;
GLdouble elTime = 0.0;
GLdouble rpm = 10.0;
vec3 axis = {0.0f, 1.0f, 0.0f};

// Shader variables
// Default (color) shader program references
GLuint default_program;
GLuint default_vPos;
GLuint default_vCol;
GLuint default_proj_mat_loc;
GLuint default_cam_mat_loc;
GLuint default_model_mat_loc;
const char *default_vertex_shader = "../default.vert";
const char *default_frag_shader = "../default.frag";


// Lighting shader program reference
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

// Texture shader program reference
GLuint texture_program;
GLuint texture_vPos;
GLuint texture_vTex;
GLuint texture_proj_mat_loc;
GLuint texture_camera_mat_loc;
GLuint texture_model_mat_loc;
const char *texture_vertex_shader = "../texture.vert";
const char *texture_frag_shader = "../texture.frag";

// Debug mirror shader
GLuint debug_mirror_program;
const char *debug_mirror_vertex_shader = "../debugMirror.vert";
const char *debug_mirror_frag_shader = "../debugMirror.frag";
GLboolean mirror = false;

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 normal_matrix;
mat4 model_matrix;

vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights = 0;
GLint lightOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Global screen dimensions
GLint ww,hh;

void display();
void render_scene();
void create_mirror();
void build_geometry();
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer);
void build_materials( );
void build_lights( );
void build_textures();
void build_mirror();
void build_frame(GLuint obj);
void load_object(GLuint obj);
void draw_color_obj(GLuint obj, GLuint color);
void draw_mat_object(GLuint obj, GLuint material);
void draw_tex_object(GLuint obj, GLuint texture);
void draw_frame(GLuint obj);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Think Inside The Box");
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
    // Create textures
    build_textures();
    // Build Mirror
    build_mirror();

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

    // Load texture shaders
    ShaderInfo texture_shaders[] = { {GL_VERTEX_SHADER, texture_vertex_shader},{GL_FRAGMENT_SHADER, texture_frag_shader},{GL_NONE, NULL} };
    texture_program = LoadShaders(texture_shaders);
    texture_vPos = glGetAttribLocation(texture_program, "vPosition");
    texture_vTex = glGetAttribLocation(texture_program, "vTexCoord");
    texture_proj_mat_loc = glGetUniformLocation(texture_program, "proj_matrix");
    texture_camera_mat_loc = glGetUniformLocation(texture_program, "camera_matrix");
    texture_model_mat_loc = glGetUniformLocation(texture_program, "model_matrix");

    // Load shaders and associate variables
//    ShaderInfo default_shaders[] = { {GL_VERTEX_SHADER, default_vertex_shader},{GL_FRAGMENT_SHADER, default_frag_shader},{GL_NONE, NULL} };
//    default_program = LoadShaders(default_shaders);
//    default_vPos = glGetAttribLocation(default_program, "vPosition");
//    default_vCol = glGetAttribLocation(default_program, "vColor");
//    default_proj_mat_loc = glGetUniformLocation(default_program, "proj_matrix");
//    default_cam_mat_loc = glGetUniformLocation(default_program, "camera_matrix");
//    default_model_mat_loc = glGetUniformLocation(default_program, "model_matrix");

    ShaderInfo debug_mirror_shaders[] = { {GL_VERTEX_SHADER, debug_mirror_vertex_shader},{GL_FRAGMENT_SHADER, debug_mirror_frag_shader},{GL_NONE, NULL} };
    debug_mirror_program = LoadShaders(debug_mirror_shaders);

    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Set background color
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    // Enable blending and set blend factors
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Set Initial camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
//    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    y = 2.5;
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye = vec3(x, y, z);
    centerX = x + radius*cos(azimuth*DEG2RAD);
//    centerY = y + radius*cos(elevation*DEG2RAD);
    centerY = y;
    centerZ = z + radius*sin(azimuth*DEG2RAD);
    center = vec3(centerX, centerY, centerZ);
    dir = center - eye;
    eye = eye + dir*del;

    // Start loop
    while ( !glfwWindowShouldClose( window ) ) {
        create_mirror();
    	// Draw graphics
        display();
        // Update other events like input handling
        glfwPollEvents();

        GLdouble curTime = glfwGetTime();
        if (fan) {
            fan_angle += (curTime - elTime) * (rpm / 60.0) * 360.0;
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

    // DEFAULT ORTHOGRAPHIC PROJECTION
//    proj_matrix = ortho(-15.0f*xratio, 15.0f*xratio, -15.0f*yratio, 15.0f*yratio, 15.0f, -15.0f);
//    proj_matrix = ortho(-5.0f*xratio, 5.0f*xratio, -5.0f*yratio, 5.0f*yratio, -5.0f, 5.0f);

    proj_matrix = frustum(-0.1f*xratio, 0.1f*xratio, -0.1f*yratio, 0.1f*yratio, 0.1f, 100.0f);
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

    // Set cube transformation matrix
    trans_matrix = translate(0.0f, 0.0f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(15.0f, 0.1f, 15.0f);
	model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw floor
//    draw_color_obj(Cube, RedCube);
    draw_tex_object(Cube, Carpet);

    trans_matrix = translate(0.0f, 5.0f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(15.0f, 0.1f, 15.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw ceiling
    draw_mat_object(Cube, WhitePaint);

    trans_matrix = translate(-7.5f, 2.5f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.1f, 5.0f, 15.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw back wall
//    draw_color_obj(Cube, BlueCube);
    draw_mat_object(Cube, GreyPaint);

    trans_matrix = translate(7.5f, 2.5f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.1f, 5.0f, 15.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw front wall
//    draw_color_obj(Cube, YellowCube);
    draw_mat_object(Cube, GreyPaint);

    trans_matrix = translate(0.0f, 2.5f, 7.5f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(15.0f, 5.0f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw left wall
//    draw_color_obj(Cube, MagentaCube);
    draw_mat_object(Cube, GreyPaint);

    trans_matrix = translate(0.0f, 2.5f, -7.5f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(15.0f, 5.0f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw right wall
//    draw_color_obj(Cube, GreenCube);
    draw_mat_object(Cube, GreyPaint);

    trans_matrix = translate(0.0f, 2.5f, -7.0f);
    rot_matrix = rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
    mat4 rot2 = rotate(180.0f, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(2.0f, 1.0f, 2.0f);
    model_matrix = trans_matrix*rot2*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw window
    draw_tex_object(Mirror, Outside);

    trans_matrix = translate(7.25f, 2.5f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.25f, 2.5f, 3.5f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw art
    draw_tex_object(Cube, Art);

    trans_matrix = translate(-7.25f, 1.75f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.01f, 3.5f, 1.75f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw door
    draw_tex_object(Cube, Door);

    trans_matrix = translate(-7.25f, 1.75f, -1.25f);
    rot_matrix = rotate(-90.0f, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(0.1f, 0.1f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw switch0
    draw_mat_object(Switch, Brass);

    trans_matrix = translate(-7.25f, 1.75f, -1.5f);
    rot_matrix = rotate(-90.0f, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(0.1f, 0.1f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw switch1
    draw_mat_object(Switch, Brass);

    trans_matrix = translate(-7.25f, 1.75f, 0.65f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(0.1f, 0.1f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw knob
    draw_mat_object(Sphere, Brass);

    trans_matrix = translate(0.0f, 0.0f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(3.0f, 3.0f, 3.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw desk
    draw_mat_object(Table, Wood);

    trans_matrix = translate(0.25f, 0.0f, -1.25f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(1.5f, 1.5f, 1.5f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw chair
    draw_mat_object(Chair, Wood);

    trans_matrix = translate(-0.25f, 0.0f, 1.25f);
    rot_matrix = rotate(180.0f, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(1.5f, 1.5f, 1.5f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw chair
    draw_mat_object(Chair, Wood);

    trans_matrix = translate(-1.25f, 0.0f, -0.25f);
    rot_matrix = rotate(90.0f, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(1.5f, 1.5f, 1.5f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw chair
    draw_mat_object(Chair, Wood);

    trans_matrix = translate(1.25f, 0.0f, 0.25f);
    rot_matrix = rotate(270.0f, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(1.5f, 1.5f, 1.5f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw chair
    draw_mat_object(Chair, Wood);

    // might replace with bowl and spheres
    trans_matrix = translate(0.0f, 1.25f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.1f, 0.1f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw fruit bowl
    draw_mat_object(Fruit, Orange);

    trans_matrix = translate(-0.75f, 1.375f, -0.25f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.1f, 0.1f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw can
    draw_mat_object(Can, Brass);

    trans_matrix = translate(0.0f, 4.5f, 0.0f);
    rot_matrix = rotate(fan_angle, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(0.5f, 0.5f, 0.5f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw fan
    draw_mat_object(Fan, WhitePaint);

    if (!mirror) {
        draw_frame(Frame);
        // Render mirror in scene
        // TODO: Set mirror transformation
        trans_matrix = translate(mirror_eye);
        rot_matrix = rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
        scale_matrix = scale(2.0f, 1.0f, 2.0f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        // TODO: Draw textured mirror
        draw_tex_object(Mirror, MirrorTex);
    }

    trans_matrix = translate(-0.75f, 1.075f, 0.25f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.05f, 0.05f, 0.05f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw cup
    glDepthMask(GL_FALSE);
    draw_mat_object(Glass, Acrylic);
    glDepthMask(GL_TRUE);
}

void create_mirror( ) {
    // Clear framebuffer for mirror rendering pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Set mirror projection matrix
    proj_matrix = frustum(-0.2f, 0.2f, -0.2f, 0.2f, 0.2f, 100.0f);

    // TODO: Set mirror camera matrix
    camera_matrix = lookat(mirror_eye, mirror_center, mirror_up);

    // Render mirror scene (without mirror)
    mirror = true;
    render_scene();
    glFlush();
    mirror = false;

    // TODO: Activate texture unit 0
    glActiveTexture(GL_TEXTURE0);
    // TODO: Bind mirror texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[MirrorTex]);
    // TODO: Copy framebuffer into mirror texture
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, ww, hh, 0);

}

void build_frame(GLuint obj) {
    vector<vec4> vertices;
    vector<vec3> normals;

    // Create wireframe for mirror
    vertices = {
            vec4(1.0f, 0.0f, -1.0f, 1.0f),
            vec4(1.0f, 0.0f, 1.0f, 1.0f),
            vec4(-1.0f, 0.0f, 1.0f, 1.0f),
            vec4(-1.0f, 0.0f, -1.0f, 1.0f)
    };

    normals = {
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f)
    };

    numVertices[obj] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void build_geometry( )
{
    // Generate vertex arrays and buffers
    glGenVertexArrays(NumVAOs, VAOs);

    // Load models
    load_object(Cube);
    load_object(Table);
    load_object(Chair);
    load_object(Fruit);
    load_object(Glass);
    load_object(Switch);
    load_object(Can);
    load_object(Sphere);
    load_object(Fan);
    load_object(Mirror);

    build_frame(Frame);

    // Generate color buffers
    glGenBuffers(NumColorBuffers, ColorBuffers);

    // Build color buffers
    // Define cube vertex colors (red)
    build_solid_color_buffer(numVertices[Cube], vec4(1.0f, 0.0f, 0.0f, 1.0f), RedCube);
    build_solid_color_buffer(numVertices[Cube], vec4(0.86f, 0.86f, 0.86f, 1.0f), Paint);
    build_solid_color_buffer(numVertices[Table], vec4(0.64f, 0.45f, 0.29f, 1.0f), SolidDesk);
    build_solid_color_buffer(numVertices[Chair], vec4(0.44f, 0.25f, 0.09f, 1.0f), SolidChair);
    build_solid_color_buffer(numVertices[Fruit], vec4(1.0f, 0.0f, 0.0f, 1.0f), SolidFruit);
    build_solid_color_buffer(numVertices[Glass], vec4(1.0f, 0.0f, 0.0f, 1.0f), SolidGlass);
    build_solid_color_buffer(numVertices[Switch], vec4(1.0f, 1.0f, 1.0f, 1.0f), SolidSwitch);
    build_solid_color_buffer(numVertices[Can], vec4(1.0f, 0.0f, 0.0f, 1.0f), SolidCan);
    build_solid_color_buffer(numVertices[Sphere], vec4(1.0f, 0.65f, 0.0f, 1.0f), SolidSphere);
}

void build_materials( ) {
    // Add materials to Materials vector
    MaterialProperties orange = {
            vec4(0.3f, 0.15f, 0.0f, 1.0f), //ambient
            vec4(0.6f, 0.3f, 0.0f, 1.0f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties whitePaint = {
            vec4(0.3f, 0.3f, 0.3f, 1.0f), //ambient
            vec4(0.6f, 0.6f, 0.6f, 1.0f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties greyPaint = {
            vec4(0.25f, 0.25f, 0.25f, 1.0f), //ambient
            vec4(0.55f, 0.55f, 0.55f, 1.0f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties Brass = {
            vec4(0.33f, 0.22f, 0.03f, 1.0f), //ambient
            vec4(0.78f, 0.57f, 0.11f, 1.0f), //diffuse
            vec4(0.99f, 0.91f, 0.81f, 1.0f), //specular
            27.8f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties Wood = {
            vec4(0.5f, 0.4f, 0.3f, 1.0f),
            vec4(0.6f, 0.5f, 0.4f, 1.0f),
            vec4(0.8f, 0.7f, 0.6f, 1.0f),
            32.0f,
            {0.0f, 0.0f, 0.0f}
    };

    MaterialProperties Acrylic = {
            vec4(0.3f, 0.3f, 0.3f, 0.8f), //ambient
            vec4(0.6f, 0.6f, 0.6f, 0.8f), //diffuse
            vec4(0.8f, 0.8f, 0.8f, 0.8f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties redPlastic = {
            vec4(0.3f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.6f, 0.0f, 0.0f, 1.0f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    Materials.push_back(orange);
    Materials.push_back(whitePaint);
    Materials.push_back(greyPaint);
    Materials.push_back(Brass);
    Materials.push_back(Wood);
    Materials.push_back(Acrylic);
    Materials.push_back(redPlastic);

    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
}

void build_lights( ) {
    // Add lights to Lights vector
    LightProperties whiteSpotLight = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(0.0f, 2.0f, 0.0f, 1.0f),  //position
            vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
            30.0f,   //cutoff
            30.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    LightProperties whitePointLight = {
            POINT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(0.0f, 2.4f, 0.0f, 1.0f),  //position
            vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
            0.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    Lights.push_back(whiteSpotLight);
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

void build_mirror( ) {
    // Generate mirror texture
    glGenTextures(1, &TextureIDs[MirrorTex]);
    // Bind mirror texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[MirrorTex]);
    // TODO: Create empty mirror texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ww, hh, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void draw_frame(GLuint obj) {
    // Draw frame using lines at mirror location
    glUseProgram(lighting_program);
    // Pass projection and camera matrices to shader
    glUniformMatrix4fv(lighting_proj_mat_loc, 1, GL_FALSE, proj_matrix);
    glUniformMatrix4fv(lighting_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Bind lights
    glUniformBlockBinding(lighting_program, lighting_lights_block_idx, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size() * sizeof(LightProperties));

    // Bind materials
    glUniformBlockBinding(lighting_program, lighting_materials_block_idx, 1);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, MaterialBuffers[MaterialBuffer], 0,
                      Materials.size() * sizeof(MaterialProperties));

    // Set camera position
    glUniform3fv(lighting_eye_loc, 1, eye);

    // Set num lights and lightOn
    glUniform1i(lighting_num_lights_loc, numLights);
    glUniform1iv(lighting_light_on_loc, numLights, lightOn);

    // Set frame transformation matrix
    mat4 trans_matrix = translate(mirror_eye);
    mat4 rot_matrix = rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
    mat4 scale_matrix = scale(2.0f, 1.0f, 2.0f);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    // Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    // Pass model matrix and normal matrix to shader
    glUniformMatrix4fv(lighting_model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(lighting_norm_mat_loc, 1, GL_FALSE, normal_matrix);
    glUniform1i(lighting_material_loc, RedPlastic);

    // Draw object using line loop
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(lighting_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vPos);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glVertexAttribPointer(lighting_vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vNorm);
    glDrawArrays(GL_LINE_LOOP, 0, numVertices[obj]);

}

    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // ESC to quit
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        for (int i = 0; i < numLights; i++) {
            lightOn[i] = (lightOn[i]+1)%2;
        }
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        fan = !fan;
    }

//    // Adjust azimuth
    if (key == GLFW_KEY_A) {
        azimuth -= daz;
        if (azimuth < 0.0) {
            azimuth += 360.0;
        }
    } else if (key == GLFW_KEY_D) {
        azimuth += daz;
        if (azimuth > 360.0)
        {
            azimuth -= 360.0;
        }
    }

    // Adjust elevation angle
    if (key == GLFW_KEY_W)
    {
//        elevation += del;
//        if (elevation > 179.0)
//        {
//            elevation = 179.0;
//        }
//        del += 0.005;
//        if (del > 7.5f) {
//            del = 7.5f;
//        }
        dir = center - eye;
        eye = eye + dir*del;
    }
    else if (key == GLFW_KEY_S)
    {
//        elevation -= del;
//        if (elevation < 1.0)
//        {
//            elevation = 1.0;
//        }
//        del -= 0.005;
//        if (del < -7.5f) {
//            del = -7.5f;
//        }
        dir = center - eye;
        eye = eye - dir*del;
    }

    //
    if (key == GLFW_KEY_X)
    {
        elevation += del;
        if (elevation > 90.0)
        {
            elevation = 89.0;
        }
    }
    else if (key == GLFW_KEY_Z)
    {
        elevation -= del;
        if (elevation < 0.0)
        {
            elevation = 1.0;
        }
    }
    // Compute updated camera position

//    GLfloat x, y, z;
//    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
////    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
//    y = 2.5;
//    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
//    eye = vec3(x,y,z);

    GLfloat centerX = eye[0] + radius*cos(azimuth*DEG2RAD);
//    GLfloat centerY = y + radius*cos(elevation*DEG2RAD);
    GLfloat centerY = eye[1];
    GLfloat centerZ = eye[2] + radius*sin(azimuth*DEG2RAD);
    center = vec3(centerX, centerY, centerZ);


    if (eye[0] > 7.5) {
        eye[0] = 7.4;
    }
    if (eye[0] < -7.5) {
        eye[0] = -7.4;
    }
    if (eye[2] > 7.5) {
        eye[2] = 7.4;
    }
    if (eye[2] < -7.5) {
        eye[2] = -7.4;
    }
}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

#include "utilfuncs.cpp"
