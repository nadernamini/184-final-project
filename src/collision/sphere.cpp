#include <nanogui/nanogui.h>

#include "../clothMesh.h"
#include "../misc/sphere_drawing.h"
#include "sphere.h"

using namespace nanogui;
using namespace CGL;

void Sphere::collide(PointMass &pm) {
    // (Part 3.1): Handle collisions with spheres.
    if ((pm.position - this->origin).norm() <= this->radius) {
        Vector3D dir = (pm.position - this->origin).unit();
        Vector3D tanPt = this->origin + this->radius * dir;
        Vector3D correction = tanPt - pm.last_position;
        Vector3D lastP = pm.position;
        pm.position = pm.last_position + (1 - friction) * correction;
//        pm.last_position = lastP;
    }
}

void Sphere::render(GLShader &shader) {
    // We decrease the radius here so flat triangles don't behave strangely
    // and intersect with the sphere when rendered
    Misc::draw_sphere(shader, origin, radius * 0.92);
}
