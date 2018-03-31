#include "bbox.h"

#include "GL/glew.h"

#include <iostream>


namespace CGL {

    bool BBox::intersect(const Ray &r, double &t0, double &t1) const {

        // Part 2 Task 2
        // Implement ray - bounding box intersection test
        // If the ray intersected the bounding box within the range given by
        // t0, t1, update t0 and t1 with the new intersection times.
        double t_min_y, t_min_z, t_max_y, t_max_z;

        t0 = (bbs[r.sign[0]].x - r.o.x) * r.inv_d.x;
        t1 = (bbs[1 - r.sign[0]].x - r.o.x) * r.inv_d.x;
        t_min_y = (bbs[r.sign[1]].y - r.o.y) * r.inv_d.y;
        t_max_y = (bbs[1 - r.sign[1]].y - r.o.y) * r.inv_d.y;
        if ((t0 > t_max_y) || (t_min_y > t1))
            return false;
        if (t_min_y > t0)
            t0 = t_min_y;
        if (t_max_y < t1)
            t1 = t_max_y;
        t_min_z = (bbs[r.sign[2]].z - r.o.z) * r.inv_d.z;
        t_max_z = (bbs[1 - r.sign[2]].z - r.o.z) * r.inv_d.z;

        if ((t0 > t_max_z) || (t_min_z > t1))
            return false;
        if (t_min_z > t0)
            t0 = t_min_z;
        if (t_max_z < t1)
            t1 = t_max_z;

        return std::max(t0, r.min_t) <= std::min(t1, r.max_t);
    }

    void BBox::draw(Color c) const {

        glColor4f(c.r, c.g, c.b, c.a);

        // top
        glBegin(GL_LINE_STRIP);
        glVertex3d(max.x, max.y, max.z);
        glVertex3d(max.x, max.y, min.z);
        glVertex3d(min.x, max.y, min.z);
        glVertex3d(min.x, max.y, max.z);
        glVertex3d(max.x, max.y, max.z);
        glEnd();

        // bottom
        glBegin(GL_LINE_STRIP);
        glVertex3d(min.x, min.y, min.z);
        glVertex3d(min.x, min.y, max.z);
        glVertex3d(max.x, min.y, max.z);
        glVertex3d(max.x, min.y, min.z);
        glVertex3d(min.x, min.y, min.z);
        glEnd();

        // side
        glBegin(GL_LINES);
        glVertex3d(max.x, max.y, max.z);
        glVertex3d(max.x, min.y, max.z);
        glVertex3d(max.x, max.y, min.z);
        glVertex3d(max.x, min.y, min.z);
        glVertex3d(min.x, max.y, min.z);
        glVertex3d(min.x, min.y, min.z);
        glVertex3d(min.x, max.y, max.z);
        glVertex3d(min.x, min.y, max.z);
        glEnd();

    }

    std::ostream &operator<<(std::ostream &os, const BBox &b) {
        return os << "BBOX(" << b.min << ", " << b.max << ")";
    }

} // namespace CGL
