#include "Shader.hpp"
#include <fstream>
#include "Math.hpp"
#include "OpenGL.hpp"

namespace oeng
{
	std::string ReadFile(const std::filesystem::path& path)
	{
		std::ifstream file{ path, std::ios_base::in | std::ios_base::ate};
		if (!file.is_open()) throw std::ios_base::failure{format("Cannot read file. File not found: {}", path.string())};

		std::string code(file.tellg(), '\0');
		file.seekg(0);
		file.read(code.data(), code.size());
		return code;
	}
	
	static void CheckShader(unsigned shader)
	{
		auto is_valid = 0;
		gl(glGetShaderiv, shader, GL_COMPILE_STATUS, &is_valid);
		if (!is_valid)
		{
			auto len = 0;
			gl(glGetShaderiv, shader, GL_INFO_LOG_LENGTH, &len);
			
			std::string log(len, '\0');
			gl(glGetShaderInfoLog, shader, len, nullptr, log.data());
			
			throw std::runtime_error{log};
		}
	}

	static void CheckProgram(unsigned program)
	{
		auto is_valid = 0;
		gl(glGetProgramiv, program, GL_LINK_STATUS, &is_valid);
		if (!is_valid)
		{
			auto len = 0;
			gl(glGetProgramiv, program, GL_INFO_LOG_LENGTH, &len);
			
			std::string log(len, '\0');
			gl(glGetProgramInfoLog, program, len, nullptr, log.data());
			
			throw std::runtime_error{log};
		}
	}
	
	static unsigned Compile(const std::filesystem::path& file, unsigned type)
	{
		const auto code = ReadFile(file);
		const auto* const c_str = code.c_str();
		const auto shader = gl(glCreateShader, type);
		gl(glShaderSource, shader, 1, &c_str, nullptr);
		gl(glCompileShader, shader);
		CheckShader(shader);
		return shader;
	}

	static auto Ext(std::filesystem::path path, const char* ext)
	{
		return path += ext;
	}

	Shader::Shader(Path path)
		:path_{path},
		vert_shader_{Compile(Ext(path, ".vert"), GL_VERTEX_SHADER)},
		frag_shader_{Compile(Ext(path, ".frag"), GL_FRAGMENT_SHADER)},
		shader_program_{gl(glCreateProgram)}
	{
		gl(glAttachShader, shader_program_, vert_shader_);
		gl(glAttachShader, shader_program_, frag_shader_);
		gl(glLinkProgram, shader_program_);
		CheckProgram(shader_program_);
		Activate();
	}

	Shader::Shader(Shader&& r) noexcept
		:vert_shader_{r.vert_shader_}, frag_shader_{r.frag_shader_}, shader_program_{r.shader_program_},
		uniform_{std::move(r.uniform_)}
	{
		r.vert_shader_ = 0;
		r.frag_shader_ = 0;
		r.shader_program_ = 0;
	}

	Shader& Shader::operator=(Shader&& r) noexcept
	{
		this->~Shader();
		new (this) Shader{std::move(r)};
		return *this;
	}

	Shader::~Shader()
	{
		unsigned err;
		// glDelete functions silently ignores 0.
		gl(err, glDeleteProgram, shader_program_);
		gl(err, glDeleteShader, vert_shader_);
		gl(err, glDeleteShader, frag_shader_);
	}

	void Shader::Activate() const
	{
		gl(glUseProgram, shader_program_);
	}

	void Shader::SetUniform(int location, const Mat4& matrix)
	{
		gl(glUniformMatrix4fv, location, 1, true, matrix.AsFlatArr());
	}

	void Shader::SetUniform(int location, const Vec4& vector)
	{
		gl(glUniform4fv, location, 1, vector.data);
	}

	void Shader::SetUniform(int location, const Vec3& vector)
	{
		gl(glUniform3fv, location, 1, vector.data);
	}

	void Shader::SetUniform(int location, const Vec2& vector)
	{
		gl(glUniform2fv, location, 1, vector.data);
	}

	void Shader::SetUniform(int location, float value)
	{
		gl(glUniform1f, location, value);
	}

	int Shader::GetUniformLocation(Name name) const noexcept
	{
		if (const auto found = uniform_.find(name); found != uniform_.end())
			return found->second;

		unsigned err;
		const auto loc = gl(err, glGetUniformLocation, shader_program_, name->c_str());
		if (loc != invalid_uniform_) uniform_.try_emplace(name, loc);
		return loc;
	}

	void Shader::InvalidUniform(Name name) const
	{
		log::Error("Attempted to set invalid uniform {} for shader {}", *name, GetPath()->string());
	}

}
