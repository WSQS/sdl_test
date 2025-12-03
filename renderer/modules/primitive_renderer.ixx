// primitive_renderer.ixx
// Created by wsqsy on 12/3/2025.
//

export module primitive_renderer;

namespace sopho
{
    export class PrimitiveRenderer
    {
    public:
        virtual ~PrimitiveRenderer() = default;
        virtual void begin_frame() = 0;
        virtual void end_frame() = 0;
    };
} // namespace sopho
