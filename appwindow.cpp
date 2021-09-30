#include "appwindow.h"
#include <SDL.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/GLU.h>

#include <stdio.h>
#include <string>
#include <iostream>

#define WINDOW_WIDTH_MIN 320
#define WINDOW_HEIGHT_MIN 240
#define WINDOW_WIDTH_DEFAULT 1280
#define WINDOW_HEIGHT_DEFAULT 960

SDLGLWindow::SDLGLWindow()
{
}

SDLGLWindow::~SDLGLWindow()
{
}

SDL_Window * SDLGLWindow::GetSDLWindow()
{
	return m_window;
}

SDL_GLContext * SDLGLWindow::GetGLContext()
{
	return &m_glcontext;
}

SDL_Surface * SDLGLWindow::GetWindowSurface()
{
	return m_glsurface;
}

void SDLGLWindow::VerifyDimensions()
{
	if (m_width < WINDOW_WIDTH_MIN)
	{
		m_width = WINDOW_WIDTH_MIN;
	}

	if (m_height < WINDOW_HEIGHT_MIN)
	{
		m_height = WINDOW_WIDTH_MIN;
	}
}

bool SDLGLWindow::Initialize(const char* title, int width, int height, bool use_vsync)
{
	m_width = width == 0 ? WINDOW_WIDTH_DEFAULT : width;
	m_height = height == 0 ? WINDOW_HEIGHT_DEFAULT : height;
	VerifyDimensions();

	bool success{ true };

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
		success = false;
	}
	else
	{
		//Use OpenGL 3.1 core
		const char* glsl_version = "#version 150";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

		// window flags
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		SDL_WindowFlags window_flags = (SDL_WindowFlags)(
			SDL_WINDOW_OPENGL |
			SDL_WINDOW_RESIZABLE |
			SDL_WINDOW_ALLOW_HIGHDPI |
			SDL_WINDOW_SHOWN);

		// Create window
		m_window = SDL_CreateWindow(
			title,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			m_width,
			m_height,
			window_flags);

		if (m_window == NULL)
		{
			std::cout << "SDL window could not be created! SDL Error: " << SDL_GetError() << std::endl;
			success = false;
		}
		else
		{
			// Create the GL context
			
			m_glcontext = SDL_GL_CreateContext(m_window);
			if (m_glcontext == NULL)
			{
				std::cout << "SDL GL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
				success = false;
			}
			else
			{
				// Init GLEW
				glewExperimental = GL_TRUE;
				GLenum glewError = glewInit();
				if (glewError != GLEW_OK)
				{
					std::cout << "Error initializing GLEW: " << glewGetErrorString(glewError) << std::endl;
				}

				// get the window surface
				m_glsurface = SDL_GetWindowSurface(m_window);
				if (m_glsurface == NULL)
				{
					printf("Surface could not be created! SDL_Error: %s\n", SDL_GetError());
					success = false;
				}

				// set the current window to active
				SDL_GL_MakeCurrent(m_window, m_glcontext);

				// enable vsync
				if (SDL_GL_SetSwapInterval(use_vsync ? 1 : 0) < 0)
				{
					std::cout << "Warning: Unable to set VSYnc! SDL Error: " << SDL_GetError() << std::endl;
					success = false;
				}
			}
		}
	}

	return success;
}

void SDLGLWindow::Shutdown()
{
	SDL_GL_DeleteContext(m_glcontext);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}
