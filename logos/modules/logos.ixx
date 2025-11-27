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
        constexpr Mat() = default;
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

        Mat operator+(const Mat& rhs) const
        {
            Mat result{};

            for (std::uint8_t r = 0; r < Row; ++r)
            {
                for (std::uint8_t c = 0; c < Col; ++c)
                {
                    result(c, r) = (*this)(c, r) + rhs(c, r);
                }
            }

            return result;
        }
        Mat<TScalar, Row, Col> transpose()
        {
            Mat<TScalar, Row, Col> result{};
            for (std::uint8_t r = 0; r < Row; ++r)
            {
                for (std::uint8_t c = 0; c < Col; ++c)
                {
                    result(r, c) = (*this)(c, r);
                }
            }
            return result;
        }
        template <std::uint8_t NewCol, std::uint8_t NewRow>
        Mat<TScalar, NewCol, NewRow> resize()
        {
            Mat<TScalar, NewCol, NewRow> result{};
            for (std::uint8_t r = 0; r < std::min(Row, NewRow); ++r)
            {
                for (std::uint8_t c = 0; c < std::min(Col, NewCol); ++c)
                {
                    result(c, r) = (*this)(c, r);
                }
            }
            return result;
        }
        constexpr Mat(std::initializer_list<TScalar> list)
            requires(Col == 1)
        {
            assert(list.size() <= Row);
            auto it = list.begin();

            for (std::uint8_t r = 0; r < Row; ++r)
            {
                if (it != list.end())
                {
                    (*this)(r) = *it++;
                }
                else
                {
                    break;
                }
            }
        }
    };

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

    // Rodrigues' rotation formula
    // v_rot = cos(theta)*v+(1−cos(theta))*(v*k)*k+sin(theta)*(kxv)
    // (v*k)k=(k*k^T)v
    export Mat<float, 4, 4> rotation(Mat<float, 1, 3> k, float theta)
    {
        Mat k_k_t = (k * k.transpose()).resize<4, 4>();
        Mat<float, 4, 4> k_m{};
        k_m(0, 1) = k(2);
        k_m(0, 2) = -k(1);
        k_m(1, 0) = -k(2);
        k_m(1, 2) = k(0);
        k_m(2, 0) = k(1);
        k_m(2, 1) = -k(0);
        return scale(std::cos(theta)) + scale(1.F - std::cos(theta)) * k_k_t + scale(std::sin(theta)) * k_m;
    }

    export Mat<float, 4, 4> rotation_y(float yaw)
    {
        Mat<float, 1, 3> y{0, 1, 0};
        return rotation(y, yaw);
    }

    export Mat<float, 4, 4> rotation_x(float pitch)
    {
        Mat<float, 1, 3> x{1, 0, 0};
        return rotation(x, pitch);
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
