#include "MemViewerWindow.h"

#include "Utils/ImGuiUtils.h"
#include "imgui.h"

dev::MemViewerWindow::MemViewerWindow(Hardware& _hardware,
		const float* const _fontSizeP, const float* const _dpiScaleP)
	:
	BaseWindow(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H, _fontSizeP, _dpiScaleP),
	m_hardware(_hardware), m_ram()
{}

void dev::MemViewerWindow::Update()
{
	BaseWindow::Update();

	static bool open = true;
	ImGui::Begin("Memory Viewer", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar);

	bool isRunning = m_hardware.Request(Hardware::Req::IS_RUNNING)->at("isRunning");
	UpdateData(isRunning);

	DrawHex(isRunning);

	ImGui::End();
}

void dev::MemViewerWindow::UpdateData(const bool _isRunning)
{
	if (_isRunning) return;

	// check if the hardware updated its state
	auto res = m_hardware.Request(Hardware::Req::GET_REGS);
	const auto& data = *res;

	uint64_t cc = data["cc"];
	auto ccDiff = cc - m_ccLast;
	m_ccLastRun = ccDiff == 0 ? m_ccLastRun : ccDiff;
	m_ccLast = cc;
	if (ccDiff == 0) return;

	// update
	auto memP = m_hardware.GetRam()->data();
	std::copy(memP, memP + Memory::MEMORY_MAIN_LEN, m_ram.begin());
}

void dev::MemViewerWindow::DrawHex(const bool _isRunning)
{
	constexpr auto headerColumn = "00\0 01\0 02\0 03\0 04\0 05\0 06\0 07\0 08\0 09\0 0A\0 0B\0 0C\0 0D\0 0E\0 0F\0";
	
	const int COLUMNS_COUNT = 17;
	const char* tableName = "##MemViewer";

	static ImGuiTableFlags flags =
		ImGuiTableFlags_BordersOuter | 
		ImGuiTableFlags_Hideable;
	if (ImGui::BeginTable(tableName, COLUMNS_COUNT, flags))
	{
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 40);
		for (int column = 0; column < COLUMNS_COUNT - 1; column++)
		{
				ImGui::TableSetupColumn(headerColumn + column*4, ImGuiTableColumnFlags_WidthFixed, 18);
		}
		
		ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
		for (int column = 0; column < COLUMNS_COUNT; column++)
		{
			ImGui::TableSetColumnIndex(column);
			const char* column_name = ImGui::TableGetColumnName(column); // Retrieve the name passed to TableSetupColumn()
			if (column == 0) {
				dev::DrawHelpMarker(
					"Shows the current values of the memory data.\n\n"

					"Red highligh indicates recently written data\n"
					"Blue highligh indicates recently read data\n");
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Text, COLOR_ADDR);
				ImGui::TableHeader(column_name);
				ImGui::PopStyleColor();
			}
		}
		// addr & data
		int idx = 0;
		ImGuiListClipper clipper;
		clipper.Begin(int(m_ram.size()) / (COLUMNS_COUNT - 1));
		while (clipper.Step())
		{
			for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
			{
				ImGui::TableNextRow();
				// addr. left header 
				ImGui::TableNextColumn();
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, BG_COLOR_ADDR);
				ImGui::TextColored(COLOR_ADDR, std::format("{:04X}", row * (COLUMNS_COUNT - 1)).c_str());

				// the row of data
				if (_isRunning) ImGui::BeginDisabled();
				for (int col = 0; col < 16; col++)
				{
					ImGui::TableNextColumn();
					ImGui::TextColored(COLOR_VALUE, std::format("{:02X}", m_ram[row * (COLUMNS_COUNT - 1) + col]).c_str());
					idx++;
				}
				if (_isRunning) ImGui::EndDisabled();
			}
		}
		ImGui::EndTable();
	}
}