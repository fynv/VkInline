import VkInline as vki
import numpy as np
import threading


harr = np.array([1.0, 2.0, 3.0, 4.0, 5.0], dtype='float32')
darr = vki.device_vector_from_numpy(harr)

forLoop = vki.For(['arr_in','arr_out','k'], "inner",
'''
void inner(uint idx)
{
    set_value(arr_out, idx, get_value(arr_in, idx)*k);
}
''')

def thread_func():
    darr_out = vki.SVVector('float', 5)
    forLoop.launch_n(5, [darr, darr_out, vki.SVFloat(10.0)])
    print (darr_out.to_host())

a = threading.Thread(target = thread_func)
b = threading.Thread(target = thread_func)
c = threading.Thread(target = thread_func)

a.start()
b.start()
c.start()
c.join()
b.join()
a.join()


