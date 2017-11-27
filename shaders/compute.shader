#version 430
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

uniform vec3 camera_pos;
uniform vec3 camera_dx;
uniform vec3 camera_dy;
uniform vec3 camera_ul;

uniform int num_spheres;

vec3 ambient_light = vec3(.1, .1, .1);
vec3 directional_light_color = vec3(.7, .7, .7);
vec3 directional_light_dir   = normalize(vec3(0, 0, 1));

vec4 background_color = vec4(1.0, 1.0, 1.0, 1.0);

/*
struct Sphere {
    vec3 pos;
    vec3 ka;
    vec3 kd;
    vec3 ks;
    vec3 kt;
    float power;
    float ior;
    float radius;
};
*/
struct Sphere {
    vec4 pos;
    vec4 ka;
    vec4 kd;
    vec4 ks;
    vec4 kt;
    vec4 e;
};

layout(std430, binding=2) buffer sphere_list
{
    Sphere spheres[];
};


struct Ray {
    vec3 pos;
    vec3 dir;
} ray;

bool IntersectSphere(in const Ray r, const in Sphere s, out float tmin, out float tmax) {
    // find if ray hit the sphere
    tmin = -1;
    tmax = -1;
    vec3 OC = r.pos - s.pos.xyz;
    float b = 2*dot(r.dir, OC);
    float c = dot(OC, OC) - s.e.z*s.e.z;
    float disc = b*b - 4*c;
    if (disc < 0)
        return false;

    float sdisc = sqrt(disc);
    tmin = .5 * (-b + sdisc);
    tmax = .5 * (-b - sdisc);
    if (tmin > tmax) {
        float tmp = tmin;
        tmin = tmax;
        tmax = tmp;
    }
    if (tmax < 0)
        return false;

    if (tmin < 0)
        tmin = tmax;

    return true;
}

bool Intersect(in const Ray r, out int hit_index, out float tmin, out float tmax) {
    int lowest_hit_index = -1;
    bool hit = false;
    for (int i = 0; i < num_spheres; i++) {
        float tmin0, tmax0;
        if (IntersectSphere(r, spheres[i], tmin0, tmax0)) {
            if (!hit) {
                lowest_hit_index = i;
                tmin = tmin0;
                tmax = tmax0;
                hit = true;
            } else {
                if (tmin > tmin0) {
                    tmin = tmin0;
                    tmax = tmax0;
                    lowest_hit_index = i;
                }
            }
        }
    }
    hit_index = lowest_hit_index;
    return hit;
}

void main() {
    vec4 pixel = background_color;
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    vec3 plane_pos = camera_ul + coords.x * camera_dx + coords.y * camera_dy;
    ray = Ray(camera_pos, normalize(plane_pos - camera_pos));

    float tmin, tmax;
    int hit_sphere_index = -1;
    if (Intersect(ray, hit_sphere_index, tmin, tmax)) {
        Sphere hit_sphere = spheres[hit_sphere_index];
        vec3 l = -directional_light_dir;
        vec3 hit_p = ray.pos + tmin*ray.dir;
        vec3 n = normalize(hit_p - hit_sphere.pos.xyz);
        vec3 v = ray.dir;
        Ray shadow = Ray(hit_p + 0.01*l, l);
        if (!IntersectSphere(shadow, hit_sphere, tmin, tmax)) {
            vec3 ret = vec3(0, 0, 0);
            ret += hit_sphere.ka.xyz * ambient_light;
            ret += hit_sphere.kd.xyz * directional_light_color * max(0.0, dot(n, l));
            float specular = pow(max(dot(v, reflect(l, n)), 0), hit_sphere.e.x);
            ret += hit_sphere.ks.xyz * directional_light_color * specular;

            pixel = vec4(ret, 1);
        } else {
            pixel = vec4(0, 0, 0, 1);
        }
    }

    imageStore(img_output, coords, pixel);
}
