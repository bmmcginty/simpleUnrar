all:
	make -f makefile.unix lib
	g++ -O3 -o_simpleUnrar.so -fPIC -shared -L . simpleUnrar.c -lunrar -Wl,-rpath=.

