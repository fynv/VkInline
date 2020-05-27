#pragma once

#include "ShaderViewable.h"
#include "Context.h"

namespace VkInline
{
	namespace Internal
	{
		class UploadBuffer;
	}

	class SVObjBuffer : public ShaderViewable
	{
	public:
		std::string name_elem_type() const { return m_elem_type; }
		size_t elem_size() const { return m_elem_size; }
		size_t size() const { return m_size; }

		SVObjBuffer(const std::vector<const ShaderViewable*>& elems);
		~SVObjBuffer();

		void update();

		virtual ViewBuf view() const;
		virtual void apply_barriers(const Internal::CommandBuffer& cmdbuf, unsigned dstFlags) const;

	protected:
		std::vector<const ShaderViewable*> m_elems;
		std::string m_elem_type;
		size_t m_elem_size;
		size_t m_size;
		Internal::UploadBuffer* m_data;

	};

}

