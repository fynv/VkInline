import VkInline as vki
import numpy as np

# interface with numpy
harr = np.array([1.0, 2.0, 3.0, 4.0, 5.0], dtype='float32')
darr = vki.device_vector_from_numpy(harr)
print(darr.to_host())

# GLSL data type
print(darr.name_view_type())

harr2 = np.array([6,7,8,9,10], dtype='int32')
darr2 = vki.device_vector_from_numpy(harr2)

# kernel with auto parameters, launched twice with different types
kernel = vki.Computer(['arr_in', 'arr_out', 'k'],
'''
void main()
{
    uint id = gl_GlobalInvocationID.x;
    if (id >= get_size(arr_in)) return;
    set_value(arr_out, id, get_value(arr_in, id)*k);
}
''')

darr_out = vki.SVVector('float', 5)
kernel.launch(1,128, [darr, darr_out, vki.SVFloat(10.0)])
print (darr_out.to_host())

darr_out = vki.SVVector('int', 5)
kernel.launch(1,128, [darr2, darr_out, vki.SVInt32(5)])
print (darr_out.to_host())

# create a vector from python list with GLSL type specified
darr3 = vki.device_vector_from_list([3.0, 5.0, 7.0, 9.0 , 11.0], 'float')
print(darr3.to_host())
