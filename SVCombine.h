#pragma once

#include "ShaderViewable.h"
#include "Context.h"

namespace VkInline
{
	class SVCombine : public ShaderViewable
	{
	public:
		SVCombine(const std::vector<CapturedShaderViewable>& elem_map, const char* operations);
		virtual ~SVCombine() {}
		virtual ViewBuf view() const;
		virtual void apply_barriers(const Internal::CommandBuffer& cmdbuf, unsigned dstFlags) const;

	protected:
		std::vector<const ShaderViewable*> m_components;
		std::vector<size_t> m_offsets;
	};
}

