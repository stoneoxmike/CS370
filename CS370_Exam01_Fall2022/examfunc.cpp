/******************************************/
/*!!!!!DO NOT MODIFY PAST THIS POINT!!!!!!*/
/******************************************/
void build_target() {
    vector<vec4> vertices;
    vector<ivec3> indices;
    vector<vec4> colors;

    // Bind target vertex array object
    glBindVertexArray(VAOs[Target]);

    // Define vertices for table boundary
    vertices = {
            {-TABLE_SIZE/6, 0.02f, 0.0f, 1.0f},
            {TABLE_SIZE/6, 0.02f, 0.0f, 1.0f},
            {0.0f, 0.02f, TABLE_SIZE/6, 1.0f},
            {0.0f, 0.02f,-TABLE_SIZE/6, 1.0f}
    };

    // Define vertices for table circle
    for (int i = 4; i < 68; i++) {
        vertices.push_back(vec4(CIRCLE_RAD*sin((i-4)*0.1f), 0.02f, CIRCLE_RAD*cos((i-4)*0.1f), 1.0f));
    }

    // Define target colors (black)
    for (int i = 0; i < 68; i++) {
        colors.push_back(vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }

    // Set numVertices
    numVertices[Target] = 68;

    // Generate object buffer for table
    glGenBuffers(NumObjBuffers, ObjBuffers[Target]);

    // Bind pyramid positions
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Target][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[Target], vertices.data(), GL_STATIC_DRAW);

    // Bind pyramid colors
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[TargetColorBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*numVertices[Cube], colors.data(), GL_STATIC_DRAW);
}

void draw_table(){
    mat4 scale_matrix = scale(TABLE_SIZE, 0.02f, TABLE_SIZE);
    mat4 trans_matrix = translate(0.0f, -0.2f, 0.0f);
    model_matrix = scale_matrix;
    draw_color_obj(Cube, TableColorBuffer);

    if (mode < 3) {
        model_matrix = mat4().identity();
    } else {
        mat4 targ_trans = translate(TARGET_X, TARGET_Y, TARGET_Z);
        model_matrix = targ_trans;
    }
    draw_target();
}

// Object loader
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

// Create solid color buffer
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer) {
    vector<vec4> obj_colors;
    for (int i = 0; i < num_vertices; i++) {
        obj_colors.push_back(color);
    }

    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[buffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*num_vertices, obj_colors.data(), GL_STATIC_DRAW);
}

// Draw object with color
void draw_color_obj(GLuint obj, GLuint color) {
    // Select default shader program
    glUseProgram(default_program);

    // Pass projection matrix to default shader
    glUniformMatrix4fv(default_proj_mat_loc, 1, GL_FALSE, proj_matrix);

    // Pass camera matrix to default shader
    glUniformMatrix4fv(default_cam_mat_loc, 1, GL_FALSE, camera_matrix);

    // Pass model matrix to default shader
    glUniformMatrix4fv(default_model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes for default shader
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(default_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(default_vPos);

    // Bind color buffer and set attributes for default shader
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[color]);
    glVertexAttribPointer(default_vCol, colCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(default_vCol);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_target(){
    // Select default shader program
    glUseProgram(default_program);

    // Pass projection matrix to default shader
    glUniformMatrix4fv(default_proj_mat_loc, 1, GL_FALSE, proj_matrix);

    // Pass camera matrix to default shader
    glUniformMatrix4fv(default_cam_mat_loc, 1, GL_FALSE, camera_matrix);

    // Pass model matrix to default shader
    glUniformMatrix4fv(default_model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind vertex array
    glBindVertexArray(VAOs[Target]);

    // Bind position object buffer and set attributes for default shader
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Target][PosBuffer]);
    glVertexAttribPointer(default_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(default_vPos);

    // Bind color buffer and set attributes for default shader
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[TargetColorBuffer]);
    glVertexAttribPointer(default_vCol, colCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(default_vCol);

    // Draw object
    glDrawArrays(GL_LINES, 0, 4);
    if (mode > 2) {
        glDrawArrays(GL_LINE_LOOP, 4, 64);
    }

}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Esc closes window
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        mode++;
        if (mode == 3) {
            spin_flag = true;
        } else if (mode == 6) {
            rev_flag = true;
        } else if (mode > NUM_MODES)
        {
            mode = 0;
            spin_theta = 0.0f;
            rev_theta = 0.0f;
            spin_flag = true;
            rev_flag = true;
        }

    }

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

    // Compute updated camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye = vec3(x,y,z);
}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // TODO: Store new window sizes in global variables
    ww = width;
    hh = height;
}

