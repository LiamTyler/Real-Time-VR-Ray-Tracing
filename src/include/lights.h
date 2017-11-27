#ifndef SRC_INCLUDE_LIGHTS_H_
#define SRC_INCLUDE_LIGHTS_H_

#include "include/utils.h"

typedef struct AmbientLight {
    AmbientLight() {
        color = vec3(0, 0, 0);
    }

    AmbientLight(vec3 c) {
        color = c;
    }

    vec3 color;

} AmbientLight;

typedef struct PointLight {
    PointLight() {
        color = vec3(0, 0, 0);
        pos = vec3(0, 0, 0);
    }

    PointLight(vec3 c, vec3 p) {
        color = c;
        pos = p;
    }

    vec3 color; float pad1;
    vec3 pos; float pad2;

} PointLight;

typedef struct DirectionalLight {
    DirectionalLight() {
        dir = vec3(0, 0, 0);
        color = vec3(0, 0, 0);
    }

    DirectionalLight(vec3 c, vec3 d) {
        color = c;
        dir = d;
    }

    vec3 color; float pad1;
    vec3 dir; float pad2;

} DirectionalLight;

#endif  // SRC_INCLUDE_LIGHTS_H_
