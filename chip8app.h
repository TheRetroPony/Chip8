#pragma once
#include "appbase.h"
#include "chip8.h"
#include "imgui/imgui.h"


extern struct Chip8DebugLog;
class Chip8App : public AppBase
{
private:
	static constexpr const char * s_appname = "Chip8 App";

public:
	virtual const char * GetAppName();

	virtual bool Initialize();
	virtual void Shutdown();

private:

	Chip8 chip8;
	
	virtual void update();
	virtual void editor_update();

	virtual void render();

	virtual void render_gamemode();
	virtual void render_editmode();

	virtual void sdl_input(SDL_Event event);

private:
	bool tickOnce;
};