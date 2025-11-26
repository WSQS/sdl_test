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
    export template <typename TScalar, std::uint8_t Col, std::uint8_t Row>
    class Mat
    {
        std::array<std::array<TScalar, Row>, Col> m_data{};

    public:
        TScalar& operator()(std::uint8_t col, std::uint8_t row) { return m_data[col][row]; }
        const TScalar& operator()(std::uint8_t col, std::uint8_t row) const { return m_data[col][row]; }
        TScalar& operator()(std::uint8_t row)
            requires(Col == 1)
        {
            return m_data[0][row];
        }
        TScalar* data() { return m_data[0].data(); }
        template <std::uint8_t OtherCol>
        Mat<TScalar, OtherCol, Row> operator*(const Mat<TScalar, OtherCol, Col>& rhs) const
        {
            Mat<TScalar, OtherCol, Row> result{};

            for (std::uint8_t r = 0; r < Row; ++r)
            {
                for (std::uint8_t c = 0; c < OtherCol; ++c)
                {
                    TScalar sum{};
                    for (std::uint8_t k = 0; k < Col; ++k)
                    {
                        sum += (*this)(k, r) * rhs(c, k);
                    }
                    result(c, r) = sum;
                }
            }

            return result;
        }
    };

    export Mat<float, 4, 4> rotation_y(float yaw)
    {
        float cy = std::cos(yaw);
        float sy = std::sin(yaw);

        Mat<float, 4, 4> R{};

        R(0, 0) = cy;
        R(0, 1) = 0;
        R(0, 2) = -sy;
        R(0, 3) = 0;
        R(1, 0) = 0;
        R(1, 1) = 1;
        R(1, 2) = 0;
        R(1, 3) = 0;
        R(2, 0) = sy;
        R(2, 1) = 0;
        R(2, 2) = cy;
        R(2, 3) = 0;
        R(3, 0) = 0;
        R(3, 1) = 0;
        R(3, 2) = 0;
        R(3, 3) = 1;

        return R;
    }

    export Mat<float, 4, 4> rotation_x(float pitch)
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
        R(1, 2) = -sp;
        R(1, 3) = 0;
        R(2, 0) = 0;
        R(2, 1) = sp;
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

    export Mat<float, 4, 4> translate(float x, float y, float z)
    {

        Mat<float, 4, 4> R{};

        R(0, 0) = 1;
        R(0, 1) = 0;
        R(0, 2) = 0;
        R(0, 3) = 0;
        R(1, 0) = 0;
        R(1, 1) = 1;
        R(1, 2) = 0;
        R(1, 3) = 0;
        R(2, 0) = 0;
        R(2, 1) = 0;
        R(2, 2) = 1;
        R(2, 3) = 0;
        R(3, 0) = x;
        R(3, 1) = y;
        R(3, 2) = z;
        R(3, 3) = 1;

        return R;
    }

    export Mat<float, 4, 4> perspective(float fovy, float aspect, float z_near, float z_far)
    {
        assert(std::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);

        const float tanHalfFovy = std::tan(fovy * 0.5f);

        Mat<float, 4, 4> P{};

        P(0, 0) = 1.0F / (aspect * tanHalfFovy);
        P(0, 1) = 0.0F;
        P(0, 2) = 0.0F;
        P(0, 3) = 0.0F;

        P(1, 0) = 0.0F;
        P(1, 1) = 1.0F / tanHalfFovy;
        P(1, 2) = 0.0F;
        P(1, 3) = 0.0F;

        P(2, 0) = 0.0F;
        P(2, 1) = 0.0F;
        P(2, 2) = -(z_far + z_near) / (z_far - z_near);
        P(2, 3) = -1.0F;

        P(3, 0) = 0.0F;
        P(3, 1) = 0.0F;
        P(3, 2) = -(2.0F * z_far * z_near) / (z_far - z_near);
        P(3, 3) = 0.0F;

        return P;
    }

} // namespace sopho
