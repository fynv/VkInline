import VkInline as vki
import numpy as np
from PIL import Image

image_in =  np.array(Image.open('fei.png').convert('RGBA'))

VK_FORMAT_R8G8B8A8_SRGB = 43

width = image_in.shape[1]
height =  image_in.shape[0]

tex2d = vki.Texture2D(width, height, VK_FORMAT_R8G8B8A8_SRGB)
tex2d.upload(image_in)

colorBuf = vki.Texture2D(width, height, VK_FORMAT_R8G8B8A8_SRGB)

positions = np.array([ [0.0, -0.5, 0.5], [0.5, 0.5, 0.5], [-0.5, 0.5, 0.5] ], dtype = np.float32)
gpuPos = vki.device_vector_from_numpy(positions)

colors =  np.array([ [0.0, 1.0, 0.0], [1.0, 0.0, 0.0], [0.0, 0.0, 1.0]], dtype = np.float32)
gpuColors = vki.device_vector_from_numpy(colors)

rp = vki.Rasterizer(['pos', 'col'])

rp.add_draw_call(vki.DrawCall(
'''
layout (location = 0) out vec2 vUV;
void main() 
{
	vec2 grid = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	vec2 vpos = grid * vec2(2.0f, 2.0f) + vec2(-1.0f, -1.0f);
	gl_Position = vec4(vpos, 1.0f, 1.0f);
	vUV = grid;
}
''',
'''
layout (location = 0) in vec2 vUV;
layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = texture(arr_tex2d[0], vUV);
}
'''))

rp.add_draw_call(vki.DrawCall(
'''
layout (location = 0) out vec3 vColor;
void main() 
{
	gl_Position = vec4(get_value(pos, gl_VertexIndex), 1.0);
	vColor = get_value(col, gl_VertexIndex);
}
''',
'''
layout (location = 0) in vec3 vColor;
layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = vec4(vColor, 1.0);
}
'''))


rp.launch([3, 3], [colorBuf], None, [0.5, 0.5, 0.5, 1.0], 1.0, [gpuPos, gpuColors], [tex2d])


image_out = np.empty((height, width, 4), dtype=np.uint8)
colorBuf.download(image_out)

Image.fromarray(image_out, 'RGBA').save('output.png')

