#include <memory.h>
#include "internal_context.h"
#include "SVObjBuffer.h"

namespace VkInline
{
	SVObjBuffer::SVObjBuffer(const std::vector<const ShaderViewable*>& elems)
	{
		if (elems.size() < 1)
		{
			printf("SVObjBuffer: cannot create with empty input.\n");
			exit(0);
		}
		m_elem_type = elems[0]->name_view_type();
		for (size_t i=1; i<elems.size(); i++)
			if (elems[i]->name_view_type() != m_elem_type)
			{
				printf("SVObjBuffer: input elements must be the same type.\n");
				exit(0);
			}
		m_elems = elems;
		m_elem_size = SizeOf(m_elem_type.c_str());
		m_size = elems.size();

		unsigned alignment = 4;
		if (m_elem_size % 8 == 0) alignment = 8;
		if (m_elem_size % 16 == 0) alignment = 16; char line[1024];

		sprintf(line, "layout(buffer_reference, scalar, buffer_reference_align = %u) buffer Buf_#hash#\n", alignment);
		std::string code = std::string(line) +
			"{\n    " + m_elem_type +
			" v;\n"
			"};\n";

		m_name_view_type = std::string("Buf_") + Add_Dynamic_Code(code.c_str());
		m_data = new Internal::UploadBuffer(m_elem_size*m_size, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

		update();
	}

	SVObjBuffer::~SVObjBuffer()
	{
		delete m_data;
	}

	void SVObjBuffer::update()
	{
		ViewBuf whole(m_elem_size*m_size);
		for (size_t i = 0; i < m_elems.size(); i++)
		{
			ViewBuf elem_view = m_elems[i]->view();
			memcpy(whole.data() + m_elem_size * i, elem_view.data(), elem_view.size());
		}
		m_data->upload(whole.data());
	}

	ViewBuf SVObjBuffer::view() const
	{
		ViewBuf buf(sizeof(VkDeviceAddress));
		VkDeviceAddress* pview = (VkDeviceAddress*)buf.data();
		*pview = m_data->get_device_address();
		return buf;		
	}

	void SVObjBuffer::apply_barriers(const Internal::CommandBuffer& cmdbuf, unsigned dstFlags) const
	{
		for (size_t i = 0; i < m_elems.size(); i++)
		{
			m_elems[i]->apply_barriers(cmdbuf, dstFlags);
		}

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
