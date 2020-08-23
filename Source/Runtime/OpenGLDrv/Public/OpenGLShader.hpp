#pragma once
#include "RHIShader.hpp"

namespace oeng
{
inline namespace opengldrv
{
struct ShaderDeleter
{
    void operator()(unsigned id) const noexcept;
};

struct ProgramDeleter
{
    void operator()(unsigned id) const noexcept;
};

using ShaderHandle = Resource<unsigned, ShaderDeleter>;
using ProgramHandle = Resource<unsigned, ProgramDeleter>;

class OPENGLDRV_API OpenGLShader : public RHIShader
{
public:
    OpenGLShader(const char* vertex_shader, const char* frag_shader);
    void Activate() const noexcept override;
    bool SetParam(Name name, const ShaderParam& value) override;
    bool IsValidParam(Name name) const noexcept override;

private:
    [[nodiscard]] int GetUniformLocation(Name name);

    ShaderHandle vertex_shader_;
    ShaderHandle frag_shader_;
    ProgramHandle shader_program_;
    std::unordered_map<Name, int> loc_cache_;
};
}
}
