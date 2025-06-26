#pragma once

#include <numbers>
#include "vector3.h"
#include "ray.h"
#include "random.h"

using namespace std;
using namespace std::numbers;

class Camera
{
public:
    Vector3 origin;
    Vector3 target;
    double fovInDegree;
    double aspectRatio;
    double focusDistance;
    double aperture;
    double exposureStart;
    double exposureEnd;
    Vector3 cameraUp;

    Vector3 unitTargetDir;
    Vector3 unitHorizontal;
    // top to bottom
    Vector3 unitVertical;
    Vector3 horizontal;
    // top to bottom
    Vector3 vertical;
    Vector3 topLeft;
    double lensRadius = 0.0;

public:
    Camera() : Camera(Vector3(0, 0, 0), Vector3(0, 0, -1), 90.0, 16.0 / 9.0) {}

    Camera(const Vector3 &origin, const Vector3 &target, double fovInDegree, double aspectRatio)
        : Camera(origin, target, fovInDegree, aspectRatio, 1.0, 0.0, 0.0, 0.0, Vector3(0, 1, 0)) {}

    Camera(const Vector3 &origin, const Vector3 &target, double fovInDegree, double aspectRatio,
           double focusDistance, double aperture)
        : Camera(origin, target, fovInDegree, aspectRatio, focusDistance, aperture, 0.0, 0.0, Vector3(0, 1, 0)) {}

    Camera(const Vector3 &origin, const Vector3 &target, double fovInDegree, double aspectRatio,
           double focusDistance, double aperture, double exposureStart)
        : Camera(origin, target, fovInDegree, aspectRatio, focusDistance, aperture, exposureStart, 0.0, Vector3(0, 1, 0)) {}

    Camera(const Vector3 &origin, const Vector3 &target, double fovInDegree, double aspectRatio,
           double focusDistance, double aperture, double exposureStart, double exposureEnd, const Vector3 &cameraUp)
        : origin(origin), target(target), fovInDegree(fovInDegree), aspectRatio(aspectRatio),
          focusDistance(focusDistance), aperture(aperture), exposureStart(exposureStart), exposureEnd(exposureEnd),
          cameraUp(cameraUp)
    {

        const double fovInRad = fovInDegree * (pi / 180.0);
        const double viewportHeight = 2.0 * std::tan(fovInRad / 2.0);
        const double viewportWidth = aspectRatio * viewportHeight;

        unitTargetDir = UnitVector(target - origin);
        unitHorizontal = UnitVector(Cross(cameraUp, -unitTargetDir));
        // top to bottom
        unitVertical = UnitVector(Cross(unitTargetDir, unitHorizontal));

        horizontal = focusDistance * viewportWidth * unitHorizontal;
        vertical = focusDistance * viewportHeight * unitVertical;

        topLeft = origin + focusDistance * unitTargetDir - horizontal / 2.0 - vertical / 2.0;

        lensRadius = aperture / 2.0;
    }

    // Get the ray for a pixel at (u, v) in normalized screen space coordinates
    // where u and v are in the range [0, 1].
    // and (0,0) corresponds to the top-left corner of the image.
    Ray GetRay(double u, double v) const
    {
        Vector3 offset = lensRadius * RandomInUnitDisk();
        Vector3 newRayOrigin = origin + unitHorizontal * offset.x() + unitVertical * offset.y();
        Vector3 screenPoint = topLeft + u * horizontal + v * vertical;
        double time = (exposureStart == exposureEnd) ? exposureStart : RandomDouble(exposureStart, exposureEnd);
        return Ray(newRayOrigin, screenPoint - newRayOrigin, time);
    }
};
