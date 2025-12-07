// stb_wrapper.cpp
// Created by sophomore on 12/6/25.
//
module;
#include <SDL3/SDL_log.h>
#include <fstream>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
module stb_wrapper;
import data_type;

namespace sopho
{
    ImageData load_image()
    {
        stbi_set_flip_vertically_on_load(true);
        std::string file_name{"assets/test_texture.png"};
        ImageData result;
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

    std::tuple<ImageData, CharData> load_ttf()
    {
        std::ifstream ifs("assets/test.ttf", std::ios::binary);
        if (!ifs)
            return {};

        ifs.seekg(0, std::ios::end);
        size_t size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        std::vector<std::byte> data(size);
        ifs.read((char*)data.data(), size);
        unsigned char bitmap[512 * 512];
        CharData char_data{.pw = 512, .ph = 512};

        stbtt_BakeFontBitmap(reinterpret_cast<const unsigned char*>(data.data()), 0, 32.0f, bitmap, 512, 512, 32, 96,
                             char_data.cdata.data());
        ImageData img{};
        img.width = 512;
        img.height = 512;
        img.channels = 4; // RGBA8

        img.pixels.resize(static_cast<std::size_t>(512) * static_cast<std::size_t>(512) * 4);

        for (int y = 0; y < 512; ++y)
        {
            for (int x = 0; x < 512; ++x)
            {
                int src_index = y * 512 + x;
                int dst_index = src_index * 4;

                unsigned char a = bitmap[src_index];

                img.pixels[dst_index + 0] = std::byte{255}; // R
                img.pixels[dst_index + 1] = std::byte{255}; // G
                img.pixels[dst_index + 2] = std::byte{255}; // B
                img.pixels[dst_index + 3] = std::byte{a}; // A
            }
        }
        return std::make_tuple(img, char_data);
    }

} // namespace sopho
