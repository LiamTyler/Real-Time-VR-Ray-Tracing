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
    vec3 getCenter() { return pos; }
    void GetBB(vec3& min_, vec3& max_) {
        min_ = vec3(pos.x - radius, pos.y - radius, pos.z - radius);
        max_ = vec3(pos.x + radius, pos.y + radius, pos.z + radius);
    }

    vec3 pos; float pad1;
    float radius; vec3 pad2;
    Material mat;

} Sphere;

typedef struct Triangle {
    Triangle() {
        v0 = 0;
        v1 = 0;
        v2 = 0;
    }
    Triangle(int v0_, int v1_, int v2_, Material m) {
        v0 = v0_;
        v1 = v1_;
        v2 = v2_;
        mat = m;
    }
    void CalculateEdges(vector<vec4>& verts) {
        v0v1 = verts[v1] - verts[v0];
        v0v2 = verts[v2] - verts[v0];
    }

    vec4 v0v1;
    vec4 v0v2;
    int v0;
    int v1;
    int v2;
    int pad3;
    Material mat;
} Triangle;

inline ostream& operator<<(ostream& out, const Sphere& s) {
    out << "pos: " << s.pos << endl;
    out << "radius: " << s.radius << endl;
    out << "Material: " << endl;
    out << s.mat << endl;
    return out;
}

#endif  // SRC_INCLUDE_SHAPES_H_
