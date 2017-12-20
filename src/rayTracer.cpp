#include "include/rayTracer.h"
#include "include/config.h"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#include "include/bvh.h"


RayTracer::RayTracer(string scene) {
    camera_vel_ = vec3(0, 0, 0);
    camera_rot_ = vec3(0, 0, 0);
    camera_rot_vel_ = vec3(0, 0, 0);
    speed_ = 20;
    fpsTime_ = currentTime_ = lastTime_ = 0;
    model_ = mat4(1.0f);
    scene_ = Config::scene_folder + scene;
    cout << scene_ << endl;
}

RayTracer::~RayTracer() {
    glDeleteBuffers(1, &quad_verts_vbo_);
    glDeleteBuffers(1, &quad_uv_vbo_);
    glDeleteVertexArrays(1, &quad_vao_);
    glDeleteProgram(compute_program_);
    glDeleteProgram(render_program_);
}

void RayTracer::EditShader(string in_file, string out_file) {
    ifstream in(in_file);
    if (in.fail()) {
        cout << "cannot open file: " << in_file << endl;
        return;
    }
    ofstream out(out_file);
    if (!out.is_open()) {
        cout << "cannot open file: " << out_file << endl;
        return;
    }
    string line;
    while (getline(in, line)) {
        int at = line.find('@');
        if (at != string::npos) {
            int space1 = line.find(' ');
            int space2 = line.find(' ', space1 + 1);
            string var = line.substr(space1 + 1, space2 - space1 - 1);
            string result = "";
            if (var == "NUM_SPHERES") {
                result = to_string(parser_.spheres.size());
            } else if (var == "NUM_DIR_LIGHTS") {
                result = to_string(parser_.directional_lights.size());
            } else if (var == "NUM_POINT_LIGHTS") {
                result = to_string(parser_.point_lights.size());
            } else if (var == "NUM_TRIANGLES") {
                result = to_string(parser_.triangles.size());
            } else if (var == "MAX_DEPTH") {
                result = to_string(parser_.max_depth);
            } else if (var == "USING_ENV_MAP") {
                if (parser_.env_map == "")
                    result = "false";
                else
                    result = "true";
            }
            line = line.substr(0, at) + result;
        }
        out << line << "\n";
    }
    in.close();
    out.close();
}

bool RayTracer::ParseEvent(Event& ename) {
    switch(ename) {
        case QUIT:
            return true;
            break;
        case L_FORWARDS_DOWN:
            camera_vel_.z = 1;
            break;
        case L_BACKWARDS_DOWN:
            camera_vel_.z = -1;
            break;
        case L_LEFT_DOWN:
            camera_vel_.x = -1;
            break;
        case L_RIGHT_DOWN:
            camera_vel_.x = 1;
            break;
        case L_UP_DOWN:
            camera_vel_.y = 1;
            break;
        case L_DOWN_DOWN:
            camera_vel_.y = -1;
            break;
        case L_FORWARDS_UP:
        case L_BACKWARDS_UP:
            camera_vel_.z = 0;
            break;
        case L_LEFT_UP:
        case L_RIGHT_UP:
            camera_vel_.x = 0;
            break;
        case L_UP_UP:
        case L_DOWN_UP:
            camera_vel_.y = 0;
            break;
        case R_FORWARDS_DOWN:
            camera_rot_vel_.x = radians(1.0f);
            break;
        case R_BACKWARDS_DOWN:
            camera_rot_vel_.x = radians(-1.0f);
            break;
        case R_LEFT_DOWN:
            camera_rot_vel_.y = radians(1.0f);
            break;
        case R_RIGHT_DOWN:
            camera_rot_vel_.y = radians(-1.0f);
            break;
        case R_FORWARDS_UP:
        case R_BACKWARDS_UP:
            camera_rot_vel_.x = 0;
            break;
        case R_LEFT_UP:
        case R_RIGHT_UP:
            camera_rot_vel_.y = 0;
            break;
    }
        
    return false;
}

void RayTracer::LoadEnvMap(string path) {
    int w, h, comp;
    unsigned char * image;

    image = stbi_load(path.c_str(), &w, &h, &comp, 4);
    if (image == nullptr) {
        cout << "Failed to load the image: " << path << endl;
    }
    glGenTextures(1, &texture_);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);

    cout << "loaded image: " << path  << ", with: w=" << w << ", h=" << h << endl;
}

void RayTracer::SetUp() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cout << "Error initializing GLEW." << std::endl;
    }

    glClearColor(1, 1, 1, 1);

    const float verts[] = {
        -1, 1, 0,
        -1, -1, 0,
        1, -1, 0,
        1, -1, 0,
        1, 1, 0,
        -1, 1, 0,
    };

    const float uv[] = {
        0, 1,
        0, 0,
        1, 0,
        1, 0,
        1, 1,
        0, 1,
    };

    // Parse the scene file
    parser_ = Parser(scene_);
    if (!parser_.Parse()) {
        cout << "Could not parse scene" << endl;
        return;
    }
    SW_ = parser_.film_resolution.x;
    SH_ = parser_.film_resolution.y;

    cout << "vendor: " << glGetString(GL_VENDOR) << endl;
    cout << "renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "version: " << glGetString(GL_VERSION) << endl;
    ivec3 work_group_cnt;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_group_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_group_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_group_cnt[2]);

    cout << "max global work group size: " << work_group_cnt.x << ", " << work_group_cnt.y
        << ", " << work_group_cnt.z << endl;

    ivec3 work_group_size;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_group_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_group_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_group_size[2]);

    cout << "max global work group size: " << work_group_size.x << ", " << work_group_size.y
        << ", " << work_group_size.z << endl;

    int work_group_invocations;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_group_invocations);
    cout << "max local work group invocations: " << work_group_invocations << endl;

    // Edit and Compile shaders
    EditShader(Config::compute_shader_in, Config::compute_shader_out);
    compute_program_ = LoadComputeShader(Config::compute_shader_out);
    render_program_ = LoadShaders(Config::vert_shader, Config::frag_shader);
    glUseProgram(compute_program_);

    tex_output_;
    glGenTextures(1, &tex_output_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SW_, SH_, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, tex_output_, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glUseProgram(render_program_);
    quad_vao_;
    glGenVertexArrays(1, &quad_vao_);
    glBindVertexArray(quad_vao_);

    quad_verts_vbo_;
    glGenBuffers(1, &quad_verts_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, quad_verts_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    GLint posAttrib = glGetAttribLocation(render_program_, "pos");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    quad_uv_vbo_;
    glGenBuffers(1, &quad_uv_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, quad_uv_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uv), uv, GL_STATIC_DRAW);
    GLint texAttrib = glGetAttribLocation(render_program_, "inTexCoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    camera_ = parser_.camera;
    camera_pos_ = camera_.pos;
    // camera_rot_.x = atan2(camera_.dir.y, camera_.dir.z);
    camera_rot_.y = atan2(-camera_.dir.x, -camera_.dir.z);
    cout << "camera_rot: " << camera_rot_ << endl;
    vec3 dx = normalize(cross(camera_.dir, camera_.up));
    vec3 dy = -camera_.up;
    rotated_dx_ = dx;
    rotated_dy_ = dy;
    rotated_camera_dir_ = camera_.dir;
    rotated_camera_up_ = camera_.up;

    glUseProgram(compute_program_);
    ivec2 ss(SW_, SH_);
    glUniform2iv(glGetUniformLocation(compute_program_, "img_size"), 1, &ss[0]);
    glUniform3fv(glGetUniformLocation(compute_program_, "camera_pos"), 1, &camera_pos_[0]);

    // Send spheres to the GPU
    Sphere* arr = new Sphere[parser_.spheres.size()];
    for (int i = 0; i < parser_.spheres.size(); i++) {
        arr[i] = *parser_.spheres[i];
    }
    glGenBuffers(1, &spheres_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spheres_ssbo_);
    // glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sphere) * parser_.spheres.size(), &parser_.spheres[0], GL_STATIC_COPY);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sphere) * parser_.spheres.size(), &arr[0], GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, spheres_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    delete[] arr;

    // Vertices
    glGenBuffers(1, &vertex_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_ssbo_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vec4) * parser_.vertices.size(), &parser_.vertices[0], GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, vertex_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &triangle_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangle_ssbo_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Triangle) * parser_.triangles.size(), &parser_.triangles[0], GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, triangle_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // send lights to the GPU
    // ambient
    glUniform3fv(glGetUniformLocation(compute_program_, "ambient_light"),
            1, (GLfloat*) &parser_.ambient_light);
    // directional
    glGenBuffers(1, &dir_lights_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dir_lights_ssbo_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DirectionalLight) * parser_.directional_lights.size(), &parser_.directional_lights[0], GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, dir_lights_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    // point
    glGenBuffers(1, &point_lights_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, point_lights_ssbo_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLight) * parser_.point_lights.size(), &parser_.point_lights[0], GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, point_lights_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    // send other variables to the GPU
    glUniform4fv(glGetUniformLocation(compute_program_, "background_color"),
            1, &parser_.background_color[0]);
    if (parser_.env_map != "") {
		LoadEnvMap(Config::main_dir + parser_.env_map);
        glUniform1i(glGetUniformLocation(compute_program_, "env_map"), 1);
    }

    /*
    BVH* root = new BVH;
    root->Partition(parser_.spheres);
    root->PrintTree(0, 5);
    int level = root->getDeepestLevel();
    int total = 0;
    for (int i = 0; i < level; i++)
        total += pow(2, i);
    cout << "total nodes: " << total << endl;
    glsl_bvh flatten[total];
    for (int i = 0; i < total; i++) {
        flatten[i].isUsed = false;
    }
    root->Fill(flatten, parser_.spheres, 0);
    cout << "sizeof glsl_bvh: " << sizeof(glsl_bvh) << endl;
    glGenBuffers(1, &bvh_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvh_ssbo_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glsl_bvh) * total, &flatten, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, bvh_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    */
}

void RayTracer::Render(mat4 &view, mat4 &proj) {
    // update model matrix
    float dt = currentTime_ - lastTime_;
    dt *= speed_;
    camera_rot_ += 2 * dt * camera_rot_vel_;
    /*
    model_ = mat4(1.0f);
    model_ = rotate(model_, dt * camera_rot_.y, vec3(0, 1, 0));
    model_ = rotate(model_, dt * camera_rot_.x, vec3(1, 0, 0));
    model_ = translate(model_, dt * camera_vel_);
    model_ = translate(model_, camera_.pos);
    */
    mat4 rot = rotate(mat4(1.0f), camera_rot_.y, vec3(0, 1, 0));
    rot = rotate(rot, camera_rot_.x, vec3(1, 0, 0));

    // vec3 new_dir = vec3(rot * vec4(camera_.dir, 0));
    // vec3 new_up = vec3(rot * vec4(camera_.up, 0));
    vec3 new_dir = vec3(rot * vec4(0, 0, -1, 0));
    vec3 new_up = vec3(rot * vec4(0, 1, 0, 0));
    vec3 new_dx = cross(new_dir, new_up);
    camera_pos_ += .5 * dt * (camera_vel_.z * new_dir + camera_vel_.x * new_dx);
    mat4 trans = translate(mat4(1.0f), camera_pos_);

    model_ =  trans * rot ;
    // model_ = rotate(mat4(1.0), radians(10.0f), vec3(0,1,0));
    // model_ = translate(mat4(1.0), vec3(0, 0, 50));
    // view = inverse(model_) * view;   // (m-1 * v)-1 => (v-1)*ray
	// cout << view << endl;
    view = view * inverse(model_);
    // keep track of FPS
    frameCounter_++;
    lastTime_ = currentTime_;
    if (currentTime_ > fpsTime_ + 1) {
        cout << "FPS: " << frameCounter_ << endl;
        fpsTime_ = currentTime_;
        frameCounter_ = 0;
    }
    mat4 iView = inverse(view);
    vec3 pos = vec3(iView * vec4(0,0,0,1));
    mat4 iProj = inverse(proj);
    mat4 invProjView = iView * iProj;
    glUseProgram(compute_program_);
    GLint loc = glGetUniformLocation(compute_program_, "invProjView");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &invProjView[0][0]);
    glUniform3fv(glGetUniformLocation(compute_program_, "camera_pos"), 1, &pos[0]);

    if (parser_.env_map != "") {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture_);
    }
    glDispatchCompute((GLuint)SW_ / 8, (GLuint)SH_ / 8, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(render_program_);
    glBindVertexArray(quad_vao_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUseProgram(0);
}
