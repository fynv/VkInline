import VkInline as vki
import numpy as np

# interface with numpy
harr = np.array([1.0, 2.0, 3.0, 4.0, 5.0], dtype='float32')
darr = vki.device_vector_from_numpy(harr)

harr2 = np.array([6,7,8,9,10], dtype='int32')
darr2 = vki.device_vector_from_numpy(harr2)

# test launching non-templated for
forLoop = vki.For(['arr_in','arr_out','k'], "inner",
'''
void inner(uint idx)
{
	set_value(arr_out, idx, get_value(arr_in, idx)*k);
}
''')

darr_out = vki.SVVector('float', 5)
forLoop.launch(0, 5, [darr, darr_out, vki.SVFloat(10.0)])
print (darr_out.to_host())

darr_out = vki.SVVector('int', 5)
forLoop.launch_n(5, [darr2, darr_out, vki.SVInt32(5)])
print (darr_out.to_host())

