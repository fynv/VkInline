#include <memory.h>
#include "SVCombine.h"
#include "Context.h"

namespace VkInline
{
	SVCombine::SVCombine(const std::vector<CapturedShaderViewable>& elem_map, const char* operations)
	{
		std::string dynamic_code=
			"struct Comb_#hash#\n"
			"{\n";

		m_components.resize(elem_map.size());
		for (size_t i = 0; i < elem_map.size(); i++)
		{
			dynamic_code += std::string("    ") + elem_map[i].obj->name_view_type() + " " + elem_map[i].obj_name + ";\n";
			m_components[i] = elem_map[i].obj;
		}
		dynamic_code += "};\n";	
		dynamic_code += operations;

		m_name_view_type = std::string("Comb_")+Add_Dynamic_Code(dynamic_code.c_str());
		m_offsets.resize(elem_map.size() + 1);
		QueryStruct(m_name_view_type.c_str(), m_offsets.data());
	}

	ViewBuf SVCombine::view() const
	{
		ViewBuf ret(m_offsets[m_components.size()]);
		for (size_t i = 0; i < m_components.size(); i++)
		{
			ViewBuf elem_view = m_components[i]->view();
			memcpy(ret.data() + m_offsets[i], elem_view.data(), elem_view.size());
		}
		return ret;
	}

	void SVCombine::apply_barriers(const Internal::CommandBuffer& cmdbuf, unsigned dstFlags) const
	{
		for (size_t i = 0; i < m_components.size(); i++)
		{
			m_components[i]->apply_barriers(cmdbuf, dstFlags);
		}
	}

}
