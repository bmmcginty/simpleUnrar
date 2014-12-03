all:
	make -f makefile.unix lib
	g++ -O3 -osimpleUnrar.so -fPIC -shared -L . simpleUnrar.c -lunrar -Wl,-rpath=.

