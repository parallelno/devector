﻿#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <cstdint>
#include "GLFW/glfw3.h"
#include "DevectorApp.h"
#include "Utils/Utils.h"
#include "Utils/JsonUtils.h"
#include "Utils/StrUtils.h"

dev::DevectorApp::DevectorApp(
	const std::string& _stringPath, nlohmann::json _settingsJ)
	:
	ImGuiApp(_settingsJ, _stringPath, APP_NAME),
	m_glUtils()
{
	SettingsInit();
	WindowsInit();
}

dev::DevectorApp::~DevectorApp()
{
	 SettingsUpdate("breakpointsWindowVisisble", m_breakpointsWindowVisisble);
	 SettingsUpdate("hardwareStatsWindowVisible", m_hardwareStatsWindowVisible);
	 SettingsUpdate("disasmWindowVisible", m_disasmWindowVisible);
	 SettingsUpdate("watchpointsWindowVisible", m_watchpointsWindowVisible);
	 SettingsUpdate("displayWindowVisible", m_displayWindowVisible);
	 SettingsUpdate("memDisplayWindowVisible", m_memDisplayWindowVisible);
	 SettingsUpdate("hexViewerWindowVisible", m_hexViewerWindowVisible);
	 SettingsUpdate("traceLogWindowVisible", m_traceLogWindowVisible);
	 SettingsUpdate("recorderWindowVisible", m_recorderWindowVisible);
}

void dev::DevectorApp::WindowsInit()
{
	std::wstring pathBootData = dev::StrToStrW(GetSettingsString("bootPath", "boot//boot.bin"));
	m_restartOnLoadFdd = GetSettingsBool("restartOnLoadFdd", true);

	m_hardwareP = std::make_unique < dev::Hardware>(pathBootData);
	m_debuggerP = std::make_unique < dev::Debugger>(*m_hardwareP);
	m_hardwareStatsWindowP = std::make_unique<dev::HardwareStatsWindow>(*m_hardwareP, &m_fontSize, &m_dpiScale, m_ruslat);
	m_disasmWindowP = std::make_unique<dev::DisasmWindow>(*m_hardwareP, *m_debuggerP, 
		m_fontItalic, &m_fontSize, &m_dpiScale, m_reqUI);
	m_displayWindowP = std::make_unique<dev::DisplayWindow>(*m_hardwareP, &m_fontSize, &m_dpiScale, m_glUtils);
	m_breakpointsWindowP = std::make_unique<dev::BreakpointsWindow>(*m_hardwareP, &m_fontSize, &m_dpiScale, m_reqUI);
	m_watchpointsWindowP = std::make_unique<dev::WatchpointsWindow>(*m_hardwareP, &m_fontSize, &m_dpiScale, m_reqUI);
	m_memDisplayWindowP = std::make_unique<dev::MemDisplayWindow>(*m_hardwareP, *m_debuggerP, &m_fontSize, &m_dpiScale, m_glUtils, m_reqUI);
	m_hexViewerWindowP = std::make_unique<dev::HexViewerWindow>(*m_hardwareP, *m_debuggerP, &m_fontSize, &m_dpiScale, m_reqUI);
	m_traceLogWindowP = std::make_unique<dev::TraceLogWindow>(*m_hardwareP, *m_debuggerP, &m_fontSize, &m_dpiScale, m_reqUI);
	m_aboutWindowP = std::make_unique<dev::AboutWindow>(&m_fontSize, &m_dpiScale);
	m_feedbackWindowP = std::make_unique<dev::FeedbackWindow>(&m_fontSize, &m_dpiScale);
	m_recorderWindowP = std::make_unique<dev::RecorderWindow>(*m_hardwareP, *m_debuggerP, &m_fontSize, &m_dpiScale, m_reqUI);

	// Set the key callback function
	glfwSetWindowUserPointer(m_window, this);
	ImGui_ImplGlfw_KeyCallback = glfwSetKeyCallback(m_window, DevectorApp::KeyHandling);

	m_hardwareP->Request(Hardware::Req::RUN);
}

void dev::DevectorApp::SettingsInit()
{
	Request(REQ::LOAD_FONT);
	AppStyleInit();
	RecentFilesInit();

	m_breakpointsWindowVisisble = GetSettingsBool("breakpointsWindowVisisble", false);
	m_hardwareStatsWindowVisible = GetSettingsBool("hardwareStatsWindowVisible", false);
	m_disasmWindowVisible = GetSettingsBool("disasmWindowVisible", false);
	m_watchpointsWindowVisible = GetSettingsBool("watchpointsWindowVisible", false);
	m_displayWindowVisible = GetSettingsBool("displayWindowVisible", true);
	m_memDisplayWindowVisible = GetSettingsBool("memDisplayWindowVisible", false);
	m_hexViewerWindowVisible = GetSettingsBool("hexViewerWindowVisible", false);
	m_traceLogWindowVisible = GetSettingsBool("traceLogWindowVisible", false);
	m_recorderWindowVisible = GetSettingsBool("recorderWindowVisible", false);
}


// UI thread
void dev::DevectorApp::Update()
{
	ReqUIHandling();
	
	MainMenuUpdate();
	
	DebugAttach();

	ResLoadingStatusHandling();

	m_hardwareStatsWindowP->Update(m_hardwareStatsWindowVisible);
	m_disasmWindowP->Update(m_disasmWindowVisible);
	m_displayWindowP->Update(m_displayWindowVisible);
	m_breakpointsWindowP->Update(m_breakpointsWindowVisisble);
	m_watchpointsWindowP->Update(m_watchpointsWindowVisible);
	m_memDisplayWindowP->Update(m_memDisplayWindowVisible);
	m_hexViewerWindowP->Update(m_hexViewerWindowVisible);
	m_traceLogWindowP->Update(m_traceLogWindowVisible);
	m_aboutWindowP->Update(m_aboutWindowVisible);
	m_feedbackWindowP->Update(m_feedbackWindowVisible);
	m_recorderWindowP->Update(m_recorderWindowVisible);

	if (m_status == AppStatus::CHECK_MOUNTED_FDDS)
	{
		m_loadingRes.Init(LoadingRes::State::CHECK_MOUNTED, LoadingRes::Type::SAVE_THEN_EXIT);
		m_status = AppStatus::WAIT_FOR_SAVING;
	}

	if (m_restartOnLoadFdd) RestartOnLoadFdd();
}

// auto press ruslat after loading fdd
void dev::DevectorApp::RestartOnLoadFdd()
{
	// ruslat
	auto ruslatHistoryJ = *m_hardwareP->Request(Hardware::Req::GET_RUSLAT_HISTORY);
	auto m_ruslatHistory = ruslatHistoryJ["data"].get<uint32_t>();
	bool newRusLat = (m_ruslatHistory & 0b1000) != 0;

	if (newRusLat != m_ruslat) {
		if (m_rustLatSwitched++ > 2)
		{
			m_rustLatSwitched = 0;
			auto romEnabledJ = *m_hardwareP->Request(Hardware::Req::IS_ROM_ENABLED);
			if (romEnabledJ["data"]) {
				m_hardwareP->Request(Hardware::Req::RESTART);
			}
		}
	}
	m_ruslat = (m_ruslatHistory & 0b1000) != 0;
}

void dev::DevectorApp::LoadRom(const std::wstring& _path)
{
	auto result = dev::LoadFile(_path);
	if (!result || result->empty()) {
		dev::Log(L"Error occurred while loading the file. Path: {}. "
			"Please ensure the file exists and you have the correct permissions to read it.", _path);
		return;
	}

	m_hardwareP->Request(Hardware::Req::STOP);
	m_hardwareP->Request(Hardware::Req::RESET);
	m_hardwareP->Request(Hardware::Req::RESTART);

	auto reqData = nlohmann::json({ {"data", *result}, {"addr", Memory::ROM_LOAD_ADDR} });
	m_hardwareP->Request(Hardware::Req::SET_MEM, reqData);

	m_hardwareP->Request(Hardware::Req::DEBUG_RESET); // has to be called after Hardware loading Rom because it stores the last state of Hardware
	

	m_debuggerP->GetDebugData().LoadDebugData(_path);
	m_hardwareP->Request(Hardware::Req::RUN);

	Log(L"File loaded: {}", _path);
}

void dev::DevectorApp::LoadFdd(const std::wstring& _path, const int _driveIdx, const bool _autoBoot)
{
	auto fddResult = dev::LoadFile(_path);
	if (!fddResult || fddResult->empty()) {
		dev::Log(L"Error occurred while loading the file. "
			"Please ensure the file exists and you have the correct permissions to read it. Path: {}", _path);
		return;
	}
	if (fddResult->size() > FDisk::dataLen) {
		dev::Log(L"Fdc1793: disk image is too big. size: {} bytes, path: {}", fddResult->size(), _path);
		return;
	}

	if (_autoBoot) m_hardwareP->Request(Hardware::Req::STOP);

	// loading the fdd data
	m_hardwareP->Request(Hardware::Req::LOAD_FDD, {
		{"data", *fddResult },
		{"driveIdx", _driveIdx},
		{"path", dev::StrWToStr(_path)}
	});

	if (_autoBoot)
	{
		m_hardwareP->Request(Hardware::Req::RESET);
		m_hardwareP->Request(Hardware::Req::DEBUG_RESET); // has to be called after Hardware loading FDD image because it stores the last state of Hardware
		m_hardwareP->Request(Hardware::Req::RUN);
	}

	Log(L"File loaded: {}", _path);
}

void dev::DevectorApp::Reload()
{
	if (m_recentFilePaths.empty()) return;
	// get latest recent path
	const auto& [path, _driveIdx, _autoBoot] = m_recentFilePaths.front();

	if (_driveIdx < 0) LoadRom(path);
	else LoadFdd(path, _driveIdx, _autoBoot);
}

void dev::DevectorApp::MainMenuUpdate()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open", "Ctrl+O"))
			{
				m_loadingRes.Init(LoadingRes::State::CHECK_MOUNTED, LoadingRes::Type::OPEN_FILE_DIALOG);
			}
			if (ImGui::BeginMenu("Recent Files"))
			{
				for (const auto& [path, driveIdx, autoBoot] : m_recentFilePaths)
				{
					std::string itemS = dev::StrWToStr(path);
					if (driveIdx >= 0)
					{
						itemS += std::format(":{}", driveIdx);
						itemS += autoBoot ? "A" : "";
					}

					if (ImGui::MenuItem(itemS.c_str()))
					{
						m_loadingRes.Init(LoadingRes::State::CHECK_MOUNTED, LoadingRes::Type::RECENT, path, driveIdx, autoBoot);
					}
				}
				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Quit", "Alt+F4")) { m_status = AppStatus::EXIT; }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools"))
		{
			ImGui::MenuItem(m_displayWindowP->m_name.c_str(), NULL, &m_displayWindowVisible);
			ImGui::MenuItem(m_hardwareStatsWindowP->m_name.c_str(), NULL, &m_hardwareStatsWindowVisible);
			ImGui::MenuItem(m_disasmWindowP->m_name.c_str(), NULL, &m_disasmWindowVisible);
			ImGui::MenuItem(m_breakpointsWindowP->m_name.c_str(), NULL, &m_breakpointsWindowVisisble);
			ImGui::MenuItem(m_watchpointsWindowP->m_name.c_str(), NULL, &m_watchpointsWindowVisible);
			ImGui::MenuItem(m_memDisplayWindowP->m_name.c_str(), NULL, &m_memDisplayWindowVisible);
			ImGui::MenuItem(m_hexViewerWindowP->m_name.c_str(), NULL, &m_hexViewerWindowVisible);
			ImGui::MenuItem(m_traceLogWindowP->m_name.c_str(), NULL, &m_traceLogWindowVisible);
			ImGui::MenuItem(m_recorderWindowP->m_name.c_str(), NULL, &m_recorderWindowVisible);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			//ImGui::MenuItem(m_feedbackWindowP->m_name.c_str(), NULL, &m_feedbackWindowVisible);
			if (ImGui::MenuItem("zx-pk.ru: Vector06C Development"))
			{
				dev::OsOpenInShell("https://zx-pk.ru/threads/34480-programmirovanie.html");
			}
			if (ImGui::MenuItem("Vector06C Software Catalog"))
			{
				dev::OsOpenInShell("https://caglrc.cc/scalar/recent20/");
			}
			if (ImGui::MenuItem("Telegram Channel: Guides & Updates"))
			{
				dev::OsOpenInShell("https://t.me/devector06C");
			}
			if (ImGui::MenuItem("zx-pk.ru: Devector Discussions"))
			{
				dev::OsOpenInShell("https://zx-pk.ru/threads/35808-devector-emulyator-kompyutera-vektor-06ts.html");
			}
			ImGui::MenuItem(m_aboutWindowP->m_name.c_str(), NULL, &m_aboutWindowVisible);
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

void dev::DevectorApp::ResLoadingStatusHandling()
{
	// handle the statuses
	switch (m_loadingRes.state)
	{
	case LoadingRes::State::CHECK_MOUNTED:
		CheckMountedFdd(); 
		break;

	case LoadingRes::State::SAVE_DISCARD:
		SaveDiscardFdd();
		break;

	case LoadingRes::State::OPEN_POPUP_SAVE_DISCARD:
		ImGui::OpenPopup(m_loadingRes.POPUP_SAVE_DISCARD);
		m_loadingRes.state = LoadingRes::State::POPUP_SAVE_DISCARD;
		break;

	case LoadingRes::State::POPUP_SAVE_DISCARD:
		DrawSaveDiscardFddPopup();
		break;

	case LoadingRes::State::ALWAYS_DISCARD:
		SettingsUpdate("showSaveDiscardFddDialog", false);
		SettingsUpdate("discardFddChanges", true);
		[[fallthrough]];
	case LoadingRes::State::DISCARD:
		m_loadingRes.state = LoadingRes::State::OPEN_FILE;
		break;

	case LoadingRes::State::ALWAYS_SAVE:
		SettingsUpdate("showSaveDiscardFddDialog", false);
		SettingsUpdate("discardFddChanges", false);
		[[fallthrough]];
	case LoadingRes::State::SAVE:
		SaveUpdatedFdd();
		m_loadingRes.state = LoadingRes::State::OPEN_FILE;
		break;

	case LoadingRes::State::OPEN_FILE:
		if (m_loadingRes.type == LoadingRes::Type::RECENT) {
			m_loadingRes.state = LoadingRes::State::LOAD;
			break;
		}
		else if (m_loadingRes.type == LoadingRes::Type::SAVE_THEN_EXIT) 
		{
			m_status = AppStatus::EXIT;
			m_loadingRes.state = LoadingRes::State::EXIT;
			break;
		}

		OpenFile();
		break;

	case LoadingRes::State::OPEN_POPUP_SELECT_DRIVE:
		ImGui::OpenPopup(m_loadingRes.POPUP_SELECT_DRIVE);
		m_loadingRes.state = LoadingRes::State::POPUP_SELECT_DRIVE;
		break;

	case LoadingRes::State::POPUP_SELECT_DRIVE:
		DrawSelectDrivePopup();
		break;

	case LoadingRes::State::LOAD:
	{
		auto ext = StrToUpper(dev::GetExt(m_loadingRes.path));

		if (ext == EXT_ROM) {
			LoadRom(m_loadingRes.path);
		}
		else if (ext == EXT_FDD) {
			LoadFdd(m_loadingRes.path, m_loadingRes.driveIdx, m_loadingRes.autoBoot);
		}
		m_loadingRes.state = LoadingRes::State::UPDATE_RECENT;
		break;
	}
	case LoadingRes::State::UPDATE_RECENT:
		RecentFilesUpdate(m_loadingRes.path, m_loadingRes.driveIdx, m_loadingRes.autoBoot);
		RecentFilesStore();
		m_loadingRes.state = LoadingRes::State::NONE;
		break;

	default:
		break;
	}
}

void dev::DevectorApp::RecentFilesInit()
{
	auto recentFiles = GetSettingsObject("recentFiles");
	for (const auto& path_driveIdx_autoBoot : recentFiles)
	{
		auto path = dev::StrToStrW(path_driveIdx_autoBoot[0]);
		int driveIdx = path_driveIdx_autoBoot[1];
		bool autoBoot = path_driveIdx_autoBoot[2];

		m_recentFilePaths.push_back({ path, driveIdx, autoBoot });
	}
}

void dev::DevectorApp::RecentFilesStore()
{
	nlohmann::json recentFiles;
	for (const auto& [path, driveIdx, autoBoot] : m_recentFilePaths)
	{
		nlohmann::json item = { dev::StrWToStr(path), driveIdx, autoBoot };

		recentFiles.push_back(item);
	}
	SettingsUpdate("recentFiles", recentFiles);
	SettingsSave(m_stringPath);
}

void dev::DevectorApp::RecentFilesUpdate(const std::wstring& _path, const int _driveIdx, const bool _autoBoot)
{
	// remove if it contains
	m_recentFilePaths.remove_if(
		[&_path](const auto& tuple) {
			return std::get<0>(tuple) == _path;
		}
	);

	// add a new one
	m_recentFilePaths.push_front({_path, _driveIdx, _autoBoot });
	// check the amount
	if (m_recentFilePaths.size() > RECENT_FILES_MAX)
	{
		m_recentFilePaths.pop_back();
	}
}

#define SCANCODE_ALT 0x38
void dev::DevectorApp::KeyHandling(GLFWwindow* _window, int _key, int _scancode, int _action, int _modes)
{
	// Retrieve the user pointer to access the class instance
	DevectorApp* instance = static_cast<DevectorApp*>(glfwGetWindowUserPointer(_window));
	if (instance) 
	{
		auto displayFocused = instance->m_displayWindowP->IsFocused();
		if (displayFocused)
		{
			instance->m_hardwareP->Request(Hardware::Req::KEY_HANDLING, { { "key", _key }, { "action", _action} });
		}
		
		if (!displayFocused || _scancode != SCANCODE_ALT) {
			instance->ImGui_ImplGlfw_KeyCallback(_window, _key, _scancode, _action, _modes);
		}
	}
}

void dev::DevectorApp::AppStyleInit()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameBorderSize = 1.0f;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.83f, 0.83f, 0.83f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.09f, 0.09f, 0.12f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.24f, 0.24f, 0.24f, 0.25f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.08f, 0.07f, 0.07f, 0.11f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.13f, 0.45f, 0.80f, 0.52f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.50f, 0.83f, 0.63f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.24f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.45f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.61f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.25f, 0.57f, 0.82f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.13f, 0.43f, 0.78f, 0.55f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.14f, 0.56f, 0.97f, 0.83f);
	colors[ImGuiCol_Button] = ImVec4(0.22f, 0.56f, 1.00f, 0.69f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.05f, 0.37f, 0.74f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.08f, 0.35f, 0.70f, 0.69f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.31f, 0.70f, 0.64f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.08f, 0.35f, 0.60f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.53f, 0.55f, 0.75f, 0.11f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.30f, 0.35f, 0.41f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.27f, 0.29f, 0.31f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.07f, 0.36f, 0.71f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.09f, 0.35f, 0.66f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.45f, 0.44f, 0.53f, 0.32f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.53f, 0.53f, 0.53f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.53f, 0.69f, 0.84f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.15f, 0.40f, 0.93f, 0.66f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.45f, 0.54f, 0.73f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.24f, 0.24f, 0.24f, 0.82f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.24f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.35f, 0.34f, 0.40f, 0.11f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.02f, 0.41f, 0.87f, 0.84f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.04f, 0.43f, 0.88f, 0.76f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

// check if any fdds were updated
void dev::DevectorApp::CheckMountedFdd()
{
	m_loadingRes.state = LoadingRes::State::OPEN_FILE;
	for (int driveIdx = 0; driveIdx < Fdc1793::DRIVES_MAX; driveIdx++)
	{
		auto fddInfo = *m_hardwareP->Request(
			Hardware::Req::GET_FDD_INFO, { {"driveIdx", driveIdx} });
		
		if (fddInfo["updated"])
		{
			m_loadingRes.pathFddUpdated = dev::StrToStrW(fddInfo["path"]);
			m_loadingRes.driveIdxUpdated = driveIdx;
			m_loadingRes.state = LoadingRes::State::SAVE_DISCARD;
			break;
		}
	}
}

// check if we need to open a popup
void dev::DevectorApp::SaveDiscardFdd()
{
	if (GetSettingsBool("showSaveDiscardFddDialog", true))
	{
		m_loadingRes.state = LoadingRes::State::OPEN_POPUP_SAVE_DISCARD;
	}
	else {
		auto discardFddChanges = GetSettingsBool("discardFddChanges", true);
		m_loadingRes.state = discardFddChanges ? LoadingRes::State::OPEN_FILE : LoadingRes::State::SAVE;
	}
}

// Popup. Save or Discard mounted updated fdd image?
void dev::DevectorApp::DrawSaveDiscardFddPopup()
{
	ImVec2 center = ImGui::GetMainViewport()->GetCenter(); 	// Always center this window when appearing
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(m_loadingRes.POPUP_SAVE_DISCARD, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static const char* diskNames[] = { "A", "B", "C", "D" };
		ImGui::Text("Previously mounted disk %s was updated. Save or discard changes?", 
			diskNames[m_loadingRes.driveIdxUpdated]);
		ImGui::Text(dev::StrWToStr(m_loadingRes.pathFddUpdated).c_str());

		ImGui::NewLine();
		static bool doNotAskAgain = false;
		ImGui::Checkbox("##oldFddDoNotAskAgain", &doNotAskAgain);
		ImGui::SameLine();
		ImGui::Text("Don't ask again");
		ImGui::Separator();

		if (ImGui::Button("Save", ImVec2(120, 0)))
		{
			m_loadingRes.state = doNotAskAgain ? LoadingRes::State::ALWAYS_SAVE : LoadingRes::State::SAVE;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Discard", ImVec2(120, 0)))
		{
			m_loadingRes.state = doNotAskAgain ? LoadingRes::State::ALWAYS_DISCARD : LoadingRes::State::DISCARD;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

// store the mounted updated fdd image
void dev::DevectorApp::SaveUpdatedFdd()
{
	auto res = *m_hardwareP->Request(
		Hardware::Req::GET_FDD_IMAGE, { {"driveIdx", m_loadingRes.driveIdxUpdated} });
	auto data = res["data"];
	dev::SaveFile(m_loadingRes.pathFddUpdated, data);	
	m_hardwareP->Request(
			Hardware::Req::RESET_UPDATE_FDD, { {"driveIdx", m_loadingRes.driveIdxUpdated} });

	// check remaining updated fdds
	m_loadingRes.state = LoadingRes::State::CHECK_MOUNTED;
}


// Function to open a file dialog
bool OpenFileDialog(WCHAR* filePath, int size)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = filePath;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = size;
	ofn.lpstrFilter = L"All Files (*.rom, *.fdd)\0*.rom;*.fdd\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	// Display the open file dialog
	if (GetOpenFileName(&ofn) == TRUE)
		return true; // User selected a file
	else
		return false; // User cancelled or an error occurred
}

// Open the file dialog
void dev::DevectorApp::OpenFile()
{
	WCHAR path[MAX_PATH];

	if (OpenFileDialog(path, MAX_PATH))
	{
		auto ext = StrToUpper(dev::GetExt(path));

		if (ext == EXT_ROM) 
		{
			LoadRom(path);
			m_loadingRes.path = path;
			m_loadingRes.driveIdx = -1;
			m_loadingRes.autoBoot = false;
			m_loadingRes.state = LoadingRes::State::UPDATE_RECENT;
		}
		else if (ext == EXT_FDD)
		{
			m_loadingRes.path = path;
			m_loadingRes.state = LoadingRes::State::OPEN_POPUP_SELECT_DRIVE;
		}
		else {
			dev::Log(L"Not supported file type: {}", path);
			m_loadingRes.state = LoadingRes::State::NONE;
		}
	}
	else {
		m_loadingRes.state = LoadingRes::State::NONE;
	}
}

void dev::DevectorApp::DrawSelectDrivePopup()
{
	ImVec2 center = ImGui::GetMainViewport()->GetCenter(); 	// Always center this window when appearing
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(m_loadingRes.POPUP_SELECT_DRIVE, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Specify the drive to mount the FDD file as \nwell as the auto boot option if required.");
		ImGui::Separator();
		static int driveSelect = 0;
		ImGui::Combo("##DriveSelect", &driveSelect, "Drive A Boot\0Drive A\0Drive B\0Drive C\0Drive D\0");

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			m_loadingRes.autoBoot = (driveSelect == 0);
			m_loadingRes.driveIdx = dev::Max(driveSelect - 1, 0); // "0" and "1" are both associated with FDisk 0
			m_loadingRes.state = LoadingRes::State::LOAD;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) 
		{ 
			m_loadingRes.state = LoadingRes::State::NONE;
			ImGui::CloseCurrentPopup(); 
		}
		ImGui::EndPopup();
	}
}

void dev::DevectorApp::ReqUIHandling()
{
	switch (m_reqUI.type)
	{
	case ReqUI::Type::RELOAD_ROM_FDD:
		Reload();
		m_reqUI.type = ReqUI::Type::DISASM_UPDATE;
		break;
	}
}

void dev::DevectorApp::DebugAttach()
{
	bool requiresDebugger = m_disasmWindowVisible || m_breakpointsWindowVisisble || m_watchpointsWindowVisible ||
		m_hexViewerWindowVisible || m_traceLogWindowVisible || m_recorderWindowVisible;
	if (requiresDebugger != m_debuggerAttached)
	{
		m_debuggerAttached = requiresDebugger;

		if (requiresDebugger) {
			m_hardwareP->Request(Hardware::Req::DEBUG_RESET); // has to be called before enabling debugging, because Hardware state was changed
		}

		m_hardwareP->Request(Hardware::Req::DEBUG_ATTACH, { { "data", requiresDebugger } });
	}
}