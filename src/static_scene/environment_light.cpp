#include "environment_light.h"

#include <algorithm>
#include <iostream>
#include <fstream>

namespace CGL {
    namespace StaticScene {

        EnvironmentLight::EnvironmentLight(const HDRImageBuffer *envMap, bool env_hemisphere_sample)
                : envMap(envMap) {
            this->env_hemisphere_sample = env_hemisphere_sample;
            init();
        }

        EnvironmentLight::~EnvironmentLight() {
            delete[] pdf_envmap;
            delete[] conds_y;
            delete[] marginal_y;
        }


        void EnvironmentLight::init() {
            uint32_t w = envMap->w, h = envMap->h;
            pdf_envmap = new double[w * h];
            conds_y = new double[w * h];
            marginal_y = new double[h];
            double marg_density[h];

            std::cout << "[PathTracer] Initializing environment light...";

            // Store the environment map pdf to pdf_envmap
            double sum = 0;
            if (!env_hemisphere_sample) {
                for (int j = 0; j < h; ++j) {
                    for (int i = 0; i < w; ++i) {
                        pdf_envmap[w * j + i] = envMap->data[w * j + i].illum() * sin(M_PI * (j + .5) / h);
                        sum += pdf_envmap[w * j + i];
                    }
                    marg_density[j] = 0;
                }

                for (int j = 0; j < h; ++j) {
                    for (int i = 0; i < w; ++i) {
                        pdf_envmap[w * j + i] /= sum;
                        marg_density[j] += pdf_envmap[w * j + i];
                    }
                }


                // 3-2 Part 3 Task 3 Step 2
                // Store the marginal distribution for y to marginal_y
                double temp;
                for (int j = 0; j < h; j++) {
                    temp = 0;
                    for (int jp = 0; jp < j; jp++) {
                        for (int i = 0; i < w; i++) {
                            temp += pdf_envmap[jp * w + i];
                        }
                    }
                    marginal_y[j] = temp;
                }


                // 3-2 Part 3 Task 3 Step 3
                // Store the conditional distribution for x given y to conds_y

                for (int j = 0; j < h; j++) {
                    for (int i = 0; i < w; i++) {
                        temp = 0;
                        for (int ip = 0; ip < i; ip++) {
                            temp += pdf_envmap[j * w + ip];
                        }
                        conds_y[j * w + i] = temp / marg_density[j];
                    }
                }

                std::cout << " Saving out probability_debug image for debug...";
                save_probability_debug();
            }

            std::cout << " Done!" << std::endl;
        }

// Helper functions

        void EnvironmentLight::save_probability_debug() {
            uint32_t w = envMap->w, h = envMap->h;
            uint8_t *img = new uint8_t[4 * w * h];

            for (int j = 0; j < h; ++j) {
                for (int i = 0; i < w; ++i) {
                    img[4 * (j * w + i) + 3] = 255;
                    img[4 * (j * w + i) + 0] = 255 * marginal_y[j];
                    img[4 * (j * w + i) + 1] = 255 * conds_y[j * w + i];
                }
            }

            lodepng::encode("probability_debug.png", img, w, h);
            delete[] img;
        }

        Vector2D EnvironmentLight::theta_phi_to_xy(const Vector2D &theta_phi) const {
            uint32_t w = envMap->w, h = envMap->h;
            double x = theta_phi.y / 2. / M_PI * w;
            double y = theta_phi.x / M_PI * h;
            return Vector2D(x, y);
        }

        Vector2D EnvironmentLight::xy_to_theta_phi(const Vector2D &xy) const {
            uint32_t w = envMap->w, h = envMap->h;
            double x = xy.x;
            double y = xy.y;
            double phi = x / w * 2.0 * M_PI;
            double theta = y / h * M_PI;
            return Vector2D(theta, phi);
        }

        Vector2D EnvironmentLight::dir_to_theta_phi(const Vector3D &dir) const {
            dir.unit();
            double theta = acos(dir.y);
            double phi = atan2(-dir.z, dir.x) + M_PI;
            return Vector2D(theta, phi);
        }

        Vector3D EnvironmentLight::theta_phi_to_dir(const Vector2D &theta_phi) const {
            double theta = theta_phi.x;
            double phi = theta_phi.y;

            double y = cos(theta);
            double x = cos(phi - M_PI) * sin(theta);
            double z = -sin(phi - M_PI) * sin(theta);

            return Vector3D(x, y, z);
        }

        Spectrum EnvironmentLight::bilerp(const Vector2D &xy) const {
            long right = lround(xy.x), left, v = lround(xy.y);
            double u1 = right - xy.x + .5, v1;
            if (right == 0 || right == envMap->w) {
                left = envMap->w - 1;
                right = 0;
            } else left = right - 1;
            if (v == 0) v1 = v = 1;
            else if (v == envMap->h) {
                v = envMap->h - 1;
                v1 = 0;
            } else v1 = v - xy.y + .5;
            auto bottom = envMap->w * v, top = bottom - envMap->w;
            auto u0 = 1 - u1;
            return (envMap->data[top + left] * u1 + envMap->data[top + right] * u0) * v1 +
                   (envMap->data[bottom + left] * u1 + envMap->data[bottom + right] * u0) * (1 - v1);
        }


        Spectrum EnvironmentLight::sample_L(const Vector3D &p, Vector3D *wi,
                                            double *distToLight,
                                            double *pdf) const {
            // 3-2 Part 3 Tasks 2 and 3 (step 4)
            // First implement uniform sphere sampling for the environment light
            // Later implement full importance sampling
            if (env_hemisphere_sample) {
                /*
                 * Uniform Sampling
                 */
                Spectrum rad;
                *wi = sampler_uniform_sphere.get_sample();
                *distToLight = INF_D;
                rad = bilerp(theta_phi_to_xy(dir_to_theta_phi(*wi)));
                *pdf = 1.0 / (4.0 * PI);
                return rad;
            } else {
                /*
                 * Importance Sampling
                 */
                Vector2D sample = sampler_uniform2d.get_sample(), theta_phi;
                double *ptx, *pty;
                long x, y;
                pty = std::upper_bound(&marginal_y[0], &marginal_y[envMap->h], sample.y);
                y = pty - &marginal_y[0];
                ptx = std::upper_bound(&conds_y[y * envMap->w], &conds_y[(y + 1) * envMap->w], sample.x);
                x = ptx - &conds_y[y * envMap->w];
                Vector2D xy(x + 0.5, y + 0.5);
                theta_phi = xy_to_theta_phi(xy);
                *wi = theta_phi_to_dir(theta_phi);
                *distToLight = INF_D;
                if (sin(theta_phi.x) == 0) {
                    *pdf = 0;
                    return {};
                }
                *pdf = pdf_envmap[y * envMap->w + x] * (envMap->w * envMap->h) /
                       (2 * pow(PI, 2) * sin(theta_phi.x));
                return *pdf == 0 ? Spectrum{} : bilerp(xy);
            }
        }

        Spectrum EnvironmentLight::sample_dir(const Ray &r) const {
            // 3-2 Part 3 Task 1
            // Use the helper functions to convert r.d into (x,y)
            // then bilerp the return value
            Vector2D xy = theta_phi_to_xy(dir_to_theta_phi(r.d));

            return bilerp(xy);

        }

    } // namespace StaticScene
} // namespace CGL
