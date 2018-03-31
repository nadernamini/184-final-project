#include "sphere.h"

#include <cmath>

#include  "../bsdf.h"
#include "../misc/sphere_drawing.h"

namespace CGL {
    namespace StaticScene {

        bool Sphere::test(const Ray &r, double &t1, double &t2) const {

            // Part 1 Task 4:
            // Implement ray - sphere intersection test.
            // Return true if there are intersections and writing the
            // smaller of the two intersection times in t1 and the larger in t2.
            double a, b, c, del, tmp;
            a = dot(r.d, r.d);
            b = 2 * dot(r.o - this->o, r.d);
            c = dot(r.o - this->o, r.o - this->o) - pow(this->r, 2);
            del = pow(b, 2) - (4 * a * c);
            if (del <= 0)
                return false;
            else {
                t1 = (-b - sqrt(del)) / (2 * a);
                t2 = (-b + sqrt(del)) / (2 * a);
                if (t1 > t2) {
                    tmp = t2;
                    t2 = t1;
                    t1 = tmp;
                }
//                return r.min_t <= t2 && t1 <= t2 && t2 <= r.max_t;
                return std::max(t1, r.min_t) <= std::min(t2, r.max_t) && t2 <= r.max_t;
            }

        }

        bool Sphere::intersect(const Ray &r) const {

            // Part 1 Task 4:
            // Implement ray - sphere intersection.
            // Note that you might want to use the the Sphere::test helper here.
            double t1, t2;
            bool in = test(r, t1, t2);

            if (in) {
                r.max_t = t1 >= r.min_t ? t1 : t2;
                return true;
            } else {
                return false;
            }
        }

        bool Sphere::intersect(const Ray &r, Intersection *i) const {
            // Part 1 Task 4:
            // Implement ray - sphere intersection.
            // Note again that you might want to use the the Sphere::test helper here.
            // When an intersection takes place, the Intersection data should be updated
            // correspondingly.
            double t1, t2;
            bool in = test(r, t1, t2);
            if (in) {
                r.max_t = t1 >= r.min_t ? t1 : t2;
                i->t = r.max_t;
                Vector3D p = r.o + i->t * r.d;
                Vector3D normal = p - this->o;
                normal.normalize();
                i->n = normal;
                i->bsdf = get_bsdf();
                i->primitive = this;
                return true;
            } else {
                return false;
            }
        }

        void Sphere::draw(const Color &c) const {
            Misc::draw_sphere_opengl(o, r, c);
        }

        void Sphere::drawOutline(const Color &c) const {
            //Misc::draw_sphere_opengl(o, r, c);
        }


    } // namespace StaticScene
} // namespace CGL
