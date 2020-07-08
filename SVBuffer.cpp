#include "internal_context.h"
#include "SVBuffer.h"
#include "Context.h"

namespace VkInline
{
	SVBuffer::SVBuffer(const char* elem_type, size_t size, void* hdata)
	{
		m_elem_type = elem_type;
		m_elem_size = SizeOf(elem_type);
		m_size = size;

		unsigned alignment = 4;
		if (m_elem_size % 8 == 0) alignment = 8;
		if (m_elem_size % 16 == 0) alignment = 16;

		char line[1024];
		sprintf(line, "layout(buffer_reference, scalar, buffer_reference_align = %u) buffer Buf_#hash#\n", alignment);
		std::string code = std::string(line) +
			"{\n    " + elem_type +
			" v;\n"
			"};\n";

		m_name_view_type = std::string("Buf_") + Add_Dynamic_Code(code.c_str());
		VkBufferUsageFlags usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
#ifdef _VkInlineEX
		usage |= VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR;
#endif
		m_data = new Internal::DeviceBuffer(m_elem_size*m_size, usage);
		if (hdata != nullptr)
			m_data->upload(hdata);
		else
			m_data->zero();
	}

	SVBuffer::~SVBuffer()
	{
		delete m_data;
	}

	void SVBuffer::from_host(void* hdata)
	{
		if (m_size > 0)
			m_data->upload(hdata);
	}

	void SVBuffer::to_host(void* hdata, size_t begin, size_t end) const
	{
		if (end == (size_t)(-1) || end > m_size) end = m_size;
		size_t n = end - begin;
		if (n > 0)
			m_data->download(hdata, begin*m_elem_size, end*m_elem_size);
	}

	ViewBuf SVBuffer::view() const
	{
		ViewBuf buf(sizeof(VkDeviceAddress));
		VkDeviceAddress* pview = (VkDeviceAddress*)buf.data();
		*pview = m_data->get_device_address();
		return buf;
	}

	void SVBuffer::apply_barriers(const Internal::CommandBuffer& cmdbuf, unsigned dstFlags) const
	{
		m_data->apply_barrier(cmdbuf, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, dstFlags);
	}
}

