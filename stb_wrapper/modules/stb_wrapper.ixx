// stb_wrapper.ixx
// Created by sophomore on 12/6/25.
//
module;
#include <array>
#include <cstdint>
#include <stb_truetype.h>
#include <tuple>
export module stb_wrapper;
import data_type;
export namespace sopho
{
    ImageData load_image();
    struct CharData
    {
        std::array<stbtt_bakedchar, 96> cdata{};
        std::int32_t pw{};
        std::int32_t ph{};
    };
    std::tuple<ImageData, CharData> load_ttf();
} // namespace sopho
