#include "include/bvh.h"
#include <functional>
#include <iostream>

using namespace std;

BVH::BVH() {
    left_ = nullptr;
    right_ = nullptr;
    min_ = vec3(0,0,0);
    max_ = vec3(0,0,0);
}

/*
   bool BVH::RayHitsBB(Ray& ray) {
   vec3 p = ray.p;
   vec3 d = ray.dir;
   float tmin = (min_.x - p.x) / d.x;
   float tmax = (max_.x - p.x) / d.x;
   if (tmin > tmax)
   swap(tmin, tmax);
   float tymin = (min_.y - p.y) / d.y;
   float tymax = (max_.y - p.y) / d.y;
   if (tymin > tymax)
   swap(tymin, tymax);
   if ((tmin > tymax) || (tymin > tmax))
   return false;

   tmin = max(tymin, tmin);
   tmax = min(tymax, tmax);

   float tzmin = (min_.z - p.z) / d.z;
   float tzmax = (max_.z - p.z) / d.z;
   if (tzmin > tzmax)
   swap(tzmin, tzmax);
   if ((tmin > tzmax) || (tzmin > tmax))
   return false;

// tmin = max(tzmin, tmin);
// tmax = min(tzmax, tmax);
// ray.tmin = tmin;
// ray.tmax = tmax;
return true;
}
*/

void BVH::Fill(glsl_bvh flatten[], vector<Sphere*> spheres, int currIndex) {
    flatten[currIndex].min_.x = min_.x;
    flatten[currIndex].min_.y = min_.y;
    flatten[currIndex].min_.z = min_.z;
    flatten[currIndex].max_.x = max_.x;
    flatten[currIndex].max_.y = max_.y;
    flatten[currIndex].max_.z = max_.z;
    flatten[currIndex].isUsed = true;
    if (shapes_.size()) {
        flatten[currIndex].hasShapes = true;
        for (int i = 0; i < spheres.size(); i++) {
            if (spheres[i] == shapes_[0])
                flatten[currIndex].s1 = i;
            if (spheres[i] == shapes_[1])
                flatten[currIndex].s2 = i;
        }
    } else {
        flatten[currIndex].hasShapes = false;
    }
    if (left_) {
        flatten[currIndex].hasLeft = true;
        left_->Fill(flatten, spheres, currIndex * 2 + 1);
    } else {
        flatten[currIndex].hasLeft = false;
    }
    if (right_) {
        flatten[currIndex].hasRight = true;
        right_->Fill(flatten, spheres, currIndex * 2 + 2);
    } else {
        flatten[currIndex].hasRight = false;
    }
}

int BVH::getDeepestLevel() {
    int l = -1;
    int r = -1;
    if (left_)
        l = left_->getDeepestLevel();
    if (right_)
        r = right_->getDeepestLevel();

    int maxC = std::max(l, r);
    if (l == -1 && r == -1)
        maxC = 0;
    return 1 + maxC;
}

void BVH::Partition(const vector<Sphere*>& shapes) {
    if (shapes.size() == 0) {
        cout << "ERROR, TRYING TO PARTITION 0 SHAPES" << endl;
        return;
    }

    // find the bounding box for all shapes combined
    vec3 curr_min, curr_max;
    float minX, maxX, minY, maxY, minZ, maxZ;
    shapes[0]->GetBB(curr_min, curr_max);
    minX = curr_min.x; maxX = curr_max.x;
    minY = curr_min.y; maxY = curr_max.y;
    minZ = curr_min.z; maxZ = curr_max.z;
    vec3 minC = shapes[0]->getCenter();
    vec3 maxC = shapes[0]->getCenter();

    int i = 0;
    for (vector<Sphere*>::const_iterator it = shapes.begin(); it != shapes.end(); ++it) {
        // get bounding box for shape
        (*it)->GetBB(curr_min, curr_max);
        minX = fmin(minX, curr_min.x);
        minY = fmin(minY, curr_min.y);
        minZ = fmin(minZ, curr_min.z);
        maxX = fmax(maxX, curr_max.x);
        maxY = fmax(maxY, curr_max.y);
        maxZ = fmax(maxZ, curr_max.z);

        // find the min / max components of all of the shape centers
        vec3 c = (*it)->getCenter();
        minC.x = fmin(minC.x, c.x);
        minC.y = fmin(minC.y, c.y);
        minC.z = fmin(minC.z, c.z);
        maxC.x = fmax(maxC.x, c.x);
        maxC.y = fmax(maxC.y, c.y);
        maxC.z = fmax(maxC.z, c.z);

    }
    min_ = vec3(minX, minY, minZ);
    max_ = vec3(maxX, maxY, maxZ);

    vec3 mid = (minC + maxC) / 2;
    vec3 d = maxC - minC;
    function<bool(const vec3&, const vec3&)> cmp;
    // split on largest axis
    if (d.x >= d.y && d.x >= d.z) {
        cmp = [&](vec3 mid, vec3 center) { return mid.x >= center.x; };
    } else if (d.y >= d.x && d.y >= d.z) {
        cmp = [&](vec3 mid, vec3 center) { return mid.y >= center.y; };
    } else {
        cmp = [&](vec3 mid, vec3 center) { return mid.z >= center.z; };
    }

    vector<Sphere*> left;
    vector<Sphere*> right;
    i = 0;
    for (vector<Sphere*>::const_iterator it = shapes.begin(); it != shapes.end(); ++it) {
        if (cmp(mid, (*it)->getCenter()))
            left.push_back(*it);
        else
            right.push_back(*it);
    }
    if (left.size() == 0 || right.size() == 0) {
        shapes_ = shapes;
        if (shapes_.size() > 1)
            cout << shapes_.size() << endl;
    } else {
        left_ = new BVH;
        right_ = new BVH;
        left_->Partition(left);
        right_->Partition(right);
    }
}

void BVH::PrintTree(int curr_depth, int max_depth) {
    if (curr_depth > max_depth)
        return;
    string t(curr_depth, '\t');
    cout << t << "num shapes: " << shapes_.size() << endl;
    cout << t << "min: " << min_ << endl;
    cout << t << "max: " << max_ << endl;
    curr_depth++;
    if (curr_depth <= max_depth) {
        if (left_) {
            cout << t << "left:" << endl;
            left_->PrintTree(curr_depth, max_depth);
        } if (right_) {
            cout << t << "right:" << endl;
            right_->PrintTree(curr_depth, max_depth);
        }
    }
}

