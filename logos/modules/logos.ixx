// logos.ixx
// Created by wsqsy on 11/26/2025.
//
module;
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
export module logos;

namespace sopho
{
    export template <typename TScalar, std::uint8_t Row, std::uint8_t Col>
    class Mat
    {
        std::array<std::array<TScalar, Col>, Row> m_data{};

    public:
        TScalar& operator()(std::uint8_t row, std::uint8_t col) { return m_data[row][col]; }
        const TScalar& operator()(std::uint8_t row, std::uint8_t col) const { return m_data[row][col]; }
        TScalar& operator()(std::uint8_t row)
            requires(Col == 1)
        {
            return m_data[row][0];
        }
        TScalar* data() { return m_data[0].data(); }
        template <std::uint8_t OtherCol>
        Mat<TScalar, Row, OtherCol> operator*(const Mat<TScalar, Col, OtherCol>& rhs) const
        {
            Mat<TScalar, Row, OtherCol> result{};

            for (std::uint8_t r = 0; r < Row; ++r)
            {
                for (std::uint8_t c = 0; c < OtherCol; ++c)
                {
                    TScalar sum{};
                    for (std::uint8_t k = 0; k < Col; ++k)
                    {
                        sum += (*this)(r, k) * rhs(k, c);
                    }
                    result(r, c) = sum;
                }
            }

            return result;
        }
    };

    export Mat<float, 4, 4> make_rotation_y(float yaw)
    {
        float cy = std::cos(yaw);
        float sy = std::sin(yaw);

        Mat<float, 4, 4> R{};

        R(0, 0) = cy;
        R(0, 1) = 0;
        R(0, 2) = sy;
        R(0, 3) = 0;
        R(1, 0) = 0;
        R(1, 1) = 1;
        R(1, 2) = 0;
        R(1, 3) = 0;
        R(2, 0) = -sy;
        R(2, 1) = 0;
        R(2, 2) = cy;
        R(2, 3) = 0;
        R(3, 0) = 0;
        R(3, 1) = 0;
        R(3, 2) = 0;
        R(3, 3) = 1;

        return R;
    }

    export Mat<float, 4, 4> make_rotation_x(float pitch)
    {
        float cp = std::cos(pitch);
        float sp = std::sin(pitch);

        Mat<float, 4, 4> R{};

        R(0, 0) = 1;
        R(0, 1) = 0;
        R(0, 2) = 0;
        R(0, 3) = 0;
        R(1, 0) = 0;
        R(1, 1) = cp;
        R(1, 2) = sp;
        R(1, 3) = 0;
        R(2, 0) = 0;
        R(2, 1) = -sp;
        R(2, 2) = cp;
        R(2, 3) = 0;
        R(3, 0) = 0;
        R(3, 1) = 0;
        R(3, 2) = 0;
        R(3, 3) = 1;

        return R;
    }

    export Mat<float, 4, 4> scale(float scale_size)
    {

        Mat<float, 4, 4> R{};

        R(0, 0) = scale_size;
        R(0, 1) = 0;
        R(0, 2) = 0;
        R(0, 3) = 0;
        R(1, 0) = 0;
        R(1, 1) = scale_size;
        R(1, 2) = 0;
        R(1, 3) = 0;
        R(2, 0) = 0;
        R(2, 1) = 0;
        R(2, 2) = scale_size;
        R(2, 3) = 0;
        R(3, 0) = 0;
        R(3, 1) = 0;
        R(3, 2) = 0;
        R(3, 3) = 1;

        return R;
    }

} // namespace sopho
