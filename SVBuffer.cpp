#include "internal_context.h"
#include "SVBuffer.h"
namespace VkInline
{
	SVBuffer::SVBuffer(const char* elem_type, size_t size, void* hdata, int streamId)
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
		m_data = new Internal::DeviceBuffer(m_elem_size*m_size, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		if (hdata != nullptr)
			m_data->upload(hdata, streamId);
		else
			m_data->zero(streamId);
	}

	SVBuffer::~SVBuffer()
	{
		delete m_data;
	}

	void SVBuffer::from_host(void* hdata, int streamId)
	{
		if (m_size > 0)
			m_data->upload(hdata, streamId);
	}

	void SVBuffer::to_host(void* hdata, size_t begin, size_t end, int streamId) const
	{
		if (end == (size_t)(-1) || end > m_size) end = m_size;
		size_t n = end - begin;
		if (n > 0)
			m_data->download(hdata, begin*m_elem_size, end*m_elem_size, streamId);
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
		VkBufferMemoryBarrier barriers[1];
		barriers[0] = {};
		barriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barriers[0].buffer = m_data->buf();
		barriers[0].offset = 0;
		barriers[0].size = VK_WHOLE_SIZE;
		barriers[0].srcAccessMask = 0;
		barriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		vkCmdPipelineBarrier(
			cmdbuf.m_buf,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			dstFlags,
			0,
			0, nullptr,
			1, barriers,
			0, nullptr
		);
	}
}

