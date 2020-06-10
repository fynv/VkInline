# VkInline

Trying to develop another "easy way" to program GPU using a non-C++ host language.

Previously, I did [ThrustRTC](https://github.com/fynv/ThrustRTC) and [CUDAInline](https://github.com/fynv/CUDAInline),
both of which are based on CUDA (NVRTC). 

This time, however, I'm trying to do similar things basing on Vulkan. I found it more challenging than using CUDA (NVRTC) 
because of less friendly lauguage feature of GLSL comparing to CUDA C, and more complicated host API struture. 
However, I still found it attractive to do, because:

* Vulkan is neutral to GPU vendors
* Vulkan exposes more GPU features. Computing is only a small part, there are rasterization and ray-tracing pipelines.

Currently VkInline has implemented similar features as CUDAInline. You can easily launch a compute shader from Python, like:

```python

import VkInline as vki
import numpy as np

harr = np.array([1.0, 2.0, 3.0, 4.0, 5.0], dtype='float32')
darr = vki.device_vector_from_numpy(harr)

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

```

I will work on to expose the rasterization and ray-tracing functionality in a similar fashion.

## Installation

### Install from Source Code

Source code of VkInline is available at:
https://github.com/fynv/VkInline

At build time, you will need:
* UnQLite source code, as submodule: thirdparty/unqlite
* glslang, as submodule: thirdparty/glslang
* SPIRV-Cross, as submodule: thirdparty/SPIRV-Cross 
* Vulkan-Headers, as submodule: thirdparty/Vulkan-Headers
* volk, as submodule: thirdparty/volk
* CMake 3.x

After cloning the repo from github and resolving the submodules, you can build it
with CMake.

```
$ mkdir build
$ cd build
$ cmake .. -DBUILD_PYTHON_BINDINGS=true -DVKINLINE_BUILD_TESTS=true -DVKINLINE_INCLUDE_PYTESTS=true
$ make
$ make install
```
You will get the library headers, binaries and examples in the "install" directory.

### Install PyVkInline from PyPi

Builds for Win64/Linux64 + Python 3.x are available from Pypi. If your
environment matches, you can try:

```
$ pip3 install VkInline
```

## Runtime Dependencies

A Vulkan-capable GPU and a recent driver is needed at run-time.

You may also need Vulkan SDK at runtime for some platforms.

At Python side, VkInline depends on:
* Python 3
* cffi
* numpy
* pyglm

## License 

I've decided to license this project under ['"Anti 996" License'](https://github.com/996icu/996.ICU/blob/master/LICENSE)

Basically, you can use the code any way you like unless you are working for a 996 company.

[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)


