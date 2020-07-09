import VkInline as vki
import numpy as np
from PIL import Image
import glm

width = 640
height =  480

darr_out = vki.SVVector('vec3', width*height)

raytracer = vki.RayTracer(['arr_out', 'width', 'height'], 
'''
void main()
{
	int x = int(gl_LaunchIDEXT.x);
	int y = int(gl_LaunchIDEXT.y);
	if (x>=width || y>height) return;

	float r = (float(x)+0.5f)/float(width);
	float g = (float(y)+0.5f)/float(height);

	set_value(arr_out, x+y*width, vec3(r,g, 0.0));
}

''', [], [])


svwidth = vki.SVInt32(width)
svheight = vki.SVInt32(height)

raytracer.launch((width, height), [darr_out, svwidth, svheight], [])

out = darr_out.to_host()
out = out.reshape((480,640,3))*255.0
out = out.astype(np.uint8)
Image.fromarray(out, 'RGB').save('output.png')




