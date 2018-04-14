#include <iostream>
#include <math.h>
#include <random>
#include <vector>

#include "cloth.h"
#include "collision/plane.h"
#include "collision/sphere.h"

using namespace std;

Cloth::Cloth(double width, double height, int num_width_points,
             int num_height_points, float thickness) {
    this->width = width;
    this->height = height;
    this->num_width_points = num_width_points;
    this->num_height_points = num_height_points;
    this->thickness = thickness;

    buildGrid();
    buildClothMesh();
}

Cloth::~Cloth() {
    point_masses.clear();
    springs.clear();

    if (clothMesh) {
        delete clothMesh;
    }
}

void Cloth::buildGrid() {
    // (Part 1.1): Build a grid of masses.
    bool pin;
    vector<int> pins;
    for (vector<int> c: this->pinned) {
        pins.emplace_back(c[0] * this->num_width_points + c[1]);
    }
    double hStep = this->height / (this->num_height_points - 1), wStep = this->width / (this->num_width_points - 1);
    if (this->orientation == HORIZONTAL) {
        for (int j = 0; j < this->num_width_points; j++) {
            for (int i = 0; i < this->num_height_points; i++) {
                pin = find(pins.begin(), pins.end(), i * this->num_width_points + j) != pins.end();
                this->point_masses.emplace_back(PointMass(Vector3D{i * hStep, 1, j * wStep}, pin));
            }
        }
    } else {
        random_device rd;
        mt19937 mt(rd());
        uniform_real_distribution<double> dist(-1.0 / 1000, 1.0 / 1000);
        for (int j = 0; j < this->num_width_points; j++) {
            for (int i = 0; i < this->num_height_points; i++) {
                pin = find(pins.begin(), pins.end(), i * this->num_width_points + j) != pins.end();
                this->point_masses.emplace_back(PointMass(Vector3D{i * hStep, j * wStep, dist(mt)}, pin));
            }
        }
    }

    // (Part 1.2): Add springs
    for (int i = 0; i < this->num_height_points; i++) {
        for (int j = 0; j < this->num_width_points; j++) {
            if (i > 0) { // Top Structural Spring
                this->springs.emplace_back(Spring(&this->point_masses[(i - 1) * this->num_width_points + j],
                                                  &this->point_masses[i * this->num_width_points + j], STRUCTURAL));
            }
            if (j > 0) { // Left Structural Spring
                this->springs.emplace_back(Spring(&this->point_masses[i * this->num_width_points + j - 1],
                                                  &this->point_masses[i * this->num_width_points + j], STRUCTURAL));
            }
            if (i > 0 && j > 0) { // Diagonal Upper Left Shearing Spring
                this->springs.emplace_back(Spring(&this->point_masses[(i - 1) * this->num_width_points + j - 1],
                                                  &this->point_masses[i * this->num_width_points + j], SHEARING));
            }
            if (i > 0 && j < (this->num_width_points - 1)) { // Diagonal Upper Right Shearing Spring
                this->springs.emplace_back(Spring(&this->point_masses[i * this->num_width_points + j],
                                                  &this->point_masses[(i - 1) * this->num_width_points + j + 1],
                                                  SHEARING));
            }
            if (i > 1) { // Top (two away) Bending Spring
                this->springs.emplace_back(Spring(&this->point_masses[(i - 2) * this->num_width_points + j],
                                                  &this->point_masses[i * this->num_width_points + j], BENDING));
            }
            if (j > 1) { // Left (two away) Bending Spring
                this->springs.emplace_back(Spring(&this->point_masses[i * this->num_width_points + j - 2],
                                                  &this->point_masses[i * this->num_width_points + j], BENDING));
            }
        }
    }


}


void Cloth::simulate(double frames_per_sec, double simulation_steps, ClothParameters *cp,
                     vector<Vector3D> external_accelerations,
                     vector<CollisionObject *> *collision_objects) {
    double mass = width * height * cp->density / num_width_points / num_height_points;
    double delta_t = 1.0f / frames_per_sec / simulation_steps;


    // (Part 2.1): Compute total force acting on each point mass.
    Vector3D force(0.0, 0.0, 0.0);
    // Total External Forces
    for (Vector3D a: external_accelerations) {
        force += a * mass;
    }
    for (PointMass &pm : this->point_masses) {
        pm.forces += force;
    }

    double fs, length;
    Vector3D dir;
    // Spring Correction Forces
    for (Spring &s : this->springs) {
        if (s.spring_type == STRUCTURAL) {
            if (cp->enable_structural_constraints) {
                dir = (s.pm_b->position - s.pm_a->position).unit();
                length = (s.pm_a->position - s.pm_b->position).norm();
                fs = cp->ks * (length - s.rest_length);
                s.pm_a->forces += dir * fs;
                s.pm_b->forces += -dir * fs;
            }
        } else if (s.spring_type == SHEARING) {
            if (cp->enable_shearing_constraints) {
                dir = (s.pm_b->position - s.pm_a->position).unit();
                length = (s.pm_a->position - s.pm_b->position).norm();
                fs = cp->ks * (length - s.rest_length);
                s.pm_a->forces += dir * fs;
                s.pm_b->forces += -dir * fs;
            }
        } else {
            if (cp->enable_bending_constraints) {
                dir = (s.pm_b->position - s.pm_a->position).unit();
                length = (s.pm_a->position - s.pm_b->position).norm();
                fs = cp->ks * (length - s.rest_length);
                s.pm_a->forces += dir * fs;
                s.pm_b->forces += -dir * fs;
            }
        }
    }

    // (Part 2.2): Use Verlet integration to compute new point mass positions
    Vector3D pos{0.0, 0.0, 0.0};
    for (PointMass &pm : this->point_masses) {
        if (!pm.pinned) {
            pos = pm.position;
            pm.position = pm.position + (1 - (cp->damping / 100)) * (pm.position - pm.last_position)
                          + (pm.forces / mass) * (delta_t * delta_t);
            pm.last_position = pos;
        }
        pm.forces = Vector3D(0.0, 0.0, 0.0);
    }



    // This won't do anything until you complete Part 4.
    build_spatial_map();
    for (PointMass &pm : point_masses) {
        self_collide(pm, simulation_steps);
    }

    // This won't do anything until you complete Part 3.
    for (PointMass &pm : point_masses) {
        for (CollisionObject *co : *collision_objects) {
            co->collide(pm);
        }
    }


    // (Part 2.3): Constrain the changes to be such that the spring does not change
    // in length more than 10% per timestep [Provot 1995].

    double limit, diff;
    for (Spring &s : this->springs) {
        length = (s.pm_a->position - s.pm_b->position).norm();
        limit = 1.1 * s.rest_length;
        if (length > limit) {
            diff = length - limit;
            dir = (s.pm_b->position - s.pm_a->position).unit();
            if (!s.pm_a->pinned && !s.pm_b->pinned) {
                s.pm_a->position += (diff / 2) * dir;
                s.pm_b->position -= (diff / 2) * dir;
            } else if (!s.pm_a->pinned) {
                s.pm_a->position += diff * dir;
            } else if (!s.pm_b->pinned) {
                s.pm_b->position -= diff * dir;
            }
        }
    }



}

void Cloth::build_spatial_map() {
    for (const auto &entry : map) {
        delete (entry.second);
    }
    map.clear();
    double key;

    // (Part 4.2): Build a spatial map out of all of the point masses.
    for (PointMass &pm : point_masses) {
        key = hash_position(pm.position);
//        cout << key << endl;
        unordered_map<double, vector<PointMass *> *>::const_iterator search = map.find(key);
        if (search == map.end()) {
            vector<PointMass *> *v = new vector<PointMass *>;
            v->emplace_back(&pm);
            map.emplace(key, v);
        } else {
            map[key]->emplace_back(&pm);
        }
    }

}

void Cloth::self_collide(PointMass &pm, double simulation_steps) {
    // (Part 4.3): Handle self-collision for a given point mass.
    double key = hash_position(pm.position);
    double dist, count = 0.0;
    Vector3D dir, correction{0.0, 0.0, 0.0};
    for (PointMass *pom : *map[key]) {
        dist = (pm.position - pom->position).norm();
        if (dist <= 2 * thickness && pom != &pm) {
            count += 1;
            dir = (pm.position - pom->position).unit();
            if (dir.x == 0.0 && dir.y == 0.0 && dir.z == 0.0) {
                dir = -(pm.position).unit();
            }
            correction += dir * (2 * thickness - dist);
        }
    }
    if (count == 0.0) {
        count = 1.0;
    }
    pm.position += correction / (count * simulation_steps);
}

double Cloth::hash_position(Vector3D pos) {
    // (Part 4.1): Hash a 3D position into a unique float identifier that represents
    // membership in some uniquely identified 3D box volume.
    double w = 3 * width / num_width_points, h = 3 * height / num_height_points, t;
    t = max(w, h);
    double x = floor((pos.x / w)), y = floor((pos.y / h)), z = floor((pos.z / t));
    return (x * 31 + y) * 31 + z;
}

///////////////////////////////////////////////////////
/// YOU DO NOT NEED TO REFER TO ANY CODE BELOW THIS ///
///////////////////////////////////////////////////////

void Cloth::reset() {
    PointMass *pm = &point_masses[0];
    for (int i = 0; i < point_masses.size(); i++) {
        pm->position = pm->start_position;
        pm->last_position = pm->start_position;
        pm++;
    }
}

void Cloth::buildClothMesh() {
    if (point_masses.size() == 0) return;

    ClothMesh *clothMesh = new ClothMesh();
    vector<Triangle *> triangles;

    // Create vector of triangles
    for (int y = 0; y < num_height_points - 1; y++) {
        for (int x = 0; x < num_width_points - 1; x++) {
            PointMass *pm = &point_masses[y * num_width_points + x];
            // Both triangles defined by vertices in counter-clockwise orientation
            triangles.push_back(new Triangle(pm, pm + num_width_points, pm + 1));
            triangles.push_back(new Triangle(pm + 1, pm + num_width_points,
                                             pm + num_width_points + 1));
        }
    }

    // For each triangle in row-order, create 3 edges and 3 internal halfedges
    for (int i = 0; i < triangles.size(); i++) {
        Triangle *t = triangles[i];

        // Allocate new halfedges on heap
        Halfedge *h1 = new Halfedge();
        Halfedge *h2 = new Halfedge();
        Halfedge *h3 = new Halfedge();

        // Allocate new edges on heap
        Edge *e1 = new Edge();
        Edge *e2 = new Edge();
        Edge *e3 = new Edge();

        // Assign a halfedge pointer to the triangle
        t->halfedge = h1;

        // Assign halfedge pointers to point masses
        t->pm1->halfedge = h1;
        t->pm2->halfedge = h2;
        t->pm3->halfedge = h3;

        // Update all halfedge pointers
        h1->edge = e1;
        h1->next = h2;
        h1->pm = t->pm1;
        h1->triangle = t;

        h2->edge = e2;
        h2->next = h3;
        h2->pm = t->pm2;
        h2->triangle = t;

        h3->edge = e3;
        h3->next = h1;
        h3->pm = t->pm3;
        h3->triangle = t;
    }

    // Go back through the cloth mesh and link triangles together using halfedge
    // twin pointers

    // Convenient variables for math
    int num_height_tris = (num_height_points - 1) * 2;
    int num_width_tris = (num_width_points - 1) * 2;

    bool topLeft = true;
    for (int i = 0; i < triangles.size(); i++) {
        Triangle *t = triangles[i];

        if (topLeft) {
            // Get left triangle, if it exists
            if (i % num_width_tris != 0) { // Not a left-most triangle
                Triangle *temp = triangles[i - 1];
                t->pm1->halfedge->twin = temp->pm3->halfedge;
            } else {
                t->pm1->halfedge->twin = nullptr;
            }

            // Get triangle above, if it exists
            if (i >= num_width_tris) { // Not a top-most triangle
                Triangle *temp = triangles[i - num_width_tris + 1];
                t->pm3->halfedge->twin = temp->pm2->halfedge;
            } else {
                t->pm3->halfedge->twin = nullptr;
            }

            // Get triangle to bottom right; guaranteed to exist
            Triangle *temp = triangles[i + 1];
            t->pm2->halfedge->twin = temp->pm1->halfedge;
        } else {
            // Get right triangle, if it exists
            if (i % num_width_tris != num_width_tris - 1) { // Not a right-most triangle
                Triangle *temp = triangles[i + 1];
                t->pm3->halfedge->twin = temp->pm1->halfedge;
            } else {
                t->pm3->halfedge->twin = nullptr;
            }

            // Get triangle below, if it exists
            if (i + num_width_tris - 1 < 1.0f * num_width_tris * num_height_tris / 2.0f) { // Not a bottom-most triangle
                Triangle *temp = triangles[i + num_width_tris - 1];
                t->pm2->halfedge->twin = temp->pm3->halfedge;
            } else {
                t->pm2->halfedge->twin = nullptr;
            }

            // Get triangle to top left; guaranteed to exist
            Triangle *temp = triangles[i - 1];
            t->pm1->halfedge->twin = temp->pm2->halfedge;
        }

        topLeft = !topLeft;
    }

    clothMesh->triangles = triangles;
    this->clothMesh = clothMesh;
}
