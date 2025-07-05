#pragma once

#include <fstream>
#include <cstdint>
#include <vector>
#include <iostream>
#include "core/vector3.h"

struct Image
{
    int width, height;
    Vector3 **pixels;

    Image(int w, int h) : width(w), height(h)
    {
        pixels = new Vector3 *[height];
        for (int i = 0; i < height; ++i)
        {
            pixels[i] = new Vector3[width];
        }
    }

    ~Image()
    {
        for (int i = 0; i < height; ++i)
        {
            delete[] pixels[i];
        }
        delete[] pixels;
    }
};

inline double LinearTosRGB(double x)
{
    if (x <= 0.0031308)
        return 12.92 * x;
    else
        return 1.055 * std::pow(x, 1.0 / 2.4) - 0.055;
}

// Write a 24-bit BMP file (no compression)
void SaveBmp(const Image &image, const std::string &filename, bool convertToSRGB = false)
{
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs)
    {
        throw std::runtime_error("Cannot open file for writing: " + filename);
    }

    int width = image.width;
    int height = image.height;

    // BMP rows are padded to multiples of 4 bytes
    int rowSize = (3 * width + 3) & (~3);
    int dataSize = rowSize * height;
    int fileSize = 54 + dataSize;

    // BMP file header (14 bytes)
    uint8_t fileHeader[14] = {
        'B', 'M',   // Signature
        0, 0, 0, 0, // File size (will fill later)
        0, 0,       // Reserved
        0, 0,       // Reserved
        54, 0, 0, 0 // Pixel data offset
    };

    fileHeader[2] = (uint8_t)(fileSize);
    fileHeader[3] = (uint8_t)(fileSize >> 8);
    fileHeader[4] = (uint8_t)(fileSize >> 16);
    fileHeader[5] = (uint8_t)(fileSize >> 24);

    // DIB header (BITMAPINFOHEADER, 40 bytes)
    uint8_t dibHeader[40] = {
        40, 0, 0, 0, // Header size
        0, 0, 0, 0,  // Width (fill later)
        0, 0, 0, 0,  // Height (fill later)
        1, 0,        // Color planes
        24, 0,       // Bits per pixel
        0, 0, 0, 0,  // Compression (0 = none)
        0, 0, 0, 0,  // Image size (can be 0)
        0, 0, 0, 0,  // X pixels per meter (optional)
        0, 0, 0, 0,  // Y pixels per meter (optional)
        0, 0, 0, 0,  // Colors in color table (0 = default)
        0, 0, 0, 0   // Important colors (0 = all)
    };

    dibHeader[4] = (uint8_t)(width);
    dibHeader[5] = (uint8_t)(width >> 8);
    dibHeader[6] = (uint8_t)(width >> 16);
    dibHeader[7] = (uint8_t)(width >> 24);

    dibHeader[8] = (uint8_t)(height);
    dibHeader[9] = (uint8_t)(height >> 8);
    dibHeader[10] = (uint8_t)(height >> 16);
    dibHeader[11] = (uint8_t)(height >> 24);

    // Write headers
    ofs.write(reinterpret_cast<char *>(fileHeader), 14);
    ofs.write(reinterpret_cast<char *>(dibHeader), 40);

    // Write pixel data (BGR format, bottom-up)
    std::vector<uint8_t> padding(rowSize - 3 * width, 0);

    for (int y = height - 1; y >= 0; --y)
    {
        for (int x = 0; x < width; ++x)
        {
            auto clamp = [](float c) -> uint8_t
            {
                if (c < 0.0f)
                    return 0;
                if (c > 1.0f)
                    return 255;
                return static_cast<uint8_t>(c * 255.0f);
            };

            uint8_t r = clamp(convertToSRGB ? LinearTosRGB(image.pixels[y][x].x()) : image.pixels[y][x].x());
            uint8_t g = clamp(convertToSRGB ? LinearTosRGB(image.pixels[y][x].y()) : image.pixels[y][x].y());
            uint8_t b = clamp(convertToSRGB ? LinearTosRGB(image.pixels[y][x].z()) : image.pixels[y][x].z());

            // BMP uses BGR order
            ofs.put(b);
            ofs.put(g);
            ofs.put(r);
        }
        ofs.write(reinterpret_cast<char *>(padding.data()), padding.size());
    }

    if (!ofs)
    {
        throw std::runtime_error("Failed to write BMP data to file: " + filename);
    }

    ofs.close();
}

void SaveBmp_sRGB(const Image &image, const std::string &filename)
{
    SaveBmp(image, filename, true);
}
