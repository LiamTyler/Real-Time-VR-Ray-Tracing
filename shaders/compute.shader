#version 430
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

uniform vec3 camera_pos;
uniform vec3 camera_dx;
uniform vec3 camera_dy;
uniform vec3 camera_ul;

vec3 sphere_pos = vec3(0, 0, 300);
float sphere_r = 100;
vec3 ka = vec3(1, 0, 0);
vec3 kd = vec3(1, 0, 0);
vec3 ks = vec3(1, 1, 1);
const float pe = 50;

vec3 ambient_light = vec3(.1, .1, .1);
vec3 directional_light_color = vec3(.7, .7, .7);
vec3 directional_light_dir   = normalize(vec3(0, 0, 1));

vec4 background_color = vec4(1.0, 1.0, 1.0, 1.0);

struct Ray {
    vec3 pos;
    vec3 dir;
} ray;

bool IntersectSphere(in const Ray r, const in vec3 pos, const in float radius, out float tmin, out float tmax) {
    // find if ray hit the sphere
    tmin = -1;
    tmax = -1;
    vec3 OC = r.pos - pos;
    float b = 2*dot(r.dir, OC);
    float c = dot(OC, OC) - radius*radius;
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

void main() {
    vec4 pixel = background_color;
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    vec3 plane_pos = camera_ul + coords.x * camera_dx + coords.y * camera_dy;
    ray = Ray(camera_pos, normalize(plane_pos - camera_pos));

    float tmin, tmax;
    if (IntersectSphere(ray, sphere_pos, sphere_r, tmin, tmax)) {
        vec3 l = -directional_light_dir;
        vec3 hit_p = ray.pos + tmin*ray.dir;
        vec3 n = normalize(hit_p - sphere_pos);
        vec3 v = ray.dir;
        Ray shadow = Ray(hit_p + 0.01*l, l);
        if (!IntersectSphere(shadow, sphere_pos, sphere_r, tmin, tmax)) {
            vec3 ret = vec3(0, 0, 0);
            ret += ka * ambient_light;
            ret += kd * directional_light_color * max(0.0, dot(n, l));
            float specular = pow(max(dot(v, reflect(l, n)), 0), pe);
            ret += ks * directional_light_color * specular;

            pixel = vec4(ret, 1);
        } else {
            pixel = vec4(0, 0, 0, 1);
        }
    }

    imageStore(img_output, coords, pixel);
}
