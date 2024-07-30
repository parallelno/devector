﻿#include "RecorderWindow.h"

#include <format>
#include "Utils/ImGuiUtils.h"
#include "Utils/StrUtils.h"

dev::RecorderWindow::RecorderWindow(Hardware& _hardware, Debugger& _debugger,
	const float* const _fontSizeP, const float* const _dpiScaleP,
	ReqUI& _reqUI)
	:
	BaseWindow("Recorder", DEFAULT_WINDOW_W, DEFAULT_WINDOW_H, _fontSizeP, _dpiScaleP),
	m_hardware(_hardware), m_debugger(_debugger), 
	m_reqUI(_reqUI)
{}

void dev::RecorderWindow::Update(bool& _visible)
{
	BaseWindow::Update();

	if (_visible && ImGui::Begin(m_name.c_str(), &_visible, ImGuiWindowFlags_NoCollapse))
	{
		bool isRunning = m_hardware.Request(Hardware::Req::IS_RUNNING)->at("isRunning");
		UpdateData(isRunning);
		Draw(isRunning);

		ImGui::End();
	}
}

void dev::RecorderWindow::Draw(const bool _isRunning)
{
	int stateRecorded = m_hardware.Request(Hardware::Req::DEBUG_RECORDER_GET_STATE_RECORDED)->at("states");
	int stateCurrent = m_hardware.Request(Hardware::Req::DEBUG_RECORDER_GET_STATE_CURRENT)->at("states");

	ImGui::Text("State current / recorded: %d / %d", stateCurrent, stateRecorded);

	ImGui::Separator();

	if (ImGui::Button(_isRunning ? "Break" : " Run "))
	{
		m_hardware.Request(_isRunning ? Hardware::Req::STOP : Hardware::Req::RUN);
	}

	if (_isRunning) ImGui::BeginDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Load"))
	{
		m_ccLast = -1;
	}
	ImGui::SameLine();
	if (ImGui::Button("Save"))
	{
		m_ccLast = -1;
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear"))
	{
		m_hardware.Request(Hardware::Req::DEBUG_RECORDER_RESET);
	}
	if (_isRunning) ImGui::EndDisabled();
	
	ImGui::SameLine();
	dev::DrawHelpMarker("Left Ctrl + R - reverse playback. Works runtime and while break\n"
						"Left Ctrl + F - forward playback. Works while break only");

	ImGui::Dummy({ 1, 5 });

	// Frame slider
	dev::PushStyleCompact(0.5f);
	int stateCurrentOld = stateCurrent;
	if (ImGui::SliderInt("##recTimeline", &stateCurrent, 1, stateRecorded, "%d", ImGuiSliderFlags_AlwaysClamp))
	{
		int diff = stateCurrent - stateCurrentOld;
		if (diff < 0) {
			m_hardware.Request(Hardware::Req::DEBUG_RECORDER_PLAY_REVERSE, { {"frames", abs(diff)} });
		}
		else {
			m_hardware.Request(Hardware::Req::DEBUG_RECORDER_PLAY_FORWARD, { {"frames", abs(diff)} });
		}
	}

	ImGui::SameLine();
	if (ImGui::Button(" - ") ||
		(ImGui::IsKeyPressed(ImGuiKey_R) && ImGui::IsKeyPressed(ImGuiKey_LeftCtrl)))
	{
		m_hardware.Request(Hardware::Req::DEBUG_RECORDER_PLAY_REVERSE, { {"frames", 10}});
	}

	ImGui::SameLine();
	if (ImGui::Button(" + "))
	{
		m_hardware.Request(Hardware::Req::DEBUG_RECORDER_PLAY_FORWARD, { {"frames", 10} });
	}
	dev::PopStyleCompact();
}

void dev::RecorderWindow::UpdateData(const bool _isRunning)
{
	/*
	// check if the hardware updated its state
	uint64_t cc = m_hardware.Request(Hardware::Req::GET_CC)->at("cc");
	auto ccDiff = cc - m_ccLast;
	//if (ccDiff == 0) return;
	m_ccLast = cc;

	*/
	// update
	
}