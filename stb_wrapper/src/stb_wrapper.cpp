// stb_wrapper.cpp
// Created by sophomore on 12/6/25.
//
module;
#include <string>
#include <SDL3/SDL_log.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
module stb_wrapper;
import data_type;

namespace sopho
{
    ImageData load_image()
    {
        stbi_set_flip_vertically_on_load(true);
        std::string file_name{"assets/test_texture.png"};
        sopho::ImageData result;
        auto data = stbi_load(file_name.data(), &result.width, &result.height, &result.channels, 4);
        result.channels = 4;

        if (!data)
        {
            SDL_Log("stbi_load failed for %s: %s", file_name.data(), stbi_failure_reason());
        }
        else
        {
            result.pixels.assign(reinterpret_cast<std::byte*>(data),
                                 reinterpret_cast<std::byte*>(data) + result.width * result.height * result.channels);
            stbi_image_free(data);
            SDL_Log("stbi_load succeeded, w: %d h:%d ch:%d", result.width, result.height, result.channels);
        }
        return result;
    }
} // namespace sopho
