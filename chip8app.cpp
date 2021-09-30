#include "chip8app.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>

#include <iostream>
#include <string.h>


struct DebugLog
{
	ImGuiTextBuffer     Buf;
	ImGuiTextFilter     Filter;
	ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
	bool                AutoScroll;  // Keep scrolling if already at the bottom.

	DebugLog()
	{
		AutoScroll = true;
		Clear();
	}

	void    Clear()
	{
		Buf.clear();
		LineOffsets.clear();
		LineOffsets.push_back(0);
	}

	void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
	{
		int old_size = Buf.size();
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n')
				LineOffsets.push_back(old_size + 1);
	}

	void    VAddLog(const char* fmt, va_list args)
	{
		int old_size = Buf.size();
		Buf.appendfv(fmt, args); 
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n')
				LineOffsets.push_back(old_size + 1);
	}

	void    Draw(const char* title, bool* p_open = NULL)
	{
		if (!ImGui::Begin(title, p_open))
		{
			ImGui::End();
			return;
		}

		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Auto-scroll", &AutoScroll);
			ImGui::EndPopup();
		}

		// Main window
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");
		ImGui::SameLine();
		bool clear = ImGui::Button("Clear");
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		Filter.Draw("Filter", -100.0f);

		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (clear)
			Clear();
		if (copy)
			ImGui::LogToClipboard();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const char* buf = Buf.begin();
		const char* buf_end = Buf.end();
		if (Filter.IsActive())
		{
			// In this example we don't use the clipper when Filter is enabled.
			// This is because we don't have a random access on the result on our filter.
			// A real application processing logs with ten of thousands of entries may want to store the result of
			// search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
			for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
			{
				const char* line_start = buf + LineOffsets[line_no];
				const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
				if (Filter.PassFilter(line_start, line_end))
					ImGui::TextUnformatted(line_start, line_end);
			}
		}
		else
		{
			// The simplest and easy way to display the entire buffer:
			//   ImGui::TextUnformatted(buf_begin, buf_end);
			// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
			// to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
			// within the visible area.
			// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
			// on your side is recommended. Using ImGuiListClipper requires
			// - A) random access into your data
			// - B) items all being the  same height,
			// both of which we can handle since we an array pointing to the beginning of each line of text.
			// When using the filter (in the block of code above) we don't have random access into the data to display
			// anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
			// it possible (and would be recommended if you want to search through tens of thousands of entries).
			ImGuiListClipper clipper;
			clipper.Begin(LineOffsets.Size);
			while (clipper.Step())
			{
				for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
				{
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					ImGui::TextUnformatted(line_start, line_end);
				}
			}
			clipper.End();
		}
		ImGui::PopStyleVar();

		if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();
		ImGui::End();
	}
};


static DebugLog gDebugLog;

const char * Chip8App::GetAppName()
{
	return s_appname;
}

void add_log(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	gDebugLog.VAddLog(fmt, args);
	va_end(args);
}

bool Chip8App::Initialize()
{
	// set the internal render size to the Chip8 resolution
	m_internal_render_width = 64;
	m_internal_render_height = 32;

	bool success = AppBase::Initialize();
	if (!success)
	{
		return false;
	}

	// start paused
	mUpdatePaused = true;
	// start in edit mode
	mEditorMode = true;

	tickOnce = false;

	// load test rom
	//chip8.LoadROMFromFile("roms/games/Paddles.ch8");
	chip8_log_func logfunc = &add_log;
	chip8.SetLogFunc(logfunc);
	//chip8.LoadROMFromFile("C:/Projects/GLFW/Chip8/Chip8/x64/Debug/roms/demos/Maze [David Winter, 199x].ch8");
	//Clock Program [Bill Fisher, 1981]chip8.LoadROMFromFile("C:/Projects/GLFW/Chip8/Chip8/x64/Debug/roms//programs/Fishie [Hap, 2005].ch8");
	//chip8.LoadROMFromFile("C:/Projects/GLFW/Chip8/Chip8/x64/Debug/roms/c8_test.c8");
	//chip8.LoadROMFromFile("C:/Projects/GLFW/Chip8/Chip8/x64/Debug/roms/test_opcode.ch8");
	//chip8.LoadROMFromFile("C:/Projects/GLFW/Chip8/Chip8/x64/Debug/roms/games/Space Invaders [David Winter].ch8");
	//chip8.LoadROMFromFile("C:/Projects/GLFW/Chip8/Chip8/x64/Debug/roms/IBM Logo.ch8");

	// todo: - file loader for roms ^
	//		 - input latching
	//		 - 
}

void Chip8App::Shutdown()
{
	AppBase::Shutdown();
}

void Chip8App::update()
{
	// appbase has been configured to tick at 60fps
	// the chip8 wants to tick around 400hz to 800hz configurable

	if (tickOnce)
	{
		chip8.Tick(tick_count);

		tickOnce = false;
		mUpdatePaused = true;
	}
	else
	{
		int ticks = (400 / 60);
		for (int i = 0; i < ticks; i++)
		{
			chip8.Tick(tick_count++);
		}
	}
}

void Chip8App::editor_update()
{
	AppBase::editor_update();
}

void Chip8App::render()
{
	// backbuffer rendered and ortho matrix setup for (mRENDER_WIDTH, mRENDER_HEIGHT)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_TEXTURE_2D);
	glPointSize(1);
	glBegin(GL_POINTS);
	uint8_t * graphics = chip8.GetScreenBuf();
	uint8_t x = 0;
	uint8_t y = 0;

	glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

	for(int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			if (graphics[x + y * 64])
			{
				glVertex2f(x, y + 1);
			}
		}
	}

	glEnd();
}

void Chip8App::render_gamemode()
{
	AppBase::render_gamemode();

	// do our gui
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Window (Gameplay Mode)");
	if (ImGui::Button("Editor Mode"))
	{
		mEditorMode = true;
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

void Chip8App::render_editmode()
{
	AppBase::render_editmode();

	glClearColor(0.1f, 0.1f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// do our edit mode gui

	/////////////////////////////////////////////////////////////////
	// GAME WINDOW
	////////////////////////////////////////////////////////////////
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Chip8");
	ImVec2 region = ImGui::GetContentRegionAvail();
	//it's a rendertexture so it's upside down for GPU reasons
	ImGui::Image((void*)(intptr_t)mLGRenderTex, region, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::End();
	ImGui::PopStyleVar();

	

	/////////////////////////////////////////////////////////////////
	// LOG WINDOW
	////////////////////////////////////////////////////////////////
	ImGui::Begin("DEBUG LOG");

	if (ImGui::Button("Open ROM..."))
	{
		OPENFILENAME ofn;
		const char* filter = "Chip8 ROMs(.ch8)\0*.ch8\0Chip8 ROMs(.c8)\0.c8\0All Files(.)\0*.*\0";
		const char* title = "Open ROM...";
		const char* default_extension = ".ch8";
		char szFileName[MAX_PATH]{ 0 };
		char szFileTitle[MAX_PATH]{ 0 };

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = GetFocus();
		ofn.lpstrFilter = filter;
		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter = 0;
		ofn.nFilterIndex = 0;
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrInitialDir = "."; // Initial directory.
		ofn.lpstrFileTitle = szFileTitle;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.lpstrTitle = title;
		ofn.lpstrDefExt = default_extension;

		ofn.Flags = OFN_FILEMUSTEXIST;

		if (!GetOpenFileName((LPOPENFILENAME)&ofn))
		{
			// error
		}
		else
		{
			chip8.LoadROMFromFile(szFileName);
		}
	}

	if (ImGui::Button("Reset Chip8"))
	{
		chip8.Reset();
		mUpdatePaused = true;
	}
	if (ImGui::Button("Gameplay Mode"))
	{
		mEditorMode = false;
		if (mUpdatePaused)
		{
			mUpdatePaused = false;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button(mUpdatePaused ? "RUN" : "PAUSE"))
	{
		mUpdatePaused = !mUpdatePaused;
	}
	ImGui::SameLine();
	if (ImGui::Button("> STEP >"))
	{
		mUpdatePaused = false;
		tickOnce = true;
	}

	ImGui::Text("--- OPCODE CONFIG ---");
	bool mode_8xy6 = chip8.GetConfig_8XY6_8XYE_VY_mode();
	bool mode_bnnn = chip8.GetConfig_BNNN_ADD_mode();
	bool mode_fx55 = chip8.GetConfig_FX55_FX65_VY_mode();
	if (ImGui::Button(mode_8xy6 == 0 ? "8XY6/E Mode: 0" : "8XY6/E Mode: 1"))
	{
		chip8.SetConfig_8XY6_8XYE_VY_mode(!mode_8xy6);
	}

	if (ImGui::Button(mode_bnnn == 0 ? "BNNN Mode: 0" : "BNNN Mode: 1"))
	{
		chip8.SetConfig_BNNN_ADD_mode(!mode_bnnn);
	}

	if (ImGui::Button(mode_fx55 == 0 ? "FX55/65 Mode: 0" : "FX55/65 Mode: 1"))
	{
		chip8.SetConfig_FX55_FX65_VY_mode(!mode_fx55);
	}

	ImGui::End();

	
	gDebugLog.Draw("DEBUG LOG", 0);

	/////////////////////////////////////////////////////////////////
	// DEBUG WINDOW
	////////////////////////////////////////////////////////////////
	uint8_t* v_regs = chip8.GetVRegs();
	uint8_t* keys = chip8.GetKeys();
	uint8_t sound_timer = chip8.GetSoundTimer();
	uint8_t delay_timer = chip8.GetDelayTimer();

	short index_reg = chip8.GetIndexRef();
	short prog_count = chip8.GetProgCount();

	uint8_t * memory = chip8.GetMemory();

	ImGui::Begin("DEBUG");

	ImGui::Text("--- REGISTERS ---");

		ImGui::Text("Program Counter: %X", prog_count);
		ImGui::Text("Index Reg: %X", index_reg);
		ImGui::Text("Delay Timer: %X", delay_timer);
		ImGui::Text("Sound Timer: %X", sound_timer);
		ImGui::Text("");
		ImGui::Text("V REGISTERS");
		ImGui::Text("");

		for (int i = 0; i < 16; i++)
		{
			ImGui::Text("V%X: ", i);
			ImGui::SameLine();
			ImGui::Text("%X: ", v_regs[i]);
			if (i % 2 == 0)
			{
				ImGui::SameLine();
			}
		}

		ImGui::Text("--- KEYS ---");
		for (int i = 0; i < 16; i++)
		{
			ImGui::Text("%i: [%X] ", i, keys[i]);
			if (i % 4 != 0)
			{
				ImGui::SameLine();
			}
		}
	
	ImGui::End();

	ImGui::Begin("MEMORY");
	long memsize = chip8.GetMemorySize();
	prog_count;

	for (int i = 0x200; i < memsize; i += 8)
	{
		for(int j = 0; j < 8; j+=2)
		{
			if (prog_count == (i + j))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
			}
			ImGui::Text("%02X%02X ", memory[i + j], memory[i + j + 1]);
			if (prog_count == (i + j))
			{
				ImGui::PopStyleColor();
			}

			if(j < 6) ImGui::SameLine();
		}
		
	}

	ImGui::End();
}

void Chip8App::sdl_input(SDL_Event event)
{
	switch (event.type)
	{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			uint8_t key = 255;
			switch (event.key.keysym.scancode)
			{
				case SDL_SCANCODE_1: key = 0; break;
				case SDL_SCANCODE_2: key = 1; break;
				case SDL_SCANCODE_3: key = 2; break;
				case SDL_SCANCODE_4: key = 3; break;

				case SDL_SCANCODE_Q: key = 4; break;
				case SDL_SCANCODE_W: key = 5; break;
				case SDL_SCANCODE_E: key = 6; break;
				case SDL_SCANCODE_R: key = 7; break;

				case SDL_SCANCODE_A: key = 8; break;
				case SDL_SCANCODE_S: key = 9; break;
				case SDL_SCANCODE_D: key = 10; break;
				case SDL_SCANCODE_F: key = 11; break;

				case SDL_SCANCODE_Z: key = 12; break;
				case SDL_SCANCODE_X: key = 13; break;
				case SDL_SCANCODE_C: key = 14; break;
				case SDL_SCANCODE_V: key = 15; break;
				default: break;
			}

			if (key != 255)
			{
				chip8.SetKey(key, (event.type == SDL_KEYDOWN));
			}

			break;
		}
	}
}

