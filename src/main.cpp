#include<iostream>
#include<cmath>
#include <fstream>
#include <cstdint>
#include <vector>
using namespace std;

const float PI = 3.14159265358979323846;

struct Vector3 {
    float x, y, z;
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    float length() const {
        return sqrt(x*x + y*y + z*z);
    }

    Vector3 normalized() const {
        float len = length();
        return (len > 0) ? Vector3(x/len, y/len, z/len) : Vector3(0,0,0);
    }
};

struct Ray {
    Vector3 origin;
    Vector3 direction;    
    Ray(Vector3 o, Vector3 d) : origin(o), direction(d) {}
};

struct Camera {
    Vector3 position;
    Vector3 direction;
    float fov;
    Camera() : position(), direction(0, 0, -1), fov(60.0f) {}
    Camera(Vector3 pos, Vector3 dir, float fov) : position(pos), direction(dir), fov(fov) {}
};

struct Sphere {
    Vector3 center;
    float radius;
    Sphere() : center(), radius(1.0f) {}
    Sphere(Vector3 c, float r) : center(c), radius(r) {}

    bool intersect(const Ray& ray, float& t) const {
        Vector3 oc = ray.origin - center;
        float a = ray.direction.x * ray.direction.x + ray.direction.y * ray.direction.y + ray.direction.z * ray.direction.z;
        float b = 2.0f * (oc.x * ray.direction.x + oc.y * ray.direction.y + oc.z * ray.direction.z);
        float c = oc.x * oc.x + oc.y * oc.y + oc.z * oc.z - radius * radius;
        float discriminant = b * b - 4 * a * c;
        if (discriminant < 0) {
            return false; 
        } else {
            t = (-b - sqrt(discriminant)) / (2.0f * a);
            return true; 
        }
    }
};

struct Image{
    int width, height;
    Vector3** pixels;

    Image(int w, int h) : width(w), height(h) {
        pixels = new Vector3*[height];
        for (int i = 0; i < height; ++i) {
            pixels[i] = new Vector3[width];
        }
    }

    ~Image() {
        for (int i = 0; i < height; ++i) {
            delete[] pixels[i];
        }
        delete[] pixels;
    }
};


void render(const Camera& camera, const Sphere& sphere, Image& image) {
    
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {            
            float aspectRatio = static_cast<float>(image.width) / image.height;
            float pixelNDCX = (x + 0.5f) / image.width;
            float pixelNDCY = (y + 0.5f) / image.height;
            float pixelScreenX = (2.0f * pixelNDCX - 1.0f) * aspectRatio * tan(camera.fov * 0.5f * PI / 180.0f);
            float pixelScreenY = (1.0f - 2.0f * pixelNDCY) * tan(camera.fov * 0.5f * PI / 180.0f);

            Vector3 rayDirection(pixelScreenX, pixelScreenY, -1);
            rayDirection = rayDirection.normalized();
            float t = 0.0f;
            if(sphere.intersect(Ray(camera.position, rayDirection), t)) {
                image.pixels[y][x] = Vector3(1.0f, 0.0f, 0.0f); 
            } else {
                image.pixels[y][x] = Vector3(0, 0, 0);
            }
        }
    }    
};

#include <fstream>
#include <cstdint>

// Write a 24-bit BMP file (no compression)
void writeImageToBMP(const Image& image, const std::string& filename) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        std::cerr << "Cannot open file for writing: " << filename << "\n";
        return;
    }

    int width = image.width;
    int height = image.height;

    // BMP rows are padded to multiples of 4 bytes
    int rowSize = (3 * width + 3) & (~3);
    int dataSize = rowSize * height;
    int fileSize = 54 + dataSize;

    // BMP file header (14 bytes)
    uint8_t fileHeader[14] = {
        'B', 'M',                  // Signature
        0,0,0,0,                  // File size (will fill later)
        0,0,                      // Reserved
        0,0,                      // Reserved
        54,0,0,0                  // Pixel data offset
    };

    fileHeader[2] = (uint8_t)(fileSize      );
    fileHeader[3] = (uint8_t)(fileSize >>  8);
    fileHeader[4] = (uint8_t)(fileSize >> 16);
    fileHeader[5] = (uint8_t)(fileSize >> 24);

    // DIB header (BITMAPINFOHEADER, 40 bytes)
    uint8_t dibHeader[40] = {
        40,0,0,0,                 // Header size
        0,0,0,0,                  // Width (fill later)
        0,0,0,0,                  // Height (fill later)
        1,0,                      // Color planes
        24,0,                     // Bits per pixel
        0,0,0,0,                  // Compression (0 = none)
        0,0,0,0,                  // Image size (can be 0)
        0,0,0,0,                  // X pixels per meter (optional)
        0,0,0,0,                  // Y pixels per meter (optional)
        0,0,0,0,                  // Colors in color table (0 = default)
        0,0,0,0                   // Important colors (0 = all)
    };

    dibHeader[4] = (uint8_t)(width      );
    dibHeader[5] = (uint8_t)(width >>  8);
    dibHeader[6] = (uint8_t)(width >> 16);
    dibHeader[7] = (uint8_t)(width >> 24);

    dibHeader[8]  = (uint8_t)(height      );
    dibHeader[9]  = (uint8_t)(height >>  8);
    dibHeader[10] = (uint8_t)(height >> 16);
    dibHeader[11] = (uint8_t)(height >> 24);

    // Write headers
    ofs.write(reinterpret_cast<char*>(fileHeader), 14);
    ofs.write(reinterpret_cast<char*>(dibHeader), 40);

    // Write pixel data (BGR format, bottom-up)
    std::vector<uint8_t> padding(rowSize - 3 * width, 0);

    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            auto clamp = [](float c) -> uint8_t {
                if (c < 0.0f) return 0;
                if (c > 1.0f) return 255;
                return static_cast<uint8_t>(c * 255.0f);
            };

            uint8_t r = clamp(image.pixels[y][x].x);
            uint8_t g = clamp(image.pixels[y][x].y);
            uint8_t b = clamp(image.pixels[y][x].z);

            // BMP uses BGR order
            ofs.put(b);
            ofs.put(g);
            ofs.put(r);
        }
        ofs.write(reinterpret_cast<char*>(padding.data()), padding.size());
    }

    ofs.close();
    std::cout << "BMP saved to " << filename << "\n";
}


int main(){
    Camera camera(Vector3(0, 0, 5), Vector3(0, 0, -1), 60.0f);
    Sphere sphere(Vector3(0, 0, 0), 1.0f);
    Image image(800, 600);

    render(camera, sphere, image);
    writeImageToBMP(image, "output.bmp");
    return 0;
}
