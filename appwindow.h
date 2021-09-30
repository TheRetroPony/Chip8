#pragma once
#include <SDL.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/GLU.h>

class SDLGLWindow
{
public:
	SDLGLWindow();
	~SDLGLWindow();

	bool Initialize(const char* title, int width = 0, int height = 0, bool use_vsync = true);
	void Shutdown();

	SDL_Window* GetSDLWindow();
	SDL_GLContext* GetGLContext();
	SDL_Surface* GetWindowSurface();

private:
	void VerifyDimensions();

private:

	int m_width;
	int m_height;

	SDL_Window *	m_window;
	SDL_GLContext	m_glcontext;
	SDL_Surface *	m_glsurface;
};