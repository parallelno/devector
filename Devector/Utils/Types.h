#pragma once

#include <cstdint>
#include "Utils/Types.h"

namespace dev
{
	using ErrCode = int;

	using GlobalAddr = uint32_t;
	using Addr = uint16_t;
	using ColorI = uint32_t;

	// TODO: replace it with GLuint
	using GLuint1 = unsigned int;
	using GLint1 = int;
	using GLenum1 = unsigned int;
	using GLsizei1 = int;

	using Id = int;

	struct ReqHexViewer{
		enum class Type : int { NONE = 0, INIT_UPDATE, UPDATE };
		GlobalAddr globalAddr;
		GlobalAddr len;
		Type type;
		bool isActive;
	};
	struct ReqDisasm {
		enum class Type : int { 
			NONE = 0, 
			UPDATE, // redraw disasm
			UPDATE_ADDR,
			NAVIGATE_TO_ADDR,
			NAVIAGATE_NEXT,
			NAVIAGATE_PREV
			}; // redraw disasm at the prodived addr in m_reqDisasmUpdateData
		Type type = Type::NONE;
		int addr = 0;
	};

	enum class UIItemMouseAction { NONE = 0, HOVERED, LEFT, RIGHT, MIDDLE };
}