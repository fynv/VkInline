#include "Context.h"
#include "SVBuffer.h"
#include "SVCombine.h"
using namespace VkInline;

class SVVector : public SVCombine
{
public:
	const std::string& name_elem_type() const 
	{ 
		return ((SVBuffer*)m_components[1])->name_elem_type();
	}
	size_t elem_size() const 
	{ 
		return ((SVBuffer*)m_components[1])->elem_size();
	}
	size_t size() const 
	{ 
		return ((SVBuffer*)m_components[1])->size();
	}

	SVVector(const char* elem_type, size_t size, void* hdata = nullptr)
		:SVCombine({
			{"size", new SVUInt32((unsigned)size)},
			{"data", new SVBuffer(elem_type, size, hdata)} },
			(std::string("") +
			"uint get_size(in Comb_#hash# vec)\n"
			"{\n"
			"    return vec.size;\n"
			"}\n" +
			elem_type + " get_value(in Comb_#hash# vec, in uint id)\n"
			"{\n"
			"    return vec.data[id].v;\n"
			"}\n"
			"void set_value(in Comb_#hash# vec, in uint id, in " + elem_type + " value)\n"
			"{\n"
			"    vec.data[id].v = value;\n"
			"}\n").c_str()){}

	~SVVector()
	{
		delete m_components[0];
		delete m_components[1];
	}

	void to_host(void* hdata, size_t begin = 0, size_t end = (size_t)(-1)) const
	{
		size_t _size = size();
		if (end == (size_t)(-1) || end > _size) end = _size;
		((SVBuffer*)m_components[1])->to_host(hdata, begin, end);
	}

};

int main()
{
	Computer ker(
		{ "arr_in", "arr_out", "k" },
		"void main()\n"
		"{\n"
		"    uint id = gl_GlobalInvocationID.x;\n"
		"    if (id >= get_size(arr_in)) return;\n"
		"    set_value(arr_out, id, get_value(arr_in, id)*k);\n"
		"}\n"
	);

	float test_f[5] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0 };
	SVVector svbuf_in("float", 5, test_f);
	SVVector svbuf_out("float", 5);
	SVFloat k1(10.0);
	const ShaderViewable* args_f[] = { &svbuf_in, &svbuf_out, &k1 };
	ker.launch({ 1,1,1 }, { 128, 1, 1 }, args_f, {});
	svbuf_out.to_host(test_f);
	printf("%f %f %f %f %f\n", test_f[0], test_f[1], test_f[2], test_f[3], test_f[4]);



	return 0;
}

