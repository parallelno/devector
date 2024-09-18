#pragma once

#include <string>

#include "utils/types.h"
#include "utils/json_utils.h"
#include "utils/glu_utils.h"
#include "imgui_app.h"

#include "ui/breakpoints_window.h"
#include "ui/watchpoints_window.h"
#include "ui/disasm_window.h"
#include "ui/hardware_stats_window.h"
#include "ui/display_window.h"
#include "ui/mem_display_window.h"
#include "ui/hex_viewer_window.h"
#include "ui/trace_log_window.h"
#include "ui/about_window.h"
#include "ui/feedback_window.h"
#include "ui/recorder_window.h"

#include "core/hardware.h"
#include "core/debugger.h"

namespace dev
{
	class DevectorApp : public ImGuiApp
	{
		// TODO: Find out why ImGuiApp(_settingsJ, _stringPath, APP_NAME) crashes
		//const std::string APP_NAME = "Devector";
		const std::string FONT_CODE_PATH_DEFAULT = "Devector";
		static constexpr int RECENT_FILES_MAX = 10;
		const std::wstring EXT_ROM = L".ROM";
		const std::wstring EXT_FDD = L".FDD";
		const std::wstring EXT_REC = L".REC";

		enum class FileType : int {ROM = 0, FDD, REC, UNDEFINED};
		
		struct LoadingRes
		{
			enum class State {
				NONE, 
				CHECK_MOUNTED, 
				SAVE_DISCARD, 
				OPEN_FILE,
				OPEN_POPUP_SAVE_DISCARD, 
				POPUP_SAVE_DISCARD,
				OPEN_POPUP_SELECT_DRIVE,
				POPUP_SELECT_DRIVE,
				ALWAYS_DISCARD,
				DISCARD, 
				ALWAYS_SAVE,
				SAVE,
				LOAD,
				UPDATE_RECENT,
				EXIT,
			};
			enum class Type { // defines the action during State = OPEN_FILE
				OPEN_FILE_DIALOG,
				RECENT,
				SAVE_THEN_EXIT,
				SAVE_REC_FILE_DIALOG,
			};

			const char* POPUP_SELECT_DRIVE = "Fdd Setup";
			const char* POPUP_SAVE_DISCARD = "Save or Discard?";

			State state = State::NONE;
			std::wstring path;
			int driveIdx = 0;
			bool autoBoot = false;
			Type type = Type::OPEN_FILE_DIALOG;
			std::wstring pathFddUpdated;
			int driveIdxUpdated = 0;
			FileType fileType = FileType::ROM;

			void Init(const State& _state, const Type _type = Type::OPEN_FILE_DIALOG, FileType _fileType = FileType::UNDEFINED, const std::wstring& _path = L"",
				const int _driveIdx = -1, bool _autoBoot = false) 
			{
				if (state == LoadingRes::State::EXIT) return;
				fileType = _fileType;
				state = _state;
				type = _type;
				path = _path;
				driveIdx = _driveIdx;
				autoBoot = _autoBoot;
			}
		};
		LoadingRes m_loadingRes; // loading resource info

		std::unique_ptr <dev::Hardware> m_hardwareP;
		std::unique_ptr <dev::Debugger> m_debuggerP;
		std::unique_ptr <dev::BreakpointsWindow>m_breakpointsWindowP;
		std::unique_ptr <dev::HardwareStatsWindow> m_hardwareStatsWindowP;
		std::unique_ptr <dev::DisasmWindow> m_disasmWindowP;
		std::unique_ptr <dev::WatchpointsWindow>m_watchpointsWindowP;
		std::unique_ptr <dev::DisplayWindow>m_displayWindowP;
		std::unique_ptr <dev::MemDisplayWindow>m_memDisplayWindowP;
		std::unique_ptr <dev::HexViewerWindow>m_hexViewerWindowP;
		std::unique_ptr <dev::TraceLogWindow>m_traceLogWindowP;
		std::unique_ptr <dev::AboutWindow>m_aboutWindowP;
		std::unique_ptr <dev::FeedbackWindow>m_feedbackWindowP;
		std::unique_ptr <dev::RecorderWindow>m_recorderWindowP;

		bool m_displayWindowVisible = false;
		bool m_disasmWindowVisible = false;
		bool m_hardwareStatsWindowVisible = false;
		bool m_breakpointsWindowVisisble = false;
		bool m_watchpointsWindowVisible = false;
		bool m_memDisplayWindowVisible = false;
		bool m_hexViewerWindowVisible = false;
		bool m_traceLogWindowVisible = false;
		bool m_aboutWindowVisible = false;
		bool m_feedbackWindowVisible = false;
		bool m_recorderWindowVisible = false;

		bool m_ruslat = false;
		bool m_restartOnLoadFdd = false;
		bool m_ramDiskClearAfterRestart = true;
		bool m_mountRecentFddImg = true;
		std::wstring m_ramDiskDataPath = L"";
		int m_rustLatSwitched = 0;

		bool m_debuggerAttached = false;

		// path, file type, driveIdx, autoBoot
		using RecentFile = std::tuple<FileType, std::wstring, int, bool>;
		using RecentFiles = std::list<RecentFile>;
		RecentFiles m_recentFilePaths;

		ReqUI m_reqUI;
		GLUtils m_glUtils;

	public:
		DevectorApp(const std::string& _stringPath, nlohmann::json _settingsJ, const std::string& _romfddPath = "");
		~DevectorApp();

		virtual void Update();

	protected:
		void WindowsInit();
		void SettingsInit(const std::string& _path = "");
		void RecentFilesInit();
		void RecentFilesStore();
		void RecentFilesUpdate(const FileType _fileType, const std::wstring& _path, const int _driveIdx = -1, const bool _autoBoot = false);
		void AppStyleInit();
		void MainMenuUpdate();
		void Load(const std::string& _path);		
		void LoadRom(const std::wstring& _path);
		void LoadFdd(const std::wstring& _path, const int _driveIdx, const bool _autoBoot);
		void LoadRecording(const std::wstring& _path);
		void Reload();
		static bool EventFilter(void* _userdata, SDL_Event* _event);
		void DrawSaveDiscardFddPopup();
		void CheckMountedFdd();
		void SaveDiscardFdd();
		void SaveUpdatedFdd();
		void OpenFile();
		void SaveFile();
		void DrawSelectDrivePopup();
		void ResLoadingStatusHandling();
		void ReqUIHandling();
		void DebugAttach();
		void RestartOnLoadFdd();
		void MountRecentFddImg();
	};
}