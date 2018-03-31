#include "triangle.h"

#include "CGL/CGL.h"
#include "GL/glew.h"

namespace CGL {
    namespace StaticScene {

        Triangle::Triangle(const Mesh *mesh, size_t v1, size_t v2, size_t v3) :
                mesh(mesh), v1(v1), v2(v2), v3(v3) {}

        BBox Triangle::get_bbox() const {

            Vector3D p1(mesh->positions[v1]), p2(mesh->positions[v2]), p3(mesh->positions[v3]);
            BBox bb(p1);
            bb.expand(p2);
            bb.expand(p3);
            return bb;

        }

        bool Triangle::intersect(const Ray &r) const {
            // Part 1 Task 3: implement ray-triangle intersection
            Vector3D E1, E2, S, S1, S2;
            Vector3D p0(mesh->positions[v1]), p1(mesh->positions[v2]), p2(mesh->positions[v3]);
            E1 = p1 - p0;
            E2 = p2 - p0;
            S = r.o - p0;
            S1 = cross(r.d, E2);
            S2 = cross(S, E1);
            double t = dot(S2, E2) / dot(S1, E1);
            if (r.min_t <= t && t <= r.max_t) {
                r.max_t = t;
                return true;
            } else {
                return false;
            }
        }

        bool Triangle::intersect(const Ray &r, Intersection *isect) const {
            // Part 1 Task 3:
            // implement ray-triangle intersection. When an intersection takes
            // place, the Intersection data should be updated accordingly
            Vector3D E1, E2, S, S1, S2;
            Vector3D p0(mesh->positions[v1]), p1(mesh->positions[v2]), p2(mesh->positions[v3]);
            E1 = p1 - p0;
            E2 = p2 - p0;
            S = r.o - p0;
            S1 = cross(r.d, E2);
            S2 = cross(S, E1);
            double norm = 1.0 / (dot(S1, E1)), gamm;
            Vector3D inter = norm * Vector3D(dot(S2, E2), dot(S1, S), dot(S2, r.d));
            gamm = 1 - inter.y - inter.z;
            if (r.min_t <= inter.x && inter.x <= r.max_t && 0 <= inter.y && inter.y <= 1 && 0 <= inter.z &&
                inter.z <= 1 && 0 <= gamm && gamm <= 1) {
                r.max_t = inter.x;
                isect->t = inter.x;

                Vector3D n1(mesh->normals[v1]), n2(mesh->normals[v2]), n3(mesh->normals[v3]);

                // Normal using Barycentric Coordinates
                isect->n = gamm * n1 + inter.y * n2 + inter.z * n3;

                // Primitive
                isect->primitive = this;

                // BSDF
                isect->bsdf = get_bsdf();

                return true;
            } else {
                return false;
            }
        }

        void Triangle::draw(const Color &c) const {
            glColor4f(c.r, c.g, c.b, c.a);
            glBegin(GL_TRIANGLES);
            glVertex3d(mesh->positions[v1].x,
                       mesh->positions[v1].y,
                       mesh->positions[v1].z);
            glVertex3d(mesh->positions[v2].x,
                       mesh->positions[v2].y,
                       mesh->positions[v2].z);
            glVertex3d(mesh->positions[v3].x,
                       mesh->positions[v3].y,
                       mesh->positions[v3].z);
            glEnd();
        }

        void Triangle::drawOutline(const Color &c) const {
            glColor4f(c.r, c.g, c.b, c.a);
            glBegin(GL_LINE_LOOP);
            glVertex3d(mesh->positions[v1].x,
                       mesh->positions[v1].y,
                       mesh->positions[v1].z);
            glVertex3d(mesh->positions[v2].x,
                       mesh->positions[v2].y,
                       mesh->positions[v2].z);
            glVertex3d(mesh->positions[v3].x,
                       mesh->positions[v3].y,
                       mesh->positions[v3].z);
            glEnd();
        }


    } // namespace StaticScene
} // namespace CGL
