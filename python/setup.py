from setuptools import setup
from codecs import open

setup(
	name = 'VkInline',
	version = '0.3.5',
	description = 'A tool making it easy to use Vulkan for Python',
	url='https://github.com/fynv/VkInline',
	license='Anti 996',
	author='Fei Yang',
	author_email='hyangfeih@gmail.com',
	keywords='GPU Vulkan Python offscreen-rendering ray-tracing',
	packages=['VkInline'],
	package_data = { 'VkInline': ['*.dll', '*.so']},
	install_requires = ['cffi','numpy', 'pyglm'],	
)


