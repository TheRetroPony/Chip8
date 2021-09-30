#include "appbase.h"

#include <SDL.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/GLU.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include <chrono>

AppBase::AppBase()
{
}

AppBase::~AppBase()
{
}

const char * AppBase::GetAppName()
{
	return s_appname;
}

void PrintProgramLog(GLuint program)
{
	//Make sure name is shader
	if (glIsProgram(program))
	{
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log
		glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			//Print Log
			printf("%s\n", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf("Name %d is not a program\n", program);
	}
}

void PrintShaderInfoLog(GLuint shader)
{
	//Make sure name is shader
	if (glIsShader(shader))
	{
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log
		glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			//Print Log
			printf("%s\n", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf("Name %d is not a shader\n", shader);
	}
}

bool AppBase::init_gl_shaders()
{
	bool success{ true };

	// ------ generate shader program -------------------------------------------
	mShaderProgramID = glCreateProgram();

	// ----------- create vertex shader ----------------------------------------------
	mVertexShader = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* vertexShaderSource[] = {
	"#version 150\nin vec2 LVertexPos2D; void main() { gl_Position = vec4( LVertexPos2D.x, LVertexPos2D.y, 0, 1 ); }"
	};

	glShaderSource(mVertexShader, 1, vertexShaderSource, NULL);
	glCompileShader(mVertexShader);

	// check vertex shader for errors
	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv(mVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if (vShaderCompiled != GL_TRUE)
	{
		std::cout << "Unable to compile vertex shader! " << mVertexShader << std::endl;
		PrintShaderInfoLog(mVertexShader);
		success = false;
	}

	// ---------- create the fragment shader ------------------------------------
	mFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* fragmentShaderSource[] =
	{
		"#version 150\nout vec4 LFragment; void main() { LFragment = vec4( 1.0, 1.0, 1.0, 1.0 ); }"
	};

	glShaderSource(mFragmentShader, 1, fragmentShaderSource, NULL);
	glCompileShader(mFragmentShader);

	// check the fragment shader for errors
	GLint fShaderCompiled = GL_FALSE;
	glGetShaderiv(mFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if (fShaderCompiled != GL_TRUE)
	{
		printf("Unable to compile fragment shader %d!\n", mFragmentShader);
		PrintShaderInfoLog(mFragmentShader);
		success = false;
	}

	// ------ attach shaders -------------------------------------------

	// Attach the vertex shader
	glAttachShader(mShaderProgramID, mVertexShader);
	glAttachShader(mShaderProgramID, mFragmentShader);
	glLinkProgram(mShaderProgramID);
	// check for errors
	GLint programSuccess = GL_TRUE;
	glGetProgramiv(mShaderProgramID, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE)
	{
		printf("Error linking program %d!\n", mShaderProgramID);
		PrintProgramLog(mShaderProgramID);
		success = false;
	}

	mVertexPos2DLocation = glGetAttribLocation(mShaderProgramID, "LVertexPos2D");
	if (mVertexPos2DLocation == -1)
	{
		printf("LVertexPos2D is not a valid glsl program variable!\n");
		success = false;
	}

	return success;
}

bool AppBase::init_imgui()
{
	// Initialize imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// enable docking
	ImGuiIO& imgui_io = ImGui::GetIO(); (void)imgui_io;

	// default style
	ImGui::StyleColorsDark();

	// init backends
	ImGui_ImplSDL2_InitForOpenGL(m_appwindow->GetSDLWindow(), m_appwindow->GetGLContext());
	ImGui_ImplOpenGL3_Init("#version 130");
	return true;
}

bool AppBase::Initialize()
{
	mEditorMode = false;
	mUpdatePaused = false;

	mShaderProgramID = 0;
	mVertexPos2DLocation = 0;
	mVAO = 0;
	mVBO = 0;
	mIBO = 0;

	mVertexShader = 0;
	mFragmentShader = 0;

	mGLRenderTexFramebuffer = 0;
	mLGRenderTex = 0;


	bool success = AppCore::Initialize();
	if (!success)
	{
		return false;
	}

	success = init_gl_shaders();
	if (!success)
	{
		return false;
	}

	success = init_imgui();
	if (!success)
	{
		return false;
	}

	// some gl buffers to get us a triangle
	// a triangle to draw
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//VBO data
	GLfloat const VertexData[] = {
		0.0f, 0.5f,
		0.5f, -0.5f,
		-0.5f, -0.5f
	};

	GLuint const IndexData[] = {
		0, 1, 2
	};

	mVBO;
	glGenBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * 3 * sizeof(GLfloat), VertexData, GL_STATIC_DRAW);

	mIBO;
	glGenBuffers(1, &mIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(GLuint), IndexData, GL_STATIC_DRAW);

	////////////////// open gl render texture ////////////////////////////////////////

	// create a framebuffer
	glGenFramebuffers(1, &mGLRenderTexFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mGLRenderTexFramebuffer);//this tex is our 'frame buffer' 
	// create a texture we will render to with that framebuffer
	glGenTextures(1, &mLGRenderTex);
	// 'bind' it so we can alter it
	glBindTexture(GL_TEXTURE_2D, mLGRenderTex);
	// create our tex
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_internal_render_width, m_internal_render_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mLGRenderTex, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		GLenum Error = glGetError();
		if (Error != GL_NO_ERROR)
		{
		}
	}
	///////////////////////////////////////////////////////////////////////////////////

	return success;
}

void AppBase::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	glDeleteProgram(mShaderProgramID);
	glDeleteShader(mFragmentShader);
	glDeleteShader(mVertexShader);
	glDeleteBuffers(1, &mIBO);
	glDeleteBuffers(1, &mVBO);
	glDeleteVertexArrays(1, &mVAO);


	// call this LAST not first! Order matters on shutdown!
	AppCore::Shutdown();
}

using namespace std::chrono_literals;
void AppBase::Run()
{
	using clock = std::chrono::high_resolution_clock;
	// tick at 400hz for now, needs to be configurable though!
	const auto target_time = 16.667ms;
	auto current_time = std::chrono::steady_clock::now();
	auto time_accumulator = 0ns;

	// Loop until the user closes the window
	bool shouldQuit = false;

	while (!shouldQuit)
	{
		tick_count++;

		// update the current time
		auto frame_start_time = std::chrono::steady_clock::now();
		auto frametime = frame_start_time - current_time;
		current_time = frame_start_time;
		time_accumulator += frametime;
		
		// handle sdl events
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				shouldQuit = true;
			if (event.type == SDL_WINDOWEVENT && 
				event.window.event == SDL_WINDOWEVENT_CLOSE && 
				event.window.windowID == SDL_GetWindowID(m_appwindow->GetSDLWindow()))
				shouldQuit = true;

			ImGui_ImplSDL2_ProcessEvent(&event);
			sdl_input(event);
		}

		// render begin, primarily to init ImGui frame
		start_frame();

		while (time_accumulator >= target_time)
		{
			time_accumulator -= std::chrono::duration_cast<std::chrono::nanoseconds>(target_time);

			// update game logic if not paused
			if (!mUpdatePaused)
			{
				update();
			}

			// game render frame
			render_begin();
			render();
			render_end();
		}

		// could always update this logic? depends I guess
		if (mEditorMode)
		{
			editor_update();
		}

		// render the game or edit mode application
		if (mEditorMode)
		{
			// render all your edit mode imgui windows
			render_editmode();
		}
		else
		{
			// primarily just blat the game render to the full app window
			render_gamemode();
		}

		finish_render();
	}
}

void AppBase::start_frame()
{
	// init imgui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void AppBase::update()
{
	// game logic update
}

void AppBase::editor_update()
{
}

void AppBase::render_begin()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mGLRenderTexFramebuffer);
	glBindTexture(GL_TEXTURE_2D, mLGRenderTex);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, m_internal_render_width, m_internal_render_height, 0);
	glViewport(0, 0, m_internal_render_width, m_internal_render_height);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void AppBase::render()
{
	// override this, do the game rendering
}

void AppBase::render_end()
{
	// unbind
	glBindFramebuffer(GL_FRAMEBUFFER, NULL);
}

void AppBase::render_gamemode()
{
	///////////////////////////////////////////////////////////

	int win_w = 0;
	int win_h = 0;// todo: set these one time on resize command
	SDL_GetWindowSize(m_appwindow->GetSDLWindow(), &win_w, &win_h);

	///////////// render our game framebuffer to the main screen if we're not in editor mode ///////////////////
	// gameplay mode, draw our render texture to the full window area

	// use the default framebuffer (backbuffer)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// set ortho view and identity modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0, win_w, 0, win_h);
	glViewport(0, 0, win_w, win_h);
	// clear
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// blit our tex to a quad 
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mLGRenderTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glColor4ub(255, 255, 255, 0);
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0); glVertex2i(0, 0);
	glTexCoord2i(0, 1); glVertex2i(0, win_h);
	glTexCoord2i(1, 1); glVertex2i(win_w, win_h);
	glTexCoord2i(1, 0); glVertex2i(win_w, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glBindFramebuffer(GL_FRAMEBUFFER, NULL);
}

void AppBase::render_editmode()
{
	int win_w = 0;
	int win_h = 0;// todo: set these one time on resize command
	SDL_GetWindowSize(m_appwindow->GetSDLWindow(), &win_w, &win_h);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluOrtho2D(0, win_w, 0, win_h);
	glViewport(0, 0, win_w, win_h);

	// clear the color and depth buffers
	glViewport(0, 0, win_w, win_h);
	glClearColor(0.0f, 0.0f, 0.0f, 1);
	glClear(GL_COLOR_BUFFER_BIT);
}

void AppBase::finish_render()
{
	// prep and draw imgui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Swap front and back buffers here
	SDL_GL_SwapWindow(m_appwindow->GetSDLWindow());

	// post-render
	glBindTexture(GL_TEXTURE_2D, 0);
}

void AppBase::sdl_input(SDL_Event event)
{
	// override to get sdl input
	int i = 0;
}
