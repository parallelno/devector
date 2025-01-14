#pragma once

#include "utils/imgui_utils.h"
#include "ui/base_window.h"

namespace dev
{
	class AboutWindow : public BaseWindow
	{
		static constexpr int DEFAULT_WINDOW_W = 500;
		static constexpr int DEFAULT_WINDOW_H = 300;

		const std::string compilation_date = __DATE__;

	public:
		AboutWindow(const float* const _dpiScaleP);
		void Update(bool& _visible, const bool _isRunning);
		void Draw();
	};
};