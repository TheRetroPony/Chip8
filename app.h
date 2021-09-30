#pragma once
#include "appwindow.h"

class AppCore
{
	static constexpr const char * s_appname = "AppCore";

public:
	AppCore();
	~AppCore();

	virtual bool InitializeSDLWindow();

	virtual bool Initialize();
	virtual void Shutdown();
	virtual void Run();

	virtual const char * GetAppName();
	SDLGLWindow* GetAppWindow();
	SDL_Window* GetSDLWindow();
	SDL_GLContext* GetGLContext();
	SDL_Surface* GetWindowSurface();

	void SetInitialWindowSize(int width, int height);

protected:

	int m_initial_wnd_width;
	int m_initial_wnd_height;
	
	SDLGLWindow* m_appwindow;

private:
	
};