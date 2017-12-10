#version 430
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

#define NUM_SPHERES @
#define NUM_POINT_LIGHTS @
#define NUM_DIR_LIGHTS @
#define USING_ENV_MAP @
#define MAX_DEPTH @

#define M_PI 3.1415926535897932384626433832795

uniform ivec2 img_size;
uniform vec3 camera_pos;
uniform vec3 camera_dx;
uniform vec3 camera_dy;
uniform vec3 camera_ul;
uniform vec4 background_color;
uniform sampler2D env_map;

uniform mat4 invProjView;

uniform vec3 ambient_light;

struct Ray {
    vec3 pos;
    vec3 dir;
};

struct Material {
    vec4 ka;
    vec4 kd;
    vec4 ks;
    vec4 kt;
    float power;
    float ior;
    float pad1;
    float pad2;
};

struct Sphere {
    vec4 pos;
    float radius;
    Material mat;
};

struct DirectionalLight {
    vec4 color;
    vec4 dir;
};

struct PointLight {
    vec4 color;
    vec4 pos;
};

layout(std430, binding=2) buffer sphere_list
{
    Sphere spheres[];
};


layout(std430, binding=3) buffer dir_light_list
{
    DirectionalLight dir_lights[];
};

layout(std430, binding=4) buffer point_light_list
{
    PointLight point_lights[];
};

bool IntersectSphere(in const Ray r, const in Sphere s, out float t) {
    // find if ray hit the sphere
    float tmin = -1;
    float tmax = -1;
    vec3 OC = r.pos - s.pos.xyz;
    float b = 2*dot(r.dir, OC);
    // float c = dot(OC, OC) - s.e.z*s.e.z;
    float c = dot(OC, OC) - s.radius*s.radius;
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

    t = tmin;

    return true;
}

/*
   bool IntersectSphere(in const Ray r, const in Sphere s, out float t) {
   vec3 OC = r.pos - s.pos.xyz;
   float b = dot(r.dir, OC);
   float c = dot(OC, OC) - s.radius*s.radius;
   float disc = b*b - c;
   if (disc < 0)
   return false;

   float sdisc = sqrt(disc);
   t = -b - sdisc;
   if (t < 0)
   return false;

   return true;
   }
 */

bool Intersect(in const Ray r, out int hit_index, out float t) {
    int lowest_hit_index = -1;
    t = 999999;
    bool hit = false;
    for (int i = 0; i < NUM_SPHERES; i++) {
        float tmp_t;
        if (IntersectSphere(r, spheres[i], tmp_t)) {
            if (tmp_t < t) {
                t = tmp_t;
                lowest_hit_index = i;
                hit = true;
            }
        }
    }
    hit_index = lowest_hit_index;
    return hit;
}

vec3 getBackgroundColor(in vec3 d) {
    vec3 pixel;
    if (USING_ENV_MAP) {
        float lat = asin(d.y);
        float lon = atan(d.x, d.z);
        float u = lon / (2*M_PI) + 0.5;
        float v = lat / M_PI + 0.5;
        pixel = texture(env_map, vec2(u, 1 - v)).rgb;
    } else {
        pixel = background_color.xyz;
    }
    return pixel;
}

vec3 getLocalIllumination(in vec3 p, in vec3 v, in vec3 n, in Sphere hit_sphere) {
    vec3 pixel = vec3(0,0,0);
    int tmp;
    float t;

    // Add Ambient Light
    pixel += hit_sphere.mat.ka.xyz * ambient_light;

    // Add directional Lights
    for (int i = 0; i < NUM_DIR_LIGHTS; ++i) {
        vec3 l = -dir_lights[i].dir.xyz;
        Ray shadow = Ray(p + 0.001*l, l);
        if (!Intersect(shadow, tmp, t)) {
            pixel += hit_sphere.mat.kd.xyz * dir_lights[i].color.xyz * max(0.0, dot(n, l));
            float specular = pow(max(dot(v, reflect(l, n)), 0), hit_sphere.mat.power);
            pixel += hit_sphere.mat.ks.xyz * dir_lights[i].color.xyz * specular;
        }
    }

    // Add point lights
    for (int i = 0; i < NUM_POINT_LIGHTS; ++i) {
        vec3 l = point_lights[i].pos.xyz - p;
        float d = length(l);
        l = normalize(l);
        Ray shadow = Ray(p + 0.001*l, l);
        if (!Intersect(shadow, tmp, t) || length(t*shadow.dir) >= d) {
            vec3 I = (1.0 / (d*d)) * point_lights[i].color.xyz;
            pixel += I* hit_sphere.mat.kd.xyz * max(0.0, dot(n, l));
            float specular = pow(max(dot(v, reflect(l, n)), 0), hit_sphere.mat.power);
            pixel += I * hit_sphere.mat.ks.xyz * specular;
        }
    }

    return pixel;
}

vec3 getColor(in Ray r) {
    vec3 color = vec3(0,0,0);
    vec3 multiplier = vec3(1,1,1);
    Ray currentRay = r;
    for (int i = 0; i < MAX_DEPTH; i++) {
        float t;
        int hit_sphere_index = -1;
        if (Intersect(currentRay, hit_sphere_index, t)) {
            Sphere hit_sphere = spheres[hit_sphere_index];
            vec3 p = currentRay.pos + t*currentRay.dir;
            vec3 n = normalize(p - hit_sphere.pos.xyz);
            vec3 v = currentRay.dir;
            color += multiplier * getLocalIllumination(p, v, n, hit_sphere);
            multiplier *= hit_sphere.mat.kt.xyz;
            vec3 dir = reflect(currentRay.dir, n);
            currentRay = Ray(p + 0.001 * dir, dir);
        } else {
            color += multiplier * getBackgroundColor(currentRay.dir);
            break;
        }
    }

    return color;
}

void main() {
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    float x = -1 + ((coords.x + 0.5) / img_size.x) * 2;
    float y =  1 - ((coords.y + 0.5) / img_size.y) * 2;

    vec3 o = camera_pos; 
    vec4 dir = invProjView * vec4(x, y, 1, 1);
    Ray ray = Ray(o, normalize((dir / dir.w).xyz - o));
    vec3 pixel = getColor(ray);

    imageStore(img_output, coords, vec4(pixel, 1));
}
