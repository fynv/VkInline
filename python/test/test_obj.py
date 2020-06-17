import VkInline as vki
import numpy as np
import tinyobjloader
from PIL import Image
import glm

reader = tinyobjloader.ObjReader()
reader.ParseFromFile('spider.obj')
attrib = reader.GetAttrib()

positions = np.array(attrib.vertices, dtype=np.float32)
positions = np.reshape(positions, (-1, 3))

normals = np.array(attrib.normals, dtype=np.float32)
normals = np.reshape(normals, (-1, 3))

shapes = reader.GetShapes()
lst_vertex_inds = []
lst_normal_inds = []
for shape in shapes:
	for ind in shape.mesh.indices:
		lst_vertex_inds += [ind.vertex_index]
		lst_normal_inds += [ind.normal_index]

vertex_inds = np.array(lst_vertex_inds, dtype=np.int32)
normal_inds = np.array(lst_normal_inds, dtype=np.int32)

VK_FORMAT_R8G8B8A8_SRGB = 43
VK_FORMAT_D32_SFLOAT = 126

width = 640
height = 480

colorBuf = vki.Texture2D(width, height, VK_FORMAT_R8G8B8A8_SRGB)
depthBuf = vki.Texture2D(width, height, VK_FORMAT_D32_SFLOAT)
gpuPos = vki.device_vector_from_numpy(positions)
gpuNormals = vki.device_vector_from_numpy(normals)
gpuInd_pos = vki.device_vector_from_numpy(vertex_inds) 
gpuInd_norm = vki.device_vector_from_numpy(normal_inds) 


rp = vki.Rasterizer(['arr_pos', 'arr_norm', 'ind_pos', 'ind_norm', 'mat_pos', 'mat_norm'])

rp.add_draw_call(vki.DrawCall(
'''
layout (location = 0) out vec3 vNorm;
void main() 
{
	vec3 pos = get_value(arr_pos, get_value(ind_pos, gl_VertexIndex));
	vec3 norm = get_value(arr_norm, get_value(ind_norm, gl_VertexIndex));
	vec4 pos_trans = mat_pos*vec4(pos, 1.0);
	pos_trans.y = -pos_trans.y;
	gl_Position = pos_trans;
	vec4 norm_trans = mat_norm*vec4(norm, 0.0);
	vNorm = norm_trans.xyz;
}
''',
'''
layout (location = 0) in vec3 vNorm;
layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = vec4(0.0, 0.0, 0.0, 1.0);

	vec3 norm = normalize(vNorm);
	vec3 light_dir = normalize(vec3(0.2, 0.5, -1.0));
	light_dir = reflect(light_dir, norm);

	if (light_dir.z>0.0)
	{
		float intensity = pow(light_dir.z, 5.0)*0.8;
		outColor += vec4(intensity, intensity, intensity, 0.0);
	}

	outColor = clamp(outColor, 0.0, 1.0);
	
}
'''))

proj = glm.perspective(glm.radians(45.0), width/height, 0.1, 2000.0)
modelView = glm.lookAt(glm.vec3(-100.0, 200.0, 200.0), glm.vec3(0.0,0.0,0.0), glm.vec3(0.0, 1.0, 0.0))
mat = vki.SVMat4x4(proj*modelView)
mat_norm = vki.SVMat4x4(glm.transpose(glm.inverse(modelView)))

rp.launch([len(vertex_inds)], [colorBuf], depthBuf, [0.5, 0.5, 0.5, 1.0], 1.0, [gpuPos, gpuNormals, gpuInd_pos, gpuInd_norm, mat, mat_norm])

image_out = np.empty((height, width, 4), dtype=np.uint8)
colorBuf.download(image_out)
Image.fromarray(image_out, 'RGBA').save('output.png')



