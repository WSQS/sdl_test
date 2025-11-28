// sdl_raii.bound_raii.ixx
// Created by sophomore on 11/28/25.
//

export module sdl_raii:bound_raii;
import :bound_traits;
namespace sopho
{
    export template <typename T>
    struct BoundRaii
    {
        using Traits = BoundTraits<T>;
        using Device = typename Traits::Device;

    private:
        Device* m_gpu_device{};
        T* m_raw{};

    public:
        BoundRaii() noexcept = default;

        explicit BoundRaii(Device* device, T* resource) noexcept : m_gpu_device(device), m_raw(resource) {}

        BoundRaii(const BoundRaii&) = delete;
        BoundRaii& operator=(const BoundRaii&) = delete;

        BoundRaii(BoundRaii&& other) noexcept : m_gpu_device(other.m_gpu_device), m_raw(other.m_raw)
        {
            other.m_gpu_device = nullptr;
            other.m_raw = nullptr;
        }

        BoundRaii& operator=(BoundRaii&& other) noexcept
        {
            if (this != &other)
            {
                reset();

                m_gpu_device = other.m_gpu_device;
                m_raw = other.m_raw;

                other.m_gpu_device = nullptr;
                other.m_raw = nullptr;
            }
            return *this;
        }

        ~BoundRaii() noexcept { reset(); }

        void reset(T* resource = nullptr, Device* device = nullptr) noexcept
        {
            if (m_raw && m_gpu_device)
            {
                Traits::release(m_gpu_device, m_raw);
            }

            m_gpu_device = device;
            m_raw = resource;
        }

        [[nodiscard]] T* raw() const noexcept { return m_raw; }
        [[nodiscard]] bool valid() const noexcept { return m_gpu_device != nullptr && m_raw != nullptr; }
        [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    };
} // namespace sopho
