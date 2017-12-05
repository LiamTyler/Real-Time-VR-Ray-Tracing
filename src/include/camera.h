#ifndef SRC_INCLUDE_CAMERA_H_
#define SRC_INCLUDE_CAMERA_H_

#include "include/utils.h"

typedef struct Camera {
    Camera() {
        pos = vec3(0, 0, 0);
        dir = vec3(0, 0, 1);
        up = vec3(0, 1, 0);
        height_fov = radians(90.0f);
    }

    Camera(vec3 p, vec3 d, vec3 u, float f) {
        pos = p;
        dir = d;
        up = u;
        height_fov = f;
    }

    vec3 pos;
    vec3 dir;
    vec3 up;
    float height_fov;

} Camera;

#endif  // SRC_INCLUDE_CAMERA_H_
