#pragma once

#include <numbers>
#include "vec3.cpp"
#include "ray.cpp"
#include "random.cpp"

using namespace std;
using namespace std::numbers;

class Camera {
public:
    vec3 position;
    vec3 target;
    double fovInDegree;
    double aspectRatio;
    double focusDistance;
    double aperture;
    double exposureStart;
    double exposureEnd;
    vec3 cameraUp;

    vec3 horizontal;
    vec3 vertical;
    vec3 leftBottom;

private:
    vec3 unitTargetDir;
    vec3 unitHorizontal;
    vec3 unitVertical;
    double lensRadius = 0.0;

public:
    Camera() : Camera(vec3(0, 0, 0), vec3(0, 0, 1), 90.0, 16.0 / 9.0) {}

    Camera(const vec3& position, const vec3& target, double fovInDegree, double aspectRatio)
        : Camera(position, target, fovInDegree, aspectRatio, 1.0, 0.0, 0.0, 0.0, vec3(0, 1, 0)) {}

    Camera(const vec3& position, const vec3& target, double fovInDegree, double aspectRatio,
           double focusDistance, double aperture)
        : Camera(position, target, fovInDegree, aspectRatio, focusDistance, aperture, 0.0, 0.0, vec3(0, 1, 0)) {}

    Camera(const vec3& position, const vec3& target, double fovInDegree, double aspectRatio,
           double focusDistance, double aperture, double exposureStart)
        : Camera(position, target, fovInDegree, aspectRatio, focusDistance, aperture, exposureStart, 0.0, vec3(0, 1, 0)) {}

    Camera(const vec3& position, const vec3& target, double fovInDegree, double aspectRatio,
           double focusDistance, double aperture, double exposureStart, double exposureEnd, const vec3& cameraUp)
        : position(position), target(target), fovInDegree(fovInDegree), aspectRatio(aspectRatio),
          focusDistance(focusDistance), aperture(aperture), exposureStart(exposureStart), exposureEnd(exposureEnd),
          cameraUp(cameraUp) {

        const double fovInRad = fovInDegree * (pi / 180.0);
        unitTargetDir = unit_vector(target - position);

        const double viewportHeight = 2.0 * std::tan(fovInRad / 2.0);
        const double viewportWidth = aspectRatio * viewportHeight;

        unitHorizontal = unit_vector(cross(-unitTargetDir, cameraUp));
        horizontal = focusDistance * viewportWidth * unitHorizontal;
        unitVertical = unit_vector(cross(horizontal, -unitTargetDir));
        vertical = focusDistance * viewportHeight * unitVertical;

        leftBottom = position + focusDistance * unitTargetDir - horizontal / 2.0 - vertical / 2.0;

        lensRadius = aperture / 2.0;
    }

    Ray get_ray(double u, double v) const {
        vec3 offset = lensRadius * random_in_unit_disk();
        vec3 originWithOffset = position + unitHorizontal * offset.x() + unitVertical * offset.y();
        vec3 screenPoint = leftBottom + u * horizontal + v * vertical;
        double time = (exposureStart == exposureEnd) ? exposureStart : random_double(exposureStart, exposureEnd);
        return Ray(originWithOffset, screenPoint - originWithOffset, time);
    }
};
