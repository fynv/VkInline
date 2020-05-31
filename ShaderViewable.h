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

}

