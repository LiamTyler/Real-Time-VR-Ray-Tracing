#ifndef SRC_INCLUDE_MATERIAL_H_
#define SRC_INCLUDE_MATERIAL_H_

#include "include/utils.h"

typedef struct Material {
    Material() {
        ka = vec3(0, 0, 0);
        kd = vec3(0, 0, 0);
        ks = vec3(0, 0, 0);
        kt = vec3(0, 0, 0);
        power = 0;
        ior = 0;
    }

    Material(vec3 a, vec3 d, vec3 s, vec3 t, float p, float i) {
        ka = a;
        kd = d;
        ks = s;
        kt = t;
        power = p;
        ior = i;
    }

    vec3 ka;
    vec3 kd;
    vec3 ks;
    vec3 kt;
    float power;
    float ior;

} Material;

#endif  // SRC_INCLUDE_MATERIAL_H_
