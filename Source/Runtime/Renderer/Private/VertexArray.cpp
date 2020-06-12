#include "VertexArray.hpp"
#include "OpenGL.hpp"
#include "Json.hpp"

namespace oeng
{
	void from_json(const Json& json, Vertex& vertex)
	{
		json.get_to(vertex.data);
	}

	VertexArray::VertexArray(std::span<const Vertex> verts, std::span<const Vec3u16> indices)
		:num_verts_{verts.size()}, num_indices_{indices.size()}
	{
		gl(glGenVertexArrays, 1, &vertex_array_);
		Activate();

		gl(glGenBuffers, 1, &vertex_buffer_);
		gl(glBindBuffer, GL_ARRAY_BUFFER, vertex_buffer_);
		gl(glBufferData, GL_ARRAY_BUFFER, verts.size_bytes(), verts.data(), GL_STATIC_DRAW);

		gl(glGenBuffers, 1, &index_buffer_);
		gl(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
		gl(glBufferData, GL_ELEMENT_ARRAY_BUFFER, indices.size_bytes(), indices.data(), GL_STATIC_DRAW);

		// for calculate offset
		constexpr Vertex* v = nullptr;

		static_assert(std::is_same_v<Float, float> || std::is_same_v<Float, double>);
		constexpr auto type = std::is_same_v<Float, float> ? GL_FLOAT : GL_DOUBLE;

		gl(glEnableVertexAttribArray, 0);
		gl(glVertexAttribPointer, 0, 3, type, false, sizeof Vertex, &v->pos);

		gl(glEnableVertexAttribArray, 1);
		gl(glVertexAttribPointer, 1, 3, type, false, sizeof Vertex, &v->norm);
		
		gl(glEnableVertexAttribArray, 2);
		gl(glVertexAttribPointer, 2, 2, type, false, sizeof Vertex, &v->uv);
	}

	VertexArray::VertexArray(VertexArray&& r) noexcept
		:num_verts_{r.num_verts_}, num_indices_{r.num_indices_},
		vertex_buffer_{r.vertex_buffer_}, index_buffer_{r.index_buffer_}, vertex_array_{r.vertex_array_}
	{
		r.num_verts_ = 0;
		r.num_indices_ = 0;
		r.vertex_buffer_ = 0;
		r.index_buffer_ = 0;
		r.vertex_array_ = 0;
	}

	VertexArray::~VertexArray()
	{
		unsigned err;
		// glDelete functions silently ignores 0
		gl(err, glDeleteBuffers, 1, &vertex_buffer_);
		gl(err, glDeleteBuffers, 1, &index_buffer_);
		gl(err, glDeleteVertexArrays, 1, &vertex_array_);
	}

	VertexArray& VertexArray::operator=(VertexArray&& r) noexcept
	{
		VertexArray{std::move(r)}.swap(*this);
		return *this;
	}

	void VertexArray::Activate() const
	{
		gl(glBindVertexArray, vertex_array_);
	}

	void VertexArray::swap(VertexArray& r) noexcept
	{
		using std::swap;
		swap(num_verts_, r.num_verts_);
		swap(num_indices_, r.num_indices_);
		swap(vertex_buffer_, r.vertex_buffer_);
		swap(index_buffer_, r.index_buffer_);
		swap(vertex_array_, r.vertex_array_);
	}
}
