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

	class BuiltIn : public ShaderViewable
	{
	public:
		BuiltIn(const char* name_view_type, const void* data_view = "", size_t size_view = 1)
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
class clsname : public BuiltIn\
{\
public:\
	clsname(type_host in) : BuiltIn(#type_dev, &in, sizeof(type_host)) {}\
};
	DECLAR_SV_BASIC(SVInt32, int32_t, int)
	DECLAR_SV_BASIC(SVUInt32, uint32_t, uint)
	DECLAR_SV_BASIC(SVFloat, float, float)
}

