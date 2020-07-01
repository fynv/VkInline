#pragma once

#include "ShaderViewable.h"

namespace VkInline
{
	namespace Internal
	{
		class DeviceBuffer;
	}

	class SVBuffer : public ShaderViewable
	{
	public:
		const std::string& name_elem_type() const { return m_elem_type; }
		size_t elem_size() const { return m_elem_size; }
		size_t size() const { return m_size; }
		Internal::DeviceBuffer* internal() { return m_data; }
		const Internal::DeviceBuffer* internal() const { return m_data; }

		SVBuffer(const char* elem_type, size_t size, void* hdata = nullptr);
		~SVBuffer();

		void from_host(void* hdata);
		void to_host(void* hdata, size_t begin = 0, size_t end = (size_t)(-1)) const;
		virtual ViewBuf view() const;
		virtual void apply_barriers(const Internal::CommandBuffer& cmdbuf, unsigned dstFlags) const;

	protected:
		std::string m_elem_type;
		size_t m_elem_size;
		size_t m_size;
		Internal::DeviceBuffer* m_data;

	};

}
