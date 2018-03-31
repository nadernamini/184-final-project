#include "bvh.h"

#include "CGL/CGL.h"
#include "static_scene/triangle.h"

#include <iostream>
#include <stack>

using namespace std;

namespace CGL {
    namespace StaticScene {

        BVHAccel::BVHAccel(const std::vector<Primitive *> &_primitives,
                           size_t max_leaf_size) {

            root = construct_bvh(_primitives, max_leaf_size);

        }

        BVHAccel::~BVHAccel() {
            if (root) delete root;
        }

        BBox BVHAccel::get_bbox() const {
            return root->bb;
        }

        void BVHAccel::draw(BVHNode *node, const Color &c) const {
            if (node->isLeaf()) {
                for (Primitive *p : *(node->prims))
                    p->draw(c);
            } else {
                draw(node->l, c);
                draw(node->r, c);
            }
        }

        void BVHAccel::drawOutline(BVHNode *node, const Color &c) const {
            if (node->isLeaf()) {
                for (Primitive *p : *(node->prims))
                    p->drawOutline(c);
            } else {
                drawOutline(node->l, c);
                drawOutline(node->r, c);
            }
        }

        BVHNode *BVHAccel::construct_bvh(const std::vector<Primitive *> &prims, size_t max_leaf_size) {
            // Part 2 Task 1:
            // Construct a BVH from the given vector of primitives and maximum leaf
            // size configuration. The starter code build a BVH aggregate with a
            // single leaf node (which is also the root) that encloses all the
            // primitives.
            BBox centroid_box, bbox;


            for (Primitive *p : prims) {
                BBox bb = p->get_bbox();
                bbox.expand(bb);
                Vector3D c = bb.centroid();
                centroid_box.expand(c);
            }
            bbox.set_bounds();
            centroid_box.set_bounds();

            auto *node = new BVHNode(bbox);
            if (prims.size() <= max_leaf_size || prims.size() <= 1) {
                node->prims = new vector<Primitive *>(prims);
                return node;
            }


            Vector3D extent = centroid_box.extent;
            vector<Primitive *> left, right;
            double pt;
            int xyz = 0, idx;
            bool same;
            double maximum = std::max(extent.x, std::max(extent.y, extent.z));
            if (extent.x == maximum) {
                xyz = 0;
            } else if (extent.y == maximum) {
                xyz = 1;
            } else {
                xyz = 2;
            }

            pt = centroid_box.centroid()[xyz];
            while (left.empty() || right.empty()) {
                left.clear();
                right.clear();
                idx = 0;
                same = this->check(prims, prims.size(), xyz);
                if (same) {
                    for (Primitive *p : prims) {
                        if (idx < prims.size() / 2) {
                            left.push_back(p);
                        } else {
                            right.push_back(p);
                        }
                        idx++;
                    }
                } else {
                    for (Primitive *p : prims) {
                        if (p->get_bbox().centroid()[xyz] < pt) {
                            left.push_back(p);
                        } else {
                            right.push_back(p);
                        }

                    }
                }
                if (left.empty()) {
                    pt += std::abs(centroid_box.max[xyz] - pt) / 2.0;
                } else if (right.empty()) {
                    pt -= std::abs(pt - centroid_box.min[xyz]) / 2.0;
                }
            }
            node->l = construct_bvh(left, max_leaf_size);
            node->r = construct_bvh(right, max_leaf_size);
            return node;
        }


        bool BVHAccel::intersect(const Ray &ray, BVHNode *node) const {
            // Part 2, Task 3
            // Fill in the intersect function.
            // Take note that this function has a short-circuit that the
            // Intersection version cannot, since it returns as soon as it finds
            // a hit, it doesn't actually have to find the closest hit.
            double t0, t1;
            bool in = get_bbox().intersect(ray, t0, t1);
            if (!in) {
                return false;
            }
            if (node->isLeaf()) {
                for (Primitive *p : *(node->prims)) {
                    total_isects++;
                    if (p->intersect(ray))
                        return true;
                }
                return false;
            }
            return intersect(ray, node->l) || intersect(ray, node->r);
        }

        bool BVHAccel::intersect(const Ray &ray, Intersection *i, BVHNode *node) const {
            // Part 2, Task 3
            // The intersect function.
            double t0, t1;
            bool in = node->bb.intersect(ray, t0, t1);
            if (!in) {
                return false;
            }
            if (node->isLeaf()) {
                bool hit = false;
                for (Primitive *p : *(node->prims)) {
                    total_isects++;
                    if (p->intersect(ray, i)) {
                        hit = true;
                    }
                }
                return hit;
            }

            bool hitl, hitr, hit1, hit2;

            double tl0, tl1, tr0, tr1;
            hitl = node->l->bb.intersect(ray, tl0, tl1);
            hitr = node->r->bb.intersect(ray, tr0, tr1);
            if (hitl && hitr) {
                if (tl0 < tr0) {
                    hit1 = intersect(ray, i, node->l);
                    hit2 = intersect(ray, i, node->r);
                    return hit1 || hit2;
                } else {
                    hit1 = intersect(ray, i, node->r);
                    hit2 = intersect(ray, i, node->l);
                    return hit1 || hit2;
                }
            } else if (hitl) {
                return intersect(ray, i, node->l);
            } else {
                return intersect(ray, i, node->r);
            }
        }

    }  // namespace StaticScene
}  // namespace CGL
