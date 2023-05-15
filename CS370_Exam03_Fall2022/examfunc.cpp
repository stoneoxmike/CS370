/******************************************/
/*!!!!!DO NOT MODIFY PAST THIS POINT!!!!!!*/
/******************************************/
void traverse_scene_graph(BaseNode *node, mat4 baseTransform) {
    mat4 model_matrix;

    // Stop when at bottom of branch
    if (node == NULL) {
        return;
    }

    // Apply local transformation and render
    model_matrix = baseTransform*node->ModelTransform;

    node->draw(proj_matrix, camera_matrix, model_matrix);

    // Recurse vertically if possible (depth-first)
    if (node->Child != NULL) {
        traverse_scene_graph(node->Child, model_matrix);
    }

    // Remove local transformation and recurse horizontal
    if (node->Sibling != NULL) {
        traverse_scene_graph(node->Sibling, baseTransform);
    }
}

void build_lights( ) {
    // White directional light
    LightProperties whiteDirLight = {
            DIRECTIONAL, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.5f, 0.5f, 0.5f, 1.0f), //diffuse
            vec4(0.5f, 0.5f, 0.5f, 1.0f), //specular
            vec4(0.0f, 0.0f, 0.0f, 1.0f),  //position
            vec4(-1.0f, -1.0f, -1.0f, 0.0f), //direction
            0.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };
    // Red present spot light
    LightProperties redPresentSpotLight = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.5f, 0.0f, 0.0f, 1.0f), //diffuse
            vec4(0.5f, 0.0f, 0.0f, 1.0f), //specular
            vec4(PRESENT_X, 3.0, PRESENT_Z, 1.0f),  //position
            vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
            20.0f,   //cutoff
            10.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    // Red spot light
    LightProperties redSpotLight1 = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 0.0f, 0.0f, 1.0f), //diffuse
            vec4(1.0f, 0.0f, 0.0f, 1.0f), //specular
            vec4(TREE_X+2.0f, BASE_HEIGHT+2*TREE_HEIGHT+STAR_HEIGHT, TREE_Z+2.0f, 1.0f),  //position
            vec4(-1.0f, 0.5f, -1.0f, 0.0f), //direction
            30.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };
    // White directional light
    LightProperties greenSpotLight1 = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.0f, 1.0f, 0.0f, 1.0f), //diffuse
            vec4(0.0f, 1.0f, 0.0f, 1.0f), //specular
            vec4(TREE_X+2.0f, BASE_HEIGHT+2*TREE_HEIGHT+STAR_HEIGHT, TREE_Z+2.0f, 1.0f),  //position
            vec4(-1.0f, 0.5f, -1.0f, 0.0f), //direction
            30.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    // Red spot light
    LightProperties redSpotLight2 = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 0.0f, 0.0f, 1.0f), //diffuse
            vec4(1.0f, 0.0f, 0.0f, 1.0f), //specular
            vec4(TREE_X-2.0f, BASE_HEIGHT+2*TREE_HEIGHT+STAR_HEIGHT, TREE_Z+2.0f, 1.0f),  //position
            vec4(1.0f, 0.5f, -1.0f, 0.0f), //direction
            30.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };
    // White directional light
    LightProperties greenSpotLight2 = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.0f, 1.0f, 0.0f, 1.0f), //diffuse
            vec4(0.0f, 1.0f, 0.0f, 1.0f), //specular
            vec4(TREE_X-2.0f, BASE_HEIGHT+2*TREE_HEIGHT+STAR_HEIGHT, TREE_Z+2.0f, 1.0f),  //position
            vec4(1.0f, 0.5f, -1.0f, 0.0f), //direction
            30.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    Lights.push_back(whiteDirLight);
    Lights.push_back(redPresentSpotLight);
    Lights.push_back(redSpotLight1);
    Lights.push_back(greenSpotLight1);
    Lights.push_back(redSpotLight2);
    Lights.push_back(greenSpotLight2);

    numLights = Lights.size();
    //for (int i = 0; i < numLights; i++) {
    // 	lightOn[i] = 1;
    //}
    lightOn[0] = 1;
    lightOn[1] = 1;

    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(), GL_STATIC_DRAW);
}

void build_materials( ) {
    // Create green plastic material
    MaterialProperties greenPlastic = {vec4(0.0f, 0.3f, 0.0f, 1.0f), //ambient
                                      vec4(0.0f, 0.3f, 0.0f, 1.0f), //diffuse
                                      vec4(0.6f, 0.8f, 0.6f, 1.0f), //specular
                                      32.0f, //shininess
                                      {0.0f, 0.0f, 0.0f}  //pad
    };

    // Create white plastic material
    MaterialProperties whitePlastic = {vec4(0.2f, 0.2f, 0.2f, 1.0f), //ambient
                                       vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
                                       vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
                                       12.0f, //shininess
                                       {0.0f, 0.0f, 0.0f}  //pad
    };

    // Create brass material
    MaterialProperties coal = {vec4(0.1f, 0.1f, 0.1f, 1.0f), //ambient
                               vec4(0.0f, 0.0f, 0.0f, 1.0f), //diffuse
                               vec4(0.1f, 0.1f, 0.1f, 1.0f), //specular
                               20.0f, //shininess
                               {0.0f, 0.0f, 0.0f}  //pad
    };

    // Create red plastic material
    MaterialProperties redAcrylic = {vec4(0.3f, 0.0f, 0.0f, 0.5f), //ambient
                                     vec4(0.6f, 0.0f, 0.0f, 0.5f), //diffuse
                                     vec4(0.8f, 0.6f, 0.6f, 0.5f), //specular
                                     32.0f, //shininess
                                     {0.0f, 0.0f, 0.0f}  //pad
    };

    // Add materials to Materials vector
    Materials.push_back(greenPlastic);
    Materials.push_back(whitePlastic);
    Materials.push_back(coal);
    Materials.push_back(redAcrylic);

    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
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
        int width_in_bytes = w * 4;
        unsigned char *top = NULL;
        unsigned char *bottom = NULL;
        unsigned char temp = 0;
        int half_height = h / 2;

        for ( int row = 0; row < half_height; row++ ) {
            top = image_data + row * width_in_bytes;
            bottom = image_data + ( h - row - 1 ) * width_in_bytes;
            for ( int col = 0; col < width_in_bytes; col++ ) {
                temp = *top;
                *top = *bottom;
                *bottom = temp;
                top++;
                bottom++;
            }
        }

        // Bind current texture id
        glBindTexture(GL_TEXTURE_2D, TextureIDs[i]);
        // Load image data into texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     image_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        // Set scaling modes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // Set wrapping modes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Set maximum anisotropic filtering for system
        GLfloat max_aniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
        // set the maximum!
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
    }
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

void build_lighting_node(MatNode& node, GLuint obj, GLuint material, GLboolean transparent, mat4 base_trans){
    // Set shader program and matrix references
    node.set_shader(lighting_program, lighting_proj_mat_loc, lighting_camera_mat_loc, lighting_norm_mat_loc, lighting_model_mat_loc);
    // Set object buffers
    node.set_buffers(VAOs[obj], ObjBuffers[obj][PosBuffer], lighting_vPos, posCoords, ObjBuffers[obj][NormBuffer], lighting_vNorm, normCoords, numVertices[obj]);
    // Set material buffers and material
    node.set_materials(MaterialBuffers[MaterialBuffer], lighting_materials_block_idx, Materials.size()*sizeof(MaterialProperties), lighting_material_loc, material, transparent);
    // Set light buffers
    node.set_lights(LightBuffers[LightBuffer], lighting_lights_block_idx, Lights.size()*sizeof(LightProperties), lighting_num_lights_loc, Lights.size(), lighting_light_on_loc, lightOn);
    // Set eye position
    node.set_eye(lighting_eye_loc, eye);
    // Set base transform
    node.set_base_transform(base_trans);
    // Set default update transform and nodes
    node.set_update_transform(mat4().identity());
    node.attach_nodes(NULL, NULL);
}

void build_texture_node(TexNode& node, GLuint obj, GLuint texture, mat4 base_trans){
    // Set shader program and matrix references
    node.set_shader(texture_program, texture_proj_mat_loc, texture_camera_mat_loc, texture_model_mat_loc);
    // Set object buffers
    node.set_buffers(VAOs[obj], ObjBuffers[obj][PosBuffer], texture_vPos, posCoords, ObjBuffers[obj][TexBuffer], texture_vTex, texCoords, numVertices[obj]);
    // Set texture
    node.set_texture(TextureIDs[texture]);
    // Set base transform
    node.set_base_transform(base_trans);
    // Default update transform and nodes
    node.set_update_transform(mat4().identity());
    node.attach_nodes(NULL, NULL);
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

void draw_mat_object(GLuint obj, GLuint material){
    // Select shader program
    glUseProgram(lighting_program);

    // Pass projection and camera matrices to shader
    glUniformMatrix4fv(lighting_proj_mat_loc, 1, GL_FALSE, proj_matrix);
    glUniformMatrix4fv(lighting_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Bind lights
    glUniformBlockBinding(lighting_program, lighting_lights_block_idx, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size()*sizeof(LightProperties));

    // Bind materials
    glUniformBlockBinding(lighting_program, lighting_materials_block_idx, 1);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, MaterialBuffers[MaterialBuffer], 0, Materials.size()*sizeof(MaterialProperties));

    // Set camera position
    glUniform3fv(lighting_eye_loc, 1, eye);

    // Set num lights and lightOn
    glUniform1i(lighting_num_lights_loc, numLights);
    glUniform1iv(lighting_light_on_loc, numLights, lightOn);

    // Pass model matrix and normal matrix to shader
    glUniformMatrix4fv(lighting_model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(lighting_norm_mat_loc, 1, GL_FALSE, normal_matrix);

    // Pass material index to shader
    glUniform1i(lighting_material_loc, material);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(lighting_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vPos);

    // Bind normal object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glVertexAttribPointer(lighting_vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vNorm);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_table(){
    // Draw cube
    mat4 trans_matrix = translate(0.0f, -0.1f, 0.0f);
    mat4 scale_matrix = scale(TABLE_SIZE, 0.2f, TABLE_SIZE);
    model_matrix = trans_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Cube, WhitePlastic);
}

void draw_carpet(){
    model_matrix = translate(0.0f, 0.05f, 0.0f)*scale(4.0f, 1.0f, 4.0f);
    draw_tex_object(Carpet, Santa);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Esc closes window
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        mode++;
        if (mode > NUM_MODES)
        {
            mode = 0;
            animate = false;
            // Turn star lights off
            for (int i = 2; i < numLights; i++) {
                lightOn[i] = 0;
            }
        }
        if (mode > 4) {
            animate = true;
        }
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        if (mode > 4) {
            animate = !animate;
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
    glViewport(0, 0, width, height);

    // Store new window sizes in global variables
    ww = width;
    hh = height;
}

