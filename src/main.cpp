#include<iostream>
#include<cmath>
#include <fstream>
#include <cstdint>
#include <vector>
#include <numbers>

#include "vec3.cpp"
#include "image.cpp"
#include "camera.cpp"
#include "sphere.cpp"

using namespace std;
using namespace std::numbers;

void render(const Camera& camera, const Sphere& sphere, Image& image) {
    
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {            
            float aspectRatio = static_cast<float>(image.width) / image.height;
            float pixelNDCX = (x + 0.5f) / image.width;
            float pixelNDCY = (y + 0.5f) / image.height;
            float pixelScreenX = (2.0f * pixelNDCX - 1.0f) * aspectRatio * tan(camera.fov * 0.5f * pi / 180.0f);
            float pixelScreenY = (1.0f - 2.0f * pixelNDCY) * tan(camera.fov * 0.5f * pi / 180.0f);

            vec3 rayDirection(pixelScreenX, pixelScreenY, -1);
            rayDirection = unit_vector(rayDirection); // Normalize the ray direction
            float t = 0.0f;
            if(sphere.intersect(Ray(camera.position, rayDirection), t)) {
                image.pixels[y][x] = vec3(1.0f, 0.0f, 0.0f); 
            } else {
                image.pixels[y][x] = vec3(0, 0, 0);
            }
        }
    }    
};




int main(){
    Camera camera(vec3(0, 0, 5), vec3(0, 0, -1), 60.0f);
    Sphere sphere(vec3(0, 0, 0), 1.0f);
    Image image(800, 600);

    render(camera, sphere, image);

    try {
        auto filename = "output.bmp";
        writeImageToBMP(image, filename);
        std::cout << "BMP saved to " << filename << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error writing BMP: " << e.what() << "\n";
    }

    return 0;
}
