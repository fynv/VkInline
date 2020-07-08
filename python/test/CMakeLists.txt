set(PYTHON_TEST
	fei.png
	SpiderTex.jpg
	spider.mtl
	spider.obj

	test_compute.py
	test_for.py
	test_reduce.py
	test_multithread.py
	test_tex2d.py
	test_rasterization.py
	test_obj.py
	test_raytrace.py
)

install(FILES ${PYTHON_TEST} DESTINATION test_python )
