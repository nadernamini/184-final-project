#include "sampler.h"

namespace CGL {

// Uniform Sampler2D Implementation //

    Vector2D UniformGridSampler2D::get_sample() const {

        return Vector2D(random_uniform(), random_uniform());

    }

// Uniform Hemisphere Sampler3D Implementation //

    Vector3D UniformHemisphereSampler3D::get_sample() const {

        double Xi1 = random_uniform();
        double Xi2 = random_uniform();

        double theta = acos(Xi1);
        double phi = 2.0 * PI * Xi2;

        double xs = sin(theta) * cos(phi);
        double ys = sin(theta) * sin(phi);
        double zs = cos(theta);

        return Vector3D(xs, ys, zs);

    }

    // Uniform Sphere Sampler3D Implementation //

    Vector3D UniformSphereSampler3D::get_sample() const {
        double z = random_uniform() * 2 - 1;
        double sinTheta = sqrt(std::max(0.0, 1.0f - z * z));

        double phi = 2.0f * M_PI * random_uniform();

        return Vector3D(cos(phi) * sinTheta, sin(phi) * sinTheta, z);
    }

    Vector3D CosineWeightedHemisphereSampler3D::get_sample() const {
        double f;
        return get_sample(&f);
    }

    Vector3D CosineWeightedHemisphereSampler3D::get_sample(double *pdf) const {

        double Xi1 = random_uniform();
        double Xi2 = random_uniform();

        double r = sqrt(Xi1);
        double theta = 2. * PI * Xi2;
        *pdf = sqrt(1 - Xi1) / PI;
        return Vector3D(r * cos(theta), r * sin(theta), sqrt(1 - Xi1));
    }

    Vector3D JitteredSampler3D::get_sample() const {
        double f;
        return get_sample(&f);
    }

    Vector3D JitteredSampler3D::get_sample(double *pdf) const {
        return Vector3D(0.0, 0.0, 0.0);
    }

} // namespace CGL
