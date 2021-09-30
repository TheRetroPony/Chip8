#include "app.h"

AppCore::AppCore()
{
	m_initial_wnd_height = 1024;
	m_initial_wnd_width = 1024;
}

AppCore::~AppCore()
{
}

bool AppCore::InitializeSDLWindow()
{
	bool success = true;
	m_appwindow = new SDLGLWindow();
	if (m_appwindow)
	{
		if (m_initial_wnd_width != 0 && m_initial_wnd_height != 0)
		{
			success = m_appwindow->Initialize(GetAppName(), m_initial_wnd_width, m_initial_wnd_height);
		}
		else
		{
			success = m_appwindow->Initialize(GetAppName());
		}
	}

	return success;
}

bool AppCore::Initialize()
{
	return InitializeSDLWindow();
}

void AppCore::Shutdown()
{
	if (m_appwindow)
	{
		m_appwindow->Shutdown();
		m_appwindow = 0;
	}
}

void AppCore::Run()
{
	SDL_Delay(2000);
}

const char * AppCore::GetAppName()
{
	return s_appname;
}

SDLGLWindow * AppCore::GetAppWindow()
{
	return m_appwindow;
}

SDL_Window * AppCore::GetSDLWindow()
{
	if (m_appwindow)
	{
		return m_appwindow->GetSDLWindow();
	}
	return nullptr;
}

SDL_GLContext * AppCore::GetGLContext()
{
	if (m_appwindow)
	{
		return m_appwindow->GetGLContext();
	}
	return nullptr;
}

SDL_Surface * AppCore::GetWindowSurface()
{
	if (m_appwindow)
	{
		return m_appwindow->GetWindowSurface();
	}
	return nullptr;
}

void AppCore::SetInitialWindowSize(int width, int height)
{
	m_initial_wnd_width = width;
	m_initial_wnd_height = height;
}
