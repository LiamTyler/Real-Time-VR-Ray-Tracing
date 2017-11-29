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

    vec3 ka; float pad1;
    vec3 kd; float pad2;
    vec3 ks; float pad3;
    vec3 kt; float pad4;
    float power;
    float ior; vec2 pad5;

} Material;



inline ostream& operator<<(ostream& out, const Material& m) {
    out << "ka: " << m.ka << endl;
    out << "kd: " << m.kd << endl;
    out << "ks: " << m.ks << endl;
    out << "kt: " << m.kt << endl;
    out << "power: " << m.power << endl;
    out << "ior: " << m.ior << endl;
    return out;
}

#endif  // SRC_INCLUDE_MATERIAL_H_
