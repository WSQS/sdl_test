//
// Created by wsqsy on 11/14/2025.
//
module;
module sdl_wrapper;
import :gpu;
import :pipeline;
namespace sopho
{
    PipelineWrapper GpuWrapper::create_pipeline() { return PipelineWrapper{shared_from_this()}; }
}
