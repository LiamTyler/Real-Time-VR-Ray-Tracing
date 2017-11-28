#include "include/utils.h"
#include "include/parser.h"

// Screen width / height (scene file overwrites these values)
int SW = 800;
int SH = 800;

Camera camera;
float dist_to_plane;
vec3 camera_vel = vec3(0, 0, 0);
vec3 camera_rot = vec3(0, 0, 0);
vec3 rotated_camera_dir;
vec3 rotated_camera_up;
vec3 rotated_dx;
vec3 rotated_dy;
float speed = 150;

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

int main() {
    // Parse the scene file
    Parser parser("scenes/single_sphere_dir.scn");
    if (!parser.Parse()) {
        cout << "Could not parse scene" << endl;
        return 1;
    }
    SW = parser.film_resolution.x;
    SH = parser.film_resolution.y;
    SDL_Window* window = InitAndWindow("Real Time RayTracer", 100, 100, SW, SH);
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

    GLuint compute_program = LoadComputeShader("shaders/compute.shader");
    GLuint render_program = LoadShaders("shaders/shader.vert", "shaders/shader.frag");
    glUseProgram(render_program);

    GLuint tex_output;
    glGenTextures(1, &tex_output);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SW, SH, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint verts_vbo;
    glGenBuffers(1, &verts_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, verts_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    GLint posAttrib = glGetAttribLocation(render_program, "pos");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    GLuint uv_vbo;
    glGenBuffers(1, &uv_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uv), uv, GL_STATIC_DRAW);
    GLint texAttrib = glGetAttribLocation(render_program, "inTexCoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glUseProgram(compute_program);
    camera = parser.camera;
    // distance from the camera to the 'plane' we are drawing on
    dist_to_plane = SH / (2 * tan(camera.height_fov / 2));
    // Middle of the plane
    vec3 mid = camera.pos + dist_to_plane * camera.dir;
    // Distance of one pixel on the X axis on the plane
    vec3 dx = normalize(cross(camera.up, camera.dir));
    // Distance of one pixel on the Y axis on the plane
    vec3 dy = -camera.up;
    // Position of the upper left pixel on the plane
    vec3 ul = mid - .5*(SW - 1)*dx - .5*(SH - 1)*dy;
    rotated_dx = dx;
    rotated_dy = dy;
    rotated_camera_dir = camera.dir;

    // Send the camera info to the compute shader
    glUseProgram(compute_program);
    glUniform3fv(glGetUniformLocation(compute_program, "camera_pos"), 1, &camera.pos[0]);
    glUniform3fv(glGetUniformLocation(compute_program, "camera_dx"), 1, &dx[0]);
    glUniform3fv(glGetUniformLocation(compute_program, "camera_dy"), 1, &dy[0]);
    glUniform3fv(glGetUniformLocation(compute_program, "camera_ul"), 1, &ul[0]);

    // Send spheres to the GPU
    glUseProgram(compute_program);
    GLuint ssbo = 0;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sphere) * parser.spheres.size(), &parser.spheres[0], GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUniform1i(glGetUniformLocation(compute_program, "num_spheres"), parser.spheres.size());

    // send lights to the GPU
    // ambient
    glUniform3fv(glGetUniformLocation(compute_program, "ambient_light"),
                 1, (GLfloat*) &parser.ambient_light);
    // directional
    GLuint dir_lights_ssbo = 0;
    glGenBuffers(1, &dir_lights_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dir_lights_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DirectionalLight) * parser.directional_lights.size(), &parser.directional_lights[0], GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, dir_lights_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUniform1i(glGetUniformLocation(compute_program, "num_dir_lights"), parser.directional_lights.size());
    // point
    GLuint point_lights_ssbo = 0;
    glGenBuffers(1, &point_lights_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, point_lights_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLight) * parser.point_lights.size(), &parser.point_lights[0], GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, point_lights_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUniform1i(glGetUniformLocation(compute_program, "num_point_lights"), parser.point_lights.size());
    // send other variables to the GPU
    glUniform4fv(glGetUniformLocation(compute_program, "background_color"),
                 1, &parser.background_color[0]);

    SDL_SetRelativeMouseMode(SDL_TRUE);
    unsigned int lastTime = SDL_GetTicks();
    unsigned int fpsTime = lastTime;
    unsigned int currentTime;
    SDL_Event e;
    bool quit = false;
    unsigned int frameCounter = 0;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;
            if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                switch(e.key.keysym.sym) {
                    case SDLK_a:
                        camera_vel.x = -1;
                        break;
                    case SDLK_d:
                        camera_vel.x = 1;
                        break;
                    case SDLK_s:
                        camera_vel.z = -1;
                        break;
                    case SDLK_w:
                        camera_vel.z = 1;
                        break;
                }
            } else if (e.type == SDL_KEYUP) {
                if (e.key.keysym.sym == SDLK_a || e.key.keysym.sym == SDLK_d)
                    camera_vel.x = 0;
                else if (e.key.keysym.sym == SDLK_w || e.key.keysym.sym == SDLK_s)
                    camera_vel.z = 0;
            } else if (e.type == SDL_MOUSEMOTION) {
                float x = e.motion.xrel;
                float y = e.motion.yrel;
                x = radians(x) / 4;
                y = radians(y) / 4;
                camera_rot.x = fmin(radians(80.0f), fmax(radians(-80.0f), camera_rot.x + y));
                camera_rot.y += x;
                mat4 r(1.0f);
                r = rotate(r, camera_rot.y, vec3(0, 1, 0));
                r = rotate(r, camera_rot.x, vec3(1, 0, 0));
                rotated_camera_dir = vec3(r*vec4(camera.dir, 0));
                rotated_camera_up = vec3(r*vec4(camera.up, 0));
                rotated_dx = cross(rotated_camera_up, rotated_camera_dir);
                rotated_dy = -rotated_camera_up;
                ul = camera.pos + dist_to_plane * rotated_camera_dir
                     - .5*(SW - 1)*rotated_dx - .5*(SH - 1)*rotated_dy;
                glUseProgram(compute_program);
                glUniform3fv(glGetUniformLocation(compute_program, "camera_dx"), 1, &rotated_dx[0]);
                glUniform3fv(glGetUniformLocation(compute_program, "camera_dy"), 1, &rotated_dy[0]);
            }
        }
        // update shit
        currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        vec3 inc = speed * dt * (camera_vel.z*rotated_camera_dir + camera_vel.x*rotated_dx);
        camera.pos += inc;
        ul += inc;
        glUseProgram(compute_program);
        glUniform3fv(glGetUniformLocation(compute_program, "camera_pos"), 1, &camera.pos[0]);
        glUniform3fv(glGetUniformLocation(compute_program, "camera_ul"), 1, &ul[0]);
        glDispatchCompute((GLuint)SW, (GLuint)SH, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(render_program);
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_output);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        SDL_GL_SwapWindow(window);
        frameCounter++;
        lastTime = currentTime;
        if (currentTime > fpsTime + 1000) {
            cout << "FPS: " << frameCounter << endl;
            fpsTime = currentTime;
            frameCounter = 0;
        }
    }

    glDeleteProgram(render_program);
    SDL_Quit();

    return 0;
}
