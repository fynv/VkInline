import VkInline as vki
import numpy as np
from PIL import Image
import glm

width = 800
height =  400

aabb_unit_sphere = np.array([-1.0, -1.0, -1.0, 1.0, 1.0, 1.0], dtype = np.float32)
d_aabb_unit_sphere = vki.device_vector_from_numpy(aabb_unit_sphere)
blas_unit_sphere = vki.BaseLevelAS(gpuAABB = d_aabb_unit_sphere)
transform = glm.identity(glm.mat4)
transform = glm.translate(transform, glm.vec3(0.0, 0.0, -1.0))
transform = glm.scale(transform, glm.vec3(0.5, 0.5, 0.5))
tlas = vki.TopLevelAS([[(blas_unit_sphere, transform)]])

darr_out = vki.SVVector('vec3', width*height)

raytracer = vki.RayTracer(['arr_out', 'width', 'height'], 
'''
struct Payload
{
	float t;
	vec3 color;
};

layout(location = 0) rayPayloadEXT Payload payload;

void main()
{
	int x = int(gl_LaunchIDEXT.x);
	int y = int(gl_LaunchIDEXT.y);
	if (x>=width || y>height) return;

	vec3 lower_left_corner = vec3(-2.0, -1.0, -1.0);
	vec3 horizontal = vec3(4.0, 0.0, 0.0);
	vec3 vertical = vec3(0.0, 2.0, 0.0);
	vec3 origin = vec3(0.0, 0.0, 0.0);

	float u = (float(x)+0.5)/float(width);
	float v = 1.0 - (float(y)+0.5)/float(height);

	vec3 direction = normalize(lower_left_corner + u * horizontal + v * vertical);

	uint rayFlags = gl_RayFlagsOpaqueEXT;
	uint cullMask = 0xff;
	float tmin = 0.001;
    float tmax = 1000000.0;

	traceRayEXT(arr_tlas[0], rayFlags, cullMask, 0, 0, 0, origin, tmin, direction, tmax, 0);

	set_value(arr_out, x+y*width, payload.color);
}

''', [
'''
struct Payload
{
	float t;
	vec3 color;
};

layout(location = 0) rayPayloadInEXT Payload payload;

void main()
{
	payload.t = -1.0;
	vec3 direction = gl_WorldRayDirectionEXT;
	float t = 0.5 * (direction.y + 1.0);
	payload.color = (1.0 - t)*vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);	
}
'''], [vki.HitShaders(
closest_hit = '''
struct Payload
{
	float t;
	vec3 color;
};

layout(location = 0) rayPayloadInEXT Payload payload;
hitAttributeEXT vec3 hitpoint;

void main()
{
	vec3 normal = normalize(hitpoint);
	payload.t = gl_HitTEXT;
	payload.color = (normal+vec3(1.0, 1.0, 1.0))*0.5;
}

''',
intersection = '''
hitAttributeEXT vec3 hitpoint;

void main()
{
	vec3 origin = gl_ObjectRayOriginEXT;
	vec3 direction = gl_ObjectRayDirectionEXT;
	float tMin = gl_RayTminEXT;
	float tMax = gl_RayTmaxEXT;

	const float a = dot(direction, direction);
	const float b = dot(origin, direction);
	const float c = dot(origin, origin) - 1.0;
	const float discriminant = b * b - a * c;

	if (discriminant >= 0)
	{
		const float t1 = (-b - sqrt(discriminant)) / a;
		const float t2 = (-b + sqrt(discriminant)) / a;

		if ((tMin <= t1 && t1 < tMax) || (tMin <= t2 && t2 < tMax))
		{
			float t = t1;
			if (tMin <= t1 && t1 < tMax)
			{
				hitpoint = origin + direction * t1;
			}
			else
			{
				t = t2;
				hitpoint = origin + direction * t2;
			}
			reportIntersectionEXT(t, 0);
		}
	}

}
'''
)])

svwidth = vki.SVInt32(width)
svheight = vki.SVInt32(height)

raytracer.launch((width, height), [darr_out, svwidth, svheight], [tlas])

out = darr_out.to_host()
out = out.reshape((height,width,3))*255.0
out = out.astype(np.uint8)
Image.fromarray(out, 'RGB').save('output.png')




