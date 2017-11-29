#ifndef SRC_INCLUDE_SHAPES_H_
#define SRC_INCLUDE_SHAPES_H_

#include "include/utils.h"
#include "include/material.h"

typedef struct Sphere {
    Sphere() {
        pos = vec3(0, 0, 0);
        radius = 0;
        mat = Material();
    }

    Sphere(vec3 p, float r, Material m) {
        pos = p;
        radius = r;
        mat = m;
    }

    vec3 pos; float pad1;
    float radius; vec3 pad2;
    Material mat;

} Sphere;

inline ostream& operator<<(ostream& out, const Sphere& s) {
    out << "pos: " << s.pos << endl;
    out << "radius: " << s.radius << endl;
    out << "Material: " << endl;
    out << s.mat << endl;
    return out;
}

#endif  // SRC_INCLUDE_SHAPES_H_
