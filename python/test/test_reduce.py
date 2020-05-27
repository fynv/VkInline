import VkInline as vki
import numpy as np

darr = vki.device_vector_from_list(range(1,1025), 'int')
BLOCK_SIZE = 256

kernel = vki.Computer(['dst', 'src', 'n'],
'''
shared {0} s_buf[{1}];
void main()
{{
	uint tid = gl_LocalInvocationID.x;
	uint i = gl_GlobalInvocationID.x;
	if (i<n) s_buf[tid] = get_value(src, i);
	barrier();
	for (uint s = {1}/2; s>0; s>>=1)
	{{
		if (tid < s && i+s<n)
    		s_buf[tid] += s_buf[tid + s];
    	barrier();
	}}
	if (tid==0) set_value(dst, gl_WorkGroupID.x, s_buf[tid]);	
}}
'''.format('int',str(BLOCK_SIZE)))

dst  = darr
while dst.size()>1:
	src = dst
	n = src.size()
	blocks = int((n + BLOCK_SIZE - 1) / BLOCK_SIZE)
	dst = vki.SVVector("int", blocks)
	kernel.launch(blocks, BLOCK_SIZE, [dst, src, vki.SVUInt32(n)])

print(dst.to_host()[0])


