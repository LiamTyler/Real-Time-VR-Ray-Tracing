#include "include/rayTracer.h"
#include "include/config.h"

RayTracer::RayTracer() {
    camera_vel_ = vec3(0, 0, 0);
    camera_rot_ = vec3(0, 0, 0);
    speed_ = 150;
    fpsTime_ = currentTime_ = lastTime_ = 0;
}

RayTracer::~RayTracer() {
    glDeleteBuffers(1, &quad_verts_vbo_);
    glDeleteBuffers(1, &quad_uv_vbo_);
    glDeleteVertexArrays(1, &quad_vao_);
    glDeleteProgram(compute_program_);
    glDeleteProgram(render_program_);
}

bool RayTracer::ParseEvent(const VREvent &event) {
    bool shutdown = false;
    if (event.getName() == "FrameStart") {
        currentTime_ = event.getDataAsFloat("ElapsedSeconds");
    } else {
        string name = event.getName();
        bool down = name[name.find('_') + 1] == 'D';
        if (down) {
            if (name == "KbdEsc_Down") {
                shutdown = true;
            } else if (name == "KbdW_Down") {
                camera_vel_.z = 1;
            } else if (name == "KbdS_Down") {
                camera_vel_.z = -1;
            } else if (name == "KbdA_Down") {
                camera_vel_.x = -1;
            } else if (name == "KbdD_Down") {
                camera_vel_.x = 1;
            }
        } else {
            if (name == "KbdW_Up" || name == "KbdS_Up") {
                camera_vel_.z = 0;
            } else if (name == "KbdA_Up" || name == "KbdD_Up") {
                camera_vel_.x = 0;
            }
        }
    }
    return shutdown;
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
    parser_ = Parser(Config::scene_file);
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

    compute_program_ = LoadComputeShader(Config::compute_shader);
    render_program_ = LoadShaders(Config::vert_shader, Config::frag_shader);
    glUseProgram(render_program_);

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
    vec3 dx = normalize(cross(camera_.dir, camera_.up));
    vec3 dy = -camera_.up;
    rotated_dx_ = dx;
    rotated_dy_ = dy;
    rotated_camera_dir_ = camera_.dir;
    rotated_camera_up_ = camera_.up;

    glUseProgram(compute_program_);
    ivec2 ss(SW_, SH_);
    glUniform2iv(glGetUniformLocation(compute_program_, "img_size"), 1, &ss[0]);
    glUniform3fv(glGetUniformLocation(compute_program_, "camera_pos"), 1, &camera_.pos[0]);

    // Send spheres to the GPU
    glGenBuffers(1, &spheres_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spheres_ssbo_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sphere) * parser_.spheres.size(), &parser_.spheres[0], GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, spheres_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUniform1i(glGetUniformLocation(compute_program_, "num_spheres"), parser_.spheres.size());

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
    glUniform1i(glGetUniformLocation(compute_program_, "num_dir_lights"), parser_.directional_lights.size());
    // point
    glGenBuffers(1, &point_lights_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, point_lights_ssbo_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLight) * parser_.point_lights.size(), &parser_.point_lights[0], GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, point_lights_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUniform1i(glGetUniformLocation(compute_program_, "num_point_lights"), parser_.point_lights.size());
    // send other variables to the GPU
    glUniform4fv(glGetUniformLocation(compute_program_, "background_color"),
            1, &parser_.background_color[0]);

}

void RayTracer::Render(const VRGraphicsState &renderState) {
    glUseProgram(compute_program_);
    GLint loc = glGetUniformLocation(compute_program_, "proj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, renderState.getProjectionMatrix());
    loc = glGetUniformLocation(compute_program_, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, renderState.getViewMatrix());

    glDispatchCompute((GLuint)SW_, (GLuint)SH_, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glClear(GL_COLOR_BUFFER_BIT);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glUseProgram(render_program_);
    glBindVertexArray(quad_vao_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    frameCounter_++;
    lastTime_ = currentTime_;
    if (currentTime_ > fpsTime_ + 1) {
        cout << "FPS: " << frameCounter_ << endl;
        fpsTime_ = currentTime_;
        frameCounter_ = 0;
    }
    glUseProgram(0);
}
