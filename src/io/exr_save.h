#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include "tinyexr/tinyexr.h"   
#include "image.h"

inline void SaveEXRImage(const Image& image, const std::string& filename)
{
    int width  = image.width;
    int height = image.height;

    std::vector<float> images[3];
    images[0].resize(width * height); // R
    images[1].resize(width * height); // G
    images[2].resize(width * height); // B

    // Copy image into planar format
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int idx = y * width + x;
            const auto& c = image.pixels[y][x];

            images[0][idx] = c.x();
            images[1][idx] = c.y();
            images[2][idx] = c.z();
        }
    }

    EXRImage exrImage;
    InitEXRImage(&exrImage);
    exrImage.num_channels = 3;

    std::vector<float*> image_ptrs(3);
    image_ptrs[0] = images[2].data(); // B
    image_ptrs[1] = images[1].data(); // G
    image_ptrs[2] = images[0].data(); // R

    exrImage.images = reinterpret_cast<unsigned char**>(image_ptrs.data());
    exrImage.width  = width;
    exrImage.height = height;

    EXRHeader exrHeader;
    InitEXRHeader(&exrHeader);

    exrHeader.num_channels = 3;
    exrHeader.channels = (EXRChannelInfo*)malloc(sizeof(EXRChannelInfo) * 3);
    strcpy(exrHeader.channels[0].name, "B");
    strcpy(exrHeader.channels[1].name, "G");
    strcpy(exrHeader.channels[2].name, "R");

    exrHeader.pixel_types = (int*)malloc(sizeof(int) * 3);
    exrHeader.requested_pixel_types = (int*)malloc(sizeof(int) * 3);
    for (int i = 0; i < 3; i++)
    {
        exrHeader.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
        exrHeader.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
    }

    const char* err = nullptr;
    int ret = SaveEXRImageToFile(&exrImage, &exrHeader, filename.c_str(), &err);
    if (ret != TINYEXR_SUCCESS)
    {
        std::string error = err ? err : "Unknown EXR error";
        FreeEXRErrorMessage(err);
        throw std::runtime_error("Failed to save EXR: " + error);
    }

    free(exrHeader.channels);
    free(exrHeader.pixel_types);
    free(exrHeader.requested_pixel_types);
}