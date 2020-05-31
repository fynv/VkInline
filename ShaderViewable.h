#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory.h>

namespace VkInline
{
	namespace Internal
	{
		struct CommandBuffer;
	}

	typedef std::vector<char> ViewBuf;

	// root class of all shader-viewable objects
	class ShaderViewable
	{
	public:
		ShaderViewable() {}
		virtual ~ShaderViewable() {}
		virtual ViewBuf view() const = 0;
		const std::string& name_view_type() const { return m_name_view_type; }
		virtual void apply_barriers(const Internal::CommandBuffer& cmdbuf, unsigned dstFlags) const {}

	protected:
		std::string m_name_view_type;
	};

	struct CapturedShaderViewable
	{
		const char* obj_name;
		const ShaderViewable* obj;
	};

	class SomeShaderViewable : public ShaderViewable
	{
	public:
		SomeShaderViewable(const char* name_view_type, const void* data_view = "", size_t size_view = 1)
		{
			m_name_view_type = name_view_type;
			m_view_buf.resize(size_view);
			memcpy(m_view_buf.data(), data_view, size_view);
		}

		virtual ViewBuf view() const
		{
			return m_view_buf;
		}

	private:
		ViewBuf m_view_buf;
	};

#define DECLAR_SV_BASIC(clsname, type_host, type_dev)\
class clsname : public SomeShaderViewable\
{\
public:\
	clsname(type_host in) : SomeShaderViewable(#type_dev, &in, sizeof(type_host)) {}\
};
	DECLAR_SV_BASIC(SVInt32, int32_t, int)
	DECLAR_SV_BASIC(SVUInt32, uint32_t, uint)
	DECLAR_SV_BASIC(SVFloat, float, float)
	DECLAR_SV_BASIC(SVDouble, double, double)

#define DECLAR_SV_VEC(clsname, type_elem_host, type_dev, num_elem)\
class clsname : public SomeShaderViewable\
{\
public:\
	clsname(const type_elem_host* in) : SomeShaderViewable(#type_dev, in, sizeof(type_elem_host)*num_elem){}\
};
	DECLAR_SV_VEC(SVIVec2, int32_t, ivec2, 2)
	DECLAR_SV_VEC(SVIVec3, int32_t, ivec3, 3)
	DECLAR_SV_VEC(SVIVec4, int32_t, ivec4, 4)

	DECLAR_SV_VEC(SVUVec2, uint32_t, uvec2, 2)
	DECLAR_SV_VEC(SVUVec3, uint32_t, uvec3, 3)
	DECLAR_SV_VEC(SVUVec4, uint32_t, uvec4, 4)
	
	DECLAR_SV_VEC(SVVec2, float, vec2, 2)
	DECLAR_SV_VEC(SVVec3, float, vec3, 3)
	DECLAR_SV_VEC(SVVec4, float, vec4, 4)

	DECLAR_SV_VEC(SVDVec2, double, dvec2, 2)
	DECLAR_SV_VEC(SVDVec3, double, dvec3, 3)
	DECLAR_SV_VEC(SVDVec4, double, dvec4, 4)

	DECLAR_SV_VEC(SVMat2x2, float, mat2x2, 2 * 2)
	DECLAR_SV_VEC(SVMat2x3, float, mat2x3, 2 * 3)
	DECLAR_SV_VEC(SVMat2x4, float, mat2x4, 2 * 4)
	DECLAR_SV_VEC(SVMat3x2, float, mat3x2, 3 * 2)
	DECLAR_SV_VEC(SVMat3x3, float, mat3x3, 3 * 3)
	DECLAR_SV_VEC(SVMat3x4, float, mat3x4, 3 * 4)
	DECLAR_SV_VEC(SVMat4x2, float, mat4x2, 4 * 2)
	DECLAR_SV_VEC(SVMat4x3, float, mat4x3, 4 * 3)
	DECLAR_SV_VEC(SVMat4x4, float, mat4x4, 4 * 4)

	DECLAR_SV_VEC(SVDMat2x2, double, dmat2x2, 2 * 2)
	DECLAR_SV_VEC(SVDMat2x3, double, dmat2x3, 2 * 3)
	DECLAR_SV_VEC(SVDMat2x4, double, dmat2x4, 2 * 4)
	DECLAR_SV_VEC(SVDMat3x2, double, dmat3x2, 3 * 2)
	DECLAR_SV_VEC(SVDMat3x3, double, dmat3x3, 3 * 3)
	DECLAR_SV_VEC(SVDMat3x4, double, dmat3x4, 3 * 4)
	DECLAR_SV_VEC(SVDMat4x2, double, dmat4x2, 4 * 2)
	DECLAR_SV_VEC(SVDMat4x3, double, dmat4x3, 4 * 3)
	DECLAR_SV_VEC(SVDMat4x4, double, dmat4x4, 4 * 4)
}

