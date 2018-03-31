#include "bsdf.h"

#include <iostream>
#include <algorithm>
#include <utility>

using std::min;
using std::max;
using std::swap;

namespace CGL {

    void make_coord_space(Matrix3x3 &o2w, const Vector3D &n) {

        Vector3D z = Vector3D(n.x, n.y, n.z);
        Vector3D h = z;
        if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z)) h.x = 1.0;
        else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z)) h.y = 1.0;
        else h.z = 1.0;

        z.normalize();
        Vector3D y = cross(h, z);
        y.normalize();
        Vector3D x = cross(z, y);
        x.normalize();

        o2w[0] = x;
        o2w[1] = y;
        o2w[2] = z;
    }

// Diffuse BSDF //

    Spectrum DiffuseBSDF::f(const Vector3D &wo, const Vector3D &wi) {
        // (Part 3.1):
        // This function takes in both wo and wi and returns the evaluation of
        // the BSDF for those two directions.

        return reflectance / PI;
    }

    Spectrum DiffuseBSDF::sample_f(const Vector3D &wo, Vector3D *wi, double *pdf) {
        // (Part 3.1):
        // This function takes in only wo and provides pointers for wi and pdf,
        // which should be assigned by this function.
        // After sampling a value for wi, it returns the evaluation of the BSDF
        // at (wo, *wi).

        *wi = sampler.get_sample(pdf);
        return reflectance / PI;
    }

// Mirror BSDF //

    Spectrum MirrorBSDF::f(const Vector3D &wo, const Vector3D &wi) {
        return Spectrum();
    }

    Spectrum MirrorBSDF::sample_f(const Vector3D &wo, Vector3D *wi, double *pdf) {
        // Implement MirrorBSDF
        *pdf = 1;
        reflect(wo, wi);
        return reflectance / abs_cos_theta(*wi);
    }

// Microfacet BSDF //

    double MicrofacetBSDF::G(const Vector3D &wo, const Vector3D &wi) {
        return 1.0 / (1.0 + Lambda(wi) + Lambda(wo));
    }

    double MicrofacetBSDF::D(const Vector3D &h) {
        // proj3-2, part 2
        // Compute Beckmann normal distribution function (NDF) here.
        // You will need the roughness alpha.
        double numerator, denominator;
        numerator = exp(-pow(sin_theta(h) / cos_theta(h) / alpha, 2));
        // TODO what is alpha??
        denominator = PI * pow(alpha, 2) * pow(cos_theta(h), 4);
        return numerator / denominator;
    }

    Spectrum MicrofacetBSDF::F(const Vector3D &wi) {
        // proj3-2, part 2
        // Compute Fresnel term for reflection on dielectric-conductor interface.
        // You will need both eta and k, both of which are Spectrum.
        Spectrum Rs, Rp;
        Rs.r = ((eta.r * eta.r) + (k.r * k.r) - 2 * eta.r * cos_theta(wi) + pow(cos_theta(wi), 2)) /
               ((eta.r * eta.r) + (k.r * k.r) + 2 * eta.r * cos_theta(wi) + pow(cos_theta(wi), 2));
        Rs.g = ((eta.g * eta.g) + (k.g * k.g) - 2 * eta.g * cos_theta(wi) + pow(cos_theta(wi), 2)) /
               ((eta.g * eta.g) + (k.g * k.g) + 2 * eta.g * cos_theta(wi) + pow(cos_theta(wi), 2));
        Rs.b = ((eta.b * eta.b) + (k.b * k.b) - 2 * eta.b * cos_theta(wi) + pow(cos_theta(wi), 2)) /
               ((eta.b * eta.b) + (k.b * k.b) + 2 * eta.b * cos_theta(wi) + pow(cos_theta(wi), 2));
        Rp.r = (((eta.r * eta.r) + (k.r * k.r)) * pow(cos_theta(wi), 2) - 2 * eta.r * cos_theta(wi) + 1) /
               (((eta.r * eta.r) + (k.r * k.r)) * pow(cos_theta(wi), 2) + 2 * eta.r * cos_theta(wi) + 1);
        Rp.g = (((eta.g * eta.g) + (k.g * k.g)) * pow(cos_theta(wi), 2) - 2 * eta.g * cos_theta(wi) + 1) /
               (((eta.g * eta.g) + (k.g * k.g)) * pow(cos_theta(wi), 2) + 2 * eta.g * cos_theta(wi) + 1);
        Rp.b = (((eta.b * eta.b) + (k.b * k.b)) * pow(cos_theta(wi), 2) - 2 * eta.b * cos_theta(wi) + 1) /
               (((eta.b * eta.b) + (k.b * k.b)) * pow(cos_theta(wi), 2) + 2 * eta.b * cos_theta(wi) + 1);
        return (Rs + Rp) / 2;
    }

    Spectrum MicrofacetBSDF::f(const Vector3D &wo, const Vector3D &wi) {
        // proj3-2, part 2
        // Implement microfacet model here.
        Vector3D n(0, 0, 1);
        if (dot(n, wi) <= 0 || dot(n, wo) <= 0) {
            return {};
        }
        return (F(wi) * G(wo, wi) * D((wi + wo).unit())) / (4 * dot(n, wo) * dot(n, wi));
    }

    Spectrum MicrofacetBSDF::sample_f(const Vector3D &wo, Vector3D *wi, double *pdf) {
        // proj3-2, part 2
        // *Importance* sample Beckmann normal distribution function (NDF) here.
        // Note: You should fill in the sampled direction *wi and the corresponding *pdf,
        //       and return the sampled BRDF value.

//        *wi = cosineHemisphereSampler.get_sample(pdf); //placeholder

        Vector2D rand = sampler.get_sample();
        double theta = atan(sqrt(-pow(alpha, 2) * log(1 - rand.x)));
        double phi = 2.0 * PI * rand.y;

        double xs = sin(theta) * cos(phi);
        double ys = sin(theta) * sin(phi);
        double zs = cos(theta);

        Vector3D h(xs, ys, zs);
        h.normalize();

        *wi = -wo + 2 * dot(wo, h) * h;

        if (dot(h, *wi) <= 0) { // check for validity of sampled wi
            *pdf = 0;
            return {};
        }

        double pwh, pwwi, pphi, ptheta;

        ptheta = ((2 * sin(theta)) / (pow(alpha, 2) * pow(cos(theta), 3))) * exp(-pow(tan(theta), 2) / pow(alpha, 2));
        pphi = 1 / (2 * PI);
        pwh = (ptheta * pphi) / sin(theta);
        *pdf = pwh / (4 * dot(*wi, h));

        return MicrofacetBSDF::f(wo, *wi);
    }

// Refraction BSDF //

    Spectrum RefractionBSDF::f(const Vector3D &wo, const Vector3D &wi) {
        return Spectrum();
    }

    Spectrum RefractionBSDF::sample_f(const Vector3D &wo, Vector3D *wi, double *pdf) {
        // TODO:
        // Implement RefractionBSDF
        return Spectrum();
    }

// Glass BSDF //

    Spectrum GlassBSDF::f(const Vector3D &wo, const Vector3D &wi) {
        return Spectrum();
    }

    Spectrum GlassBSDF::sample_f(const Vector3D &wo, Vector3D *wi, double *pdf) {
        // 3-2 Part 1 Task 4
        // Compute Fresnel coefficient and either reflect or refract based on it.
        if (refract(wo, wi, ior)) {
            double R0 = (((1. - ior) * (1. - ior)) / ((1. + ior) * (1. + ior))), R, eta;
            R = R0 + (1 - R0) * pow(1 - abs_cos_theta(wo), 5);
            eta = wo.z > 0 ? 1. / ior : ior;
            if (coin_flip(R)) {
                reflect(wo, wi);
                *pdf = R;
                return R * reflectance / abs_cos_theta(*wi);
            } else {
                refract(wo, wi, ior);
                *pdf = 1 - R;
                return (1 - R) * transmittance / abs_cos_theta(*wi) / pow(eta, 2);
            }
        } else {
            reflect(wo, wi);
            *pdf = 1;
            return reflectance / abs_cos_theta(*wi);
        }
    }


    void BSDF::reflect(const Vector3D &wo, Vector3D *wi) {
        // 3-2 Part 1 Task 1
        // Implement reflection of wo about normal (0,0,1) and store result in wi.
        *wi = -wo;
        wi->z = wo.z;
    }

    bool BSDF::refract(const Vector3D &wo, Vector3D *wi, double ior) {
        // 3-2 Part 1 Task 3
        // Use Snell's Law to refract wo surface and store result ray in wi.
        // Return false if refraction does not occur due to total internal reflection
        // and true otherwise. When wo.z is positive, then wo corresponds to a
        // ray entering the surface through vacuum.
        double eta, sgn, root;
        if (wo.z > 0) {
            eta = 1. / ior;
            sgn = -1;
        } else {
            eta = ior;
            sgn = 1;
        }
        root = 1 - (eta * eta) * (1 - (cos_theta(wo) * cos_theta(wo)));
        if (root < 0) { // total internal reflection
            return false;
        } else {
            wi->x = -eta * wo.x;
            wi->y = -eta * wo.y;
            wi->z = sgn * sqrt(root);
        }
        return true;
    }

// Emission BSDF //

    Spectrum EmissionBSDF::f(const Vector3D &wo, const Vector3D &wi) {
        return Spectrum();
    }

    Spectrum EmissionBSDF::sample_f(const Vector3D &wo, Vector3D *wi, double *pdf) {
        *pdf = 1.0 / PI;
        *wi = sampler.get_sample(pdf);
        return Spectrum();
    }

} // namespace CGL
