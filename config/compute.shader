#version 430
layout(local_size_x = 8, local_size_y = 8) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

#define NUM_SPHERES @
#define NUM_TRIANGLES @
#define NUM_POINT_LIGHTS @
#define NUM_DIR_LIGHTS @
#define USING_ENV_MAP @
#define MAX_DEPTH @

#define M_PI 3.1415926535897932384626433832795
#define QUEUE_SIZE 500
#define BVH 0

uniform ivec2 img_size;
uniform vec3 camera_pos;
uniform vec3 camera_dx;
uniform vec3 camera_dy;
uniform vec3 camera_ul;
uniform vec4 background_color;
uniform sampler2D env_map;

uniform mat4 invProjView;

uniform vec3 ambient_light;

struct Queue {
    int arr[QUEUE_SIZE];
    int front;
    int back;
} Q;

void clearQueue() {
    Q.front = 0;
    Q.back = 0;
}

bool isQueueEmpty() {
    return Q.front == Q.back;
}

int deQueue() {
    int ret = Q.front;
    Q.front = Q.front + 1;
    if (Q.front == QUEUE_SIZE)
        Q.front = 0;
    return Q.arr[ret];
}

void enQueue(in const int bb) {
    Q.back = Q.back + 1;
    if (Q.back == QUEUE_SIZE)
        Q.back = 0;

    Q.arr[Q.back] = bb;
}

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
    float pad1;
    float pad2;
    float pad3;
    Material mat;
};

struct Triangle {
    vec4 v0v1;
    vec4 v0v2;
    int v0;
    int v1;
    int v2;
    int pad;
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

struct BB {
    vec4 min_;
    vec4 max_;
    int s1;
    int s2;
    int s3;
    int pad;
    bool isUsed;
    bool hasShapes;
    bool hasLeft;
    bool hasRight;
};

layout(std430, binding=2) buffer sphere_list
{
    Sphere spheres[];
};

layout(std430, binding=6) buffer vertex_list
{
    vec4 verts[];
};

layout(std430, binding=7) buffer triangle_list
{
    Triangle tris[];
};

layout(std430, binding=3) buffer dir_light_list
{
    DirectionalLight dir_lights[];
};

layout(std430, binding=4) buffer point_light_list
{
    PointLight point_lights[];
};

layout(std430, binding=5) buffer bb_list
{
    BB bounding_boxes[];
};

/*
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
*/

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

bool IntersectTriangle(in const Ray r, const in Triangle tri, out float t) {
    vec3 v0 = verts[tri.v0].xyz;
    vec3 v1 = verts[tri.v1].xyz;
    vec3 v2 = verts[tri.v2].xyz;
    vec3 pvec = cross(r.dir, tri.v0v2.xyz);
    float invDet = 1.0 / dot(tri.v0v1.xyz, pvec);
    vec3 tvec = r.pos - v0;
    float u = dot(tvec, pvec) * invDet;
    if (u < 0 || u > 1) return false;
    vec3 qvec = cross(tvec, tri.v0v1.xyz);
    float v = dot(r.dir, qvec) * invDet;
    if (v < 0 || u + v > 1) return false;
    t = dot(tri.v0v2.xyz, qvec) * invDet;
    if (t < 0) return false;
    return true;
}

bool IntersectBB(in const vec3 bb_min, in const vec3 bb_max, in const vec3 p, in const float inv_x,
        in const float inv_y, in const float inv_z) {

    float tmin = (bb_min.x - p.x) * inv_x;
    float tmax = (bb_max.x - p.x) * inv_x;
    if (tmin > tmax) {
        float tmp = tmax;
        tmax = tmin;
        tmin = tmp;
    }
    float tminy = (bb_min.y - p.y) * inv_y;
    float tmaxy = (bb_max.y - p.y) * inv_y;
    if (tminy > tmaxy) {
        float tmp = tmaxy;
        tmaxy = tminy;
        tminy = tmp;
    }
    if ((tmin > tmaxy) || (tminy > tmax))
        return false;
    tmin = max(tminy, tmin);
    tmax = min(tmaxy, tmax);
    float tminz = (bb_min.z - p.z) * inv_z;
    float tmaxz = (bb_max.z - p.z) * inv_z;
    if (tminz > tmaxz) {
        float tmp = tmaxz;
        tmaxz = tminz;
        tminz = tmp;
    }
    if ((tmin > tmaxz) || (tminz > tmax))
        return false;

    return true;
}

#if BVH == 1
bool Intersect(in const Ray r, out int hit_index, out float t) {
    float inv_x = 1.0 / r.dir.x;
    float inv_y = 1.0 / r.dir.y;
    float inv_z = 1.0 / r.dir.z;

    clearQueue();
    int curr = 0;
    BB currBB = bounding_boxes[curr];
    if (!IntersectBB(currBB.min_.xyz, currBB.max_.xyz, r.pos, inv_x, inv_y, inv_z))
        return false;

    enQueue(curr);

    hit_index = -1;
    t = 999999;

    while (!isQueueEmpty()) {
        curr = deQueue();
        currBB = bounding_boxes[curr];
        if (currBB.hasShapes) {
            int hs;
            float ht;
            if (IntersectSphere(r, spheres[currBB.s1], ht)) {
                if (ht < t) {
                    hit_index = currBB.s1;
                    t = ht;
                }
            }
        } else {
            if (currBB.hasLeft) {
                BB ll = bounding_boxes[curr * 2 + 1];
                if (IntersectBB(ll.min_.xyz, ll.max_.xyz, r.pos, inv_x, inv_y, inv_z))
                    enQueue(curr * 2 + 1);
            }
            if (currBB.hasRight) {
                BB rr = bounding_boxes[curr * 2 + 2];
                if (IntersectBB(rr.min_.xyz, rr.max_.xyz, r.pos, inv_x, inv_y, inv_z))
                    enQueue(curr * 2 + 2);
            }
        }
    }
    if (hit_index == -1)
        return false;
    return true;
}
#else
bool Intersect(in const Ray r, out int hit_index, out bool type, out float t) {
    int lowest_hit_index = -1;
    t = 999999;
    bool hit = false;
    type = false;
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
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        float tmp_t;
        if (IntersectTriangle(r, tris[i], tmp_t)) {
            if (tmp_t < t) {
                t = tmp_t;
                lowest_hit_index = i;
                hit = true;
                type = true;
            }
        }
    }
    hit_index = lowest_hit_index;
    return hit;
}
#endif

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

vec3 getLocalIllumination(in vec3 p, in vec3 v, in vec3 n, in Material mat) {
    vec3 pixel = vec3(0,0,0);
    int tmp;
    float t;
    bool type;

    // Add Ambient Light
    pixel += mat.ka.xyz * ambient_light;

    // Add directional Lights
    for (int i = 0; i < NUM_DIR_LIGHTS; ++i) {
        vec3 l = -dir_lights[i].dir.xyz;
        Ray shadow = Ray(p + 0.001*l, l);
        if (!Intersect(shadow, tmp, type, t)) {
            pixel += mat.kd.xyz * dir_lights[i].color.xyz * max(0.0, dot(n, l));
            float specular = pow(max(dot(v, reflect(l, n)), 0), mat.power);
            pixel += mat.ks.xyz * dir_lights[i].color.xyz * specular;
        }
    }

    // Add point lights
    for (int i = 0; i < NUM_POINT_LIGHTS; ++i) {
        vec3 l = point_lights[i].pos.xyz - p;
        float d = length(l);
        l = normalize(l);
        Ray shadow = Ray(p + 0.001*l, l);
        if (!Intersect(shadow, tmp, type, t) || length(t*shadow.dir) >= d) {
            vec3 I = (1.0 / (d*d)) * point_lights[i].color.xyz;
            pixel += I* mat.kd.xyz * max(0.0, dot(n, l));
            float specular = pow(max(dot(v, reflect(l, n)), 0), mat.power);
            pixel += I * mat.ks.xyz * specular;
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
        int hit_index = -1;
        bool isTriangle;
        if (Intersect(currentRay, hit_index, isTriangle, t)) {
            vec3 p, n, v;
            Material mat;
            p = currentRay.pos + t*currentRay.dir;
            v = currentRay.dir;
            if (isTriangle) {
                mat = tris[hit_index].mat;
                n = normalize(cross(tris[hit_index].v0v1.xyz, tris[hit_index].v0v2.xyz));
            } else {
                mat = spheres[hit_index].mat;
                n = normalize(p - spheres[hit_index].pos.xyz);
            }
            color += multiplier * getLocalIllumination(p, v, n, mat);
            multiplier *= mat.kt.xyz;
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
