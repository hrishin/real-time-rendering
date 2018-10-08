build:
	rm -rf Window.app/Contents/MacOS
	mkdir -p Window.app/Contents/MacOS
	Clang -o Window.app/Contents/MacOS/window *.m -framework Cocoa

rebuild:
	Clang -o Window.app/Contents/MacOS/window *.m -framework Cocoa
	
