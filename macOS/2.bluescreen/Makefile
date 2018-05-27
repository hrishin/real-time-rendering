.phony : build rebuild clean

clean:
	rm -rf Window.app

rebuild: clean
	mkdir -p Window.app/Contents/MacOS
	Clang -o Window.app/Contents/MacOS/window *.m -framework Cocoa  -framework QuartzCore -framework OpenGL

build:
	Clang -o Window.app/Contents/MacOS/window *.m -framework Cocoa -framework QuartzCore -framework OpenGL
	
