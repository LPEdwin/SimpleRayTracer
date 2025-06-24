#include <iostream>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <vector>
#include <numbers>

#include "vector3.h"
#include "image.h"
#include "camera.h"
#include "sphere.h"
#include "render.h"

using namespace std;
using namespace std::numbers;


int main()
{
    Camera camera(Vector3(0, 0, 5), Vector3(0, 0, -1), 60.0f, 16.0 / 9.0);
    Sphere sphere(Vector3(0, 0, 0), 1.0f);
    Image image(1280, 720);

    Render(camera, sphere, image);

    try
    {
        auto filename = "output.bmp";
        WriteImageToBMP(image, filename);
        std::cout << "BMP saved to " << filename << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error writing BMP: " << e.what() << "\n";
    }

    return 0;
}
