#pragma once
#ifndef DEV_DISASM_WINDOW_H
#define DEV_DISASM_WINDOW_H

#include <mutex>
#include <string>

#include "Utils/Globals.h"
#include "Utils/ImGuiUtils.h"
#include "Ui/BaseWindow.h"

#include "..\Devector\Hardware.h"

namespace dev
{
	static constexpr int DEFAULT_WINDOW_W = 600;
	static constexpr int DEFAULT_WINDOW_H = 800;

	// disasm background colors
	const ImU32 DISASM_TBL_BG_COLOR_BRK = dev::IM_U32(0x353636FF);
	const ImU32 DISASM_TBL_BG_COLOR_ADDR = dev::IM_U32(0x353636FF);
	//const ImU32 DISASM_TBL_BG_COLOR_LABELS = dev::IM_U32(0x282828FF);
	//const ImU32 DISASM_TBL_BG_COLOR_COMMENT = dev::IM_U32(0x383838FF);

	// disasm text colors
	const ImVec4 DISASM_TBL_COLOR_COMMENT = dev::IM_VEC4(0x909090FF);
	const ImVec4 DISASM_TBL_COLOR_LABEL_GLOBAL = dev::IM_VEC4(0xD0C443FF);
	const ImVec4 DISASM_TBL_COLOR_LABEL_LOCAL = dev::IM_VEC4(0xA8742FFF);
	const ImVec4 DISASM_TBL_COLOR_LABEL_MINOR = dev::IM_VEC4(0x909090FF);
	const ImVec4 DISASM_TBL_COLOR_ADDR = dev::IM_VEC4(0x909090FF);
	const ImVec4 DISASM_TBL_COLOR_ZERO_STATS = dev::IM_VEC4(0x606060FF);
	const ImVec4 DISASM_TBL_COLOR_MNEMONIC = dev::IM_VEC4(0x578DDFFF);
	const ImVec4 DISASM_TBL_COLOR_NUMBER = dev::IM_VEC4(0xD4D4D4FF);
	const ImVec4 DISASM_TBL_COLOR_REG = dev::IM_VEC4(0x1ECF44FF);
	const ImVec4 DISASM_TBL_COLOR_CONST = dev::IM_VEC4(0x8BE0E9FF);
	// disasm icons colors
	const ImU32 DISASM_TBL_COLOR_BREAKPOINT = dev::IM_U32(0xFF2828C0);
	const ImU32 DISASM_TBL_COLOR_PC = dev::IM_U32(0x88F038FF);

	static constexpr float DISASM_TBL_BREAKPOINT_SIZE = 0.35f;

	class DisasmWindow : public BaseWindow
	{	
		// Set column widths
		static constexpr float BRK_W = 20;
		static constexpr float ADDR_W = 50.0f;
		static constexpr float CODE_W = 200.0f;
		static constexpr float COMMAND_W = 120.0f;
		static constexpr float STATS_W = 100.0f;

		static constexpr int DISASM_LINES_PRIOR = 6;
		
		Hardware& m_hardware;
		ImFont* m_fontComment = nullptr;
		char m_searchText[255] = "";
		Debugger::Disasm m_disasm;

		void DrawDebugControls();
		void DrawSearch();
		void DrawDisassembly();
		bool IsDisasmTableOutOfWindow();

	public:

		DisasmWindow(Hardware& _hardware, ImFont* fontComment, const float const* _fontSizeP, const float const* _dpiScaleP);
		void Update();
		void UpdateDisasm(const uint32_t _addr, const size_t _beforeAddrLines = DISASM_LINES_PRIOR);
	};

};

#endif // !DEV_DISASM_WINDOW_H