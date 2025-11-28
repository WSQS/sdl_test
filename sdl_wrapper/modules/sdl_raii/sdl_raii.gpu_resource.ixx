// sdl_raii.gpu_resource.ixx
// Created by wsqsy on 11/28/2025.
//

export module sdl_raii:gpu_resource;
namespace sopho
{
    template <typename T>
    struct GpuResourceTraits;
    export template <typename T>
    struct GpuResourceRaii
    {
        using Traits = GpuResourceTraits<T>;
        using Device = typename Traits::Device;

    private:
        Device* m_gpu_device{};
        T* m_raw{};

    public:
        GpuResourceRaii() noexcept = default;

        explicit GpuResourceRaii(Device* device, T* resource) noexcept : m_gpu_device(device), m_raw(resource) {}

        GpuResourceRaii(const GpuResourceRaii&) = delete;
        GpuResourceRaii& operator=(const GpuResourceRaii&) = delete;

        GpuResourceRaii(GpuResourceRaii&& other) noexcept : m_gpu_device(other.m_gpu_device), m_raw(other.m_raw)
        {
            other.m_gpu_device = nullptr;
            other.m_raw = nullptr;
        }

        GpuResourceRaii& operator=(GpuResourceRaii&& other) noexcept
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

        ~GpuResourceRaii() noexcept { reset(); }

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
