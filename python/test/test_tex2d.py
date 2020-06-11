import VkInline as vki
import numpy as np
from PIL import Image

image_in =  np.array(Image.open('fei.png').convert('RGBA'))
# print(image_in.shape, image_in.dtype)

VK_FORMAT_R8G8B8A8_SRGB = 43

width = image_in.shape[1]
height =  image_in.shape[0]

tex2d = vki.Texture2D(width, height, VK_FORMAT_R8G8B8A8_SRGB)
tex2d.upload(image_in)

darr = vki.SVVector('vec4', width*height)

kernel = vki.Computer(['width', 'height', 'arr'],
'''
void main()
{
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;
    if (x >= width || y>=height) return;

    float u = (float(x)+0.5)/float(width);
    float v = (float(y)+0.5)/float(height);

    vec4 rgba = texture(arr_tex2d[0], vec2(u,v));

    set_value(arr, x+y*width, rgba);
}
''')

blockSize = (8,8)
gridSize = (int((width+7)/8), int((height+7)/8))

kernel.launch(gridSize, blockSize, [vki.SVInt32(width), vki.SVInt32(height), darr], tex2ds=[tex2d])

harr = darr.to_host()
harr = np.reshape(harr, (height, width, 4))*255.0
harr = harr.astype(np.uint8)

img_out = Image.fromarray(harr, 'RGBA')
img_out.save('output.png')


