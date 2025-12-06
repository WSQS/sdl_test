// stb_wrapper.ixx
// Created by sophomore on 12/6/25.
//
module;
#include <tuple>
export module stb_wrapper;
import data_type;
export namespace sopho
{
    ImageData load_image();
    struct CharData;
    std::tuple<ImageData,CharData*> load_ttf();
} // namespace sopho
