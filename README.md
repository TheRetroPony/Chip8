# Chip8
A Chip8 Emulator written in C/C++ 

- Building -

I didn't feel like uploading the whole Visual Studio project with a bunch of local paths etc,
I might need to figure out how makefiles work one of these days.

You'll need to create a project that links in GLEW, SDL2, and OpenGL 3 properly.
You will also need to include Dear Imgui, and the SDL2 and OpenGL3 backends, and may need to change the includes in Chip8 depending where you put the imgui files.
