#ifndef SRC_INCLUDE_RAY_TRACER_H_
#define SRC_INCLUDE_RAY_TRACER_H_

#include "include/utils.h"
#include "include/parser.h"
#include "include/camera.h"

class RayTracer {
    public:
        RayTracer();
        ~RayTracer();
        bool ParseEvent(const VREvent & event);
        void SetUp();
        void Render(const VRGraphicsState &renderState);

    private:
        GLuint quad_vao_;
        GLuint quad_verts_vbo_;
        GLuint quad_uv_vbo_;
        GLuint tex_output_;
        GLuint render_program_;
        GLuint compute_program_;

        GLuint spheres_ssbo_;
        GLuint dir_lights_ssbo_;
        GLuint point_lights_ssbo_;

        int SW_;
        int SH_;
        Parser parser_;
        Camera camera_;
        float dist_to_plane_;
        vec3 camera_vel_;
        vec3 camera_rot_;
        vec3 rotated_camera_dir_;
        vec3 rotated_camera_up_;
        vec3 rotated_dx_;
        vec3 rotated_dy_;
        float speed_;
        float lastTime_;
        float currentTime_;
        float fpsTime_;
        unsigned int frameCounter_;
};

#endif  // SRC_INCLUDE_RAY_TRACER_H_
