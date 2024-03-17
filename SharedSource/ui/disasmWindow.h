#pragma once
#ifndef DEV_DISASM_WINDOW_H
#define DEV_DISASM_WINDOW_H

#include <mutex>
#include <string>

#include "Utils/Globals.h"
#include "Utils/ImGuiUtils.h"
#include "Ui/BaseWindow.h"

#include "..\Devector\Hardware.h"
#include "..\Devector\Debugger.h"

namespace dev
{
	static constexpr int DEFAULT_WINDOW_W = 600;
	static constexpr int DEFAULT_WINDOW_H = 800;

	// disasm background colors
	static constexpr ImU32 DISASM_TBL_BG_COLOR_BRK = dev::IM_U32(0x353636FF);
	static constexpr ImU32 DISASM_TBL_BG_COLOR_ADDR = dev::IM_U32(0x353636FF);
	//static constexpr ImU32 DISASM_TBL_BG_COLOR_LABELS = dev::IM_U32(0x282828FF);
	//static constexpr ImU32 DISASM_TBL_BG_COLOR_COMMENT = dev::IM_U32(0x383838FF);

	// disasm text colors
	static constexpr ImVec4 DISASM_TBL_COLOR_COMMENT = dev::IM_VEC4(0x909090FF);
	static constexpr ImVec4 DISASM_TBL_COLOR_LABEL_GLOBAL = dev::IM_VEC4(0xD0C443FF);
	static constexpr ImVec4 DISASM_TBL_COLOR_LABEL_LOCAL = dev::IM_VEC4(0xA8742FFF);
	static constexpr ImVec4 DISASM_TBL_COLOR_LABEL_MINOR = dev::IM_VEC4(0x909090FF);
	static constexpr ImVec4 DISASM_TBL_COLOR_ADDR = dev::IM_VEC4(0x909090FF);
	static constexpr ImVec4 DISASM_TBL_COLOR_ZERO_STATS = dev::IM_VEC4(0x606060FF);
	static constexpr ImVec4 DISASM_TBL_COLOR_MNEMONIC = dev::IM_VEC4(0x578DDFFF);
	static constexpr ImVec4 DISASM_TBL_COLOR_NUMBER = dev::IM_VEC4(0xD4D4D4FF);
	static constexpr ImVec4 DISASM_TBL_COLOR_REG = dev::IM_VEC4(0x1ECF44FF);
	static constexpr ImVec4 DISASM_TBL_COLOR_CONST = dev::IM_VEC4(0x8BE0E9FF);
	// disasm icons colors
	constexpr ImU32 DISASM_TBL_COLOR_PC = dev::IM_U32(0x88F038FF);

	class DisasmWindow : public BaseWindow
	{	
		// Set column widths
		static constexpr float BRK_W = 20;
		static constexpr float ADDR_W = 50.0f;
		static constexpr float CODE_W = 200.0f;
		static constexpr float COMMAND_W = 120.0f;
		static constexpr float STATS_W = 100.0f;

		static constexpr int DISASM_INSTRUCTION_OFFSET = -6;
		
		Hardware& m_hardware;
		Debugger& m_debugger;
		ImFont* m_fontCommentP = nullptr;
		bool& m_reqDisasmUpdate;
		char m_searchText[255] = "";
		Debugger::Disasm m_disasm;

		void DrawDebugControls();
		void DrawSearch();
		void DrawDisassembly();
		bool IsDisasmTableOutOfWindow();

	public:

		DisasmWindow(Hardware& _hardware, Debugger& _debugger, ImFont* fontComment, const float* const _fontSizeP, const float* const _dpiScaleP, bool& _reqDisasmUpdate);
		void Update();
		void UpdateDisasm(const GlobalAddr _globalAddr, const int _instructionsOffset = DISASM_INSTRUCTION_OFFSET);
	};

};

#endif // !DEV_DISASM_WINDOW_H