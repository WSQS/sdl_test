// sdl_raii.pure_raii.ixx
// Created by sophomore on 11/29/25.
//

export module sdl_raii:pure_raii;
import :pure_traits;
namespace sopho
{
    export template <typename T>
    struct PureRaii
    {
        using Traits = PureTraits<T>;

    private:
        T* m_raw{};

    public:
        PureRaii() noexcept = default;

        explicit PureRaii(T* raw) noexcept : m_raw(raw) {}

        PureRaii(const PureRaii&) = delete;
        PureRaii& operator=(const PureRaii&) = delete;

        PureRaii(PureRaii&& other) noexcept : m_raw(other.m_raw) { other.m_raw = nullptr; }
        PureRaii& operator=(PureRaii&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                m_raw = other.m_raw;
                other.m_raw = nullptr;
            }
            return *this;
        }

        ~PureRaii() noexcept { reset(); }

        void reset(T* raw = nullptr) noexcept
        {
            if (m_raw)
            {
                Traits::release(m_raw);
            }

            m_raw = raw;
        }

        [[nodiscard]] T* raw() const noexcept { return m_raw; }
        [[nodiscard]] bool valid() const noexcept { return m_raw != nullptr; }
        [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    };
} // namespace sopho
