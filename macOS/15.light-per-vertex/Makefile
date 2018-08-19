.phony : build rebuild clean

.PHONY: clean
clean:
	rm -rf Window.app

.PHONY: lib
libs:
	Clang -dynamiclib -o libsphere.dylib sphere_routines.mm

rebuild: clean libs
	mkdir -p Window.app/Contents/MacOS
	Clang -o Window.app/Contents/MacOS/window *.mm -framework Cocoa  -framework QuartzCore -framework OpenGL libsphere.dylib 

build: libs
	Clang -o Window.app/Contents/MacOS/window *.mm -framework Cocoa -framework QuartzCore -framework OpenGL libsphere.dylib
	
