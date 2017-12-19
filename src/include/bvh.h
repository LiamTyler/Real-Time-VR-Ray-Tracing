#ifndef SRC_INCLUDE_BVH_
#define SRC_INCLUDE_BVH_

#include "include/utils.h"
#include "include/shapes.h"

typedef struct glsl_bvh {
    vec4 min_;
    vec4 max_;
    int s1;
    int s2;
    int s3;
    int pad;
    int isUsed;    // bool p1, p2, p3;
    int hasShapes; // bool p11, p12, p13;
    int hasLeft;   // bool p21, p22, p23;
    int hasRight;  // bool p31, p32, p33;

} glsl_bvh;

class BVH {
    public:
        BVH();
        void Partition(const std::vector<Sphere*>& shapes);
        // bool RayHitsBB(Ray& ray);
        BVH* getLeft() { return left_; }
        BVH* getRight() { return right_; }
        int getNumShapes() { return shapes_.size(); }
        void getBB(vec3& mi, vec3& ma) { mi = min_; ma = max_; }
        int getDeepestLevel();
        void PrintTree(int curr_depth, int max_depth);
        void Fill(glsl_bvh flatten[], vector<Sphere*> spheres, int currIndex);

    private:
        BVH* left_;
        BVH* right_;
        vector<Sphere*> shapes_;
        glm::vec3 min_;
        glm::vec3 max_;
};

#endif  // SRC_INCLUDE_BVH_H_
