#pragma once
#include "app.h"

#define DEFAULT_INTERNAL_RENDER_WIDTH 320
#define DEFAULT_INTERNAL_RENDER_HEIGHT 240

class AppBase : public AppCore
{
private:
	static constexpr const char * s_appname = "AppTestGame";

public:

	AppBase();
	~AppBase();

	virtual const char * GetAppName();

	virtual bool Initialize();
	virtual void Shutdown();
	virtual void Run();

private:
	bool init_gl_shaders();
	bool init_imgui();

protected:
	int m_internal_render_width;
	int m_internal_render_height;

protected:

	bool mEditorMode;
	bool mUpdatePaused;
	
	const int target_framerate = 60;

	long long tick_count;

	GLuint mShaderProgramID;
	GLuint mVertexPos2DLocation;
	GLuint mVAO;
	GLuint mVBO;
	GLuint mIBO;

	GLuint mVertexShader;
	GLuint mFragmentShader;

	GLuint mGLRenderTexFramebuffer;
	GLuint mLGRenderTex;

protected:
	virtual void start_frame();

	virtual void update();
	virtual void editor_update();

	void render_begin();
	virtual void render();
	void render_end();

	virtual void render_gamemode();
	virtual void render_editmode();

	virtual void finish_render();

	virtual void sdl_input(SDL_Event event);
};