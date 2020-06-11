set(PYTHON_TEST
	fei.png
	test_compute.py
	test_for.py
	test_reduce.py
	test_multithread.py
	test_tex2d.py
)

install(FILES ${PYTHON_TEST} DESTINATION test_python )
