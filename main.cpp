
#include "chip8app.h"
#undef main

#define _WINDOW_WIDTH 1536
#define _WINDOW_HEIGHT 768

int main(int argc, char *argv[])
{
	// initialize app
	Chip8App* app = new Chip8App();

	app->SetInitialWindowSize(_WINDOW_WIDTH, _WINDOW_HEIGHT);
	app->Initialize();

	// run (contains main loop)
	app->Run();

	// cleeanup and exit
	app->Shutdown();

	return 0;
};
