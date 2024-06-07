#pragma once

#include <string>
#include <list>
#include <format>

#include "imgui.h"

#include "Types.h"
#include "Core/Breakpoint.h"
#include "Core/Debugger.h"
#include "Core/Disasm.h"

namespace dev 
{

#define DEV_COL32_R_MASK     0xFF000000
#define DEV_COL32_G_MASK     0x00FF0000
#define DEV_COL32_B_MASK     0x0000FF00
#define DEV_COL32_A_MASK     0x000000FF

#define DEV_COL32_R_SHIFT    24
#define DEV_COL32_G_SHIFT    16
#define DEV_COL32_B_SHIFT    8
#define DEV_COL32_A_SHIFT    0

	enum class ReqPopup : int {
		NONE = 0,
		INIT_ADD,
		INIT_EDIT,
		ADD,
		EDIT
	};

	// 0xRRGGBBAA to Vec4(r,g,b,a)
	static constexpr ImVec4 IM_VEC4(const uint32_t _color)
	{
		return ImVec4(
			((_color & DEV_COL32_R_MASK) >> DEV_COL32_R_SHIFT) / 255.0f,
			((_color & DEV_COL32_G_MASK) >> DEV_COL32_G_SHIFT) / 255.0f,
			((_color & DEV_COL32_B_MASK) >> DEV_COL32_B_SHIFT) / 255.0f,
			((_color & DEV_COL32_A_MASK) >> DEV_COL32_A_SHIFT) / 255.0f);
	}
	// 0xRGBA to 0xABGR
	static constexpr ImU32 IM_U32(const uint32_t _color)
	{
		return
			(_color & DEV_COL32_R_MASK) >> DEV_COL32_R_SHIFT << 0 |
			(_color & DEV_COL32_G_MASK) >> DEV_COL32_G_SHIFT << 8 |
			(_color & DEV_COL32_B_MASK) >> DEV_COL32_B_SHIFT << 16 |
			(_color & DEV_COL32_A_MASK) >> DEV_COL32_A_SHIFT << 24;
	}

	// disasm background colors
	static constexpr ImU32 DIS_BG_CLR_BRK = dev::IM_U32(0x353636FF);
	static constexpr ImU32 DASM_BG_CLR_ADDR = dev::IM_U32(0x353636FF);
	static constexpr ImU32 DASM_BG_CLR_ADDR_HIGHLIGHT = dev::IM_U32(0x640A96FF);

	// disasm text colors
	static constexpr ImVec4 DASM_CLR_COMMENT = dev::IM_VEC4(0x909090FF);
	static constexpr ImVec4 DASM_CLR_LABEL_GLOBAL = dev::IM_VEC4(0xE8DC5AFF);
	static constexpr ImVec4 DASM_CLR_LABEL_GLOBAL_IMM = dev::IM_VEC4(0xB0A46CFF); // a label that replaces an immediate operand
	static constexpr ImVec4 DASM_CLR_LABEL_LOCAL = dev::IM_VEC4(0xA8742FFF);
	static constexpr ImVec4 DASM_CLR_LABEL_LOCAL_IMM = dev::IM_VEC4(0x9E8154FF); // a label that replaces an immediate operand
	static constexpr ImVec4 DASM_CLR_LABEL_MINOR = dev::IM_VEC4(0x909090FF);
	static constexpr ImVec4 DASM_CLR_ADDR = dev::IM_VEC4(0x909090FF);
	static constexpr ImVec4 DASM_CLR_ZERO_STATS = dev::IM_VEC4(0x606060FF);
	static constexpr ImVec4 DASM_CLR_MNEMONIC = dev::IM_VEC4(0x578DDFFF);
	static constexpr ImVec4 DASM_CLR_MNEMONIC_BRANCH = dev::IM_VEC4(0x57FF6FFF);
	static constexpr ImVec4 DASM_CLR_NUMBER = dev::IM_VEC4(0xD0D0D0FF);
	static constexpr ImVec4 DASM_CLR_NUMBER_HIGHLIGHT = dev::IM_VEC4(0xFFFFFFFF);
	static constexpr ImVec4 DASM_CLR_REG = dev::IM_VEC4(0x1ECF44FF);
	static constexpr ImVec4 DASM_CLR_CONST = dev::IM_VEC4(0x8BE0E9FF);

	constexpr ImU32 DASM_CLR_PC = dev::IM_U32(0x88F038FF);

	// Make the UI compact because there are so many fields
	void PushStyleCompact(const float _paddingMulX = 1.0f, const float _paddingMulY = 0.6f);
	void PopStyleCompact();
	void UpdatePropertyPrintStat(const char* _parameterName);
	void ColumnClippingEnable(const float _dpiScale = 1.0f);
	void ColumnClippingDisable();


	void DrawTextSelectable(const char* _label, const std::string& _text);
	//void DrawTextSelectableColored(const ImVec4& col, const std::string& _text); 
	template <typename... Args>
	void DrawTextSelectableColored(const ImVec4& col, const std::string& _fmt, Args&&... args)
	{
		std::string text = std::vformat(_fmt, std::make_format_args(args...));

		ImVec2 text_size = ImGui::CalcTextSize(text.c_str(), text.c_str() + text.size());
		text_size.x = -FLT_MIN; // fill width (suppresses label)
		text_size.y += ImGui::GetStyle().FramePadding.y; // single pad

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 }); // make align with text height
		ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.f, 0.f, 0.f, 0.f }); // remove text input box

		ImGui::PushStyleColor(ImGuiCol_Text, col);


		ImGui::InputTextMultiline(
			"",
			const_cast<char*>(text.c_str()), // ugly const cast
			text.size() + 1, // needs to include '\0'
			text_size,
			ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_NoHorizontalScroll
		);
		ImGui::PopStyleColor();

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}
	void DrawHelpMarker(const char* _text);
	void DrawPopup(const char* _title, const char* _text);
	void DrawProgramCounter(const ImU32 _color, const ImGuiDir _dir = ImGuiDir_Right, const float _dpiScale = 1.0f, const float _posXOffset = 0.6f, const bool _itemHasSize = false);
	bool DrawBreakpoint(const char* label, Breakpoint::Status* _statusP, const float _dpiScale, const float _posXOffset = -0.2f, const bool _itemHasSize = false);
	void DrawProperty2(const std::string& _name, const std::string& _value, const std::string& _hint = "");
	void DrawSeparator2(const std::string& _text);
	void DrawProperty2EditableI(const char* _name, const char* _label, int* _value, const char* _help = "");
	void DrawProperty2EditableS(const char* _name, const char* _label, std::string* _value, 
			const char* _hint = "", const char* _help = "", const ImGuiInputTextFlags _flags = ImGuiInputTextFlags_EnterReturnsTrue);
	void DrawProperty2EditableCheckBox(const char* _name, const char* _label, bool* _val, const char* _help = "");
	void TextAligned(const char* _text, const ImVec2& aligment = { 1.0f, 0.5f });
	auto DrawCodeLine(const bool _isRunning, const Disasm::Line& _line, const bool _tab) -> UIItemMouseAction;
	auto DrawAddr(const bool _isRunning, const char* _operandS, const ImVec4& _color, 
		const ImVec4& _highlightColor, bool _forceHighlight = false) -> UIItemMouseAction;
	auto DrawSaveDiscardFddPopup(int _selectedDriveIdx, const std::wstring& _mountedFddPath) -> FddStatus;
}