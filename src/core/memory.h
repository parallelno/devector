#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <functional>
#include <mutex>
#include <string>
#include <format>

#include "utils/types.h"

namespace dev
{
	class Memory
	{
	public:
		#include "core/memory_consts.h"

		using Rom = std::vector<uint8_t>;
		using Ram = std::array<uint8_t, MEMORY_GLOBAL_LEN>;
		using RamDiskData = std::vector<uint8_t>;

#pragma pack(push, 1)
		// RAM-mapping is applied if the RAM-mapping is enabled, the ram accesssed via non-stack instructions, and the addr falls into the RAM-mapping range associated with that particular RAM mapping
		// Stack-mapping is applied if the Stack-mapping is enabled, the ram accesssed via the stack instructions (Push, Pop, XTHL, Call, Ret, C*, R*, RST)
		// special case: the RAM-mapping applies to a stack operation if the Stack-mapping is disabled, RAM-mapping is enabled, and the addr falls into the RAM-mapping range associated with that particular RAM mapping.
		union Mapping {
			struct {
				uint8_t pageRam : 2;	// Ram-Disk 64k page idx accesssed via non-stack instructions (all instructions except mentioned below)
				uint8_t pageStack : 2;	// Ram-Disk 64k page idx accesssed via the stack instructions (Push, Pop, XTHL, Call, Ret, C*, R*, RST)
				bool modeStack : 1;		// enabling stack mapping
				bool modeRamA : 1; // enabling ram [0xA000-0xDFFF] mapped into the the Ram-Disk
				bool modeRam8 : 1; // enabling ram [0x8000-0x9FFF] mapped into the the Ram-Disk
				bool modeRamE : 1; // enabling ram [0xE000-0xFFFF] mapped into the the Ram-Disk
			};
			uint8_t data = 0;
			Mapping(const uint8_t _data = 0) : data(_data) {}

			auto ToStr() const -> std::string
			{
				return std::format("mapping: ram mode:{}{}{}, stack mode:{}, ram page:{}, stack page:{}",
					modeRam8 ? "8" : "-",
					modeRamA ? "AC" : "--",
					modeRamE ? "E" : "-",
					modeStack ? "S" : "-",
					(int)pageRam, (int)pageStack);
			}
			auto RamModeToStr() const -> std::string
			{
				auto modeA = modeRamA ? "AC" : "--";
				auto mode8 = modeRam8 ? "8" : "-";
				auto modeE = modeRamE ? "E" : "-";
				return std::format("{}{}{}", mode8, modeA, modeE);
			}

		};
#pragma pack(pop)

#pragma pack(push, 1)
		Mapping m_mappings[RAM_DISK_MAX];
#pragma pack(pop)

		// contains the data stored in memory by the last executed instruction
#pragma pack(push, 1)
		struct Update
		{
			Mapping mapping;
			uint8_t ramdiskIdx : 3 = 0; // RAM_DISK_MAX = 8
			MemType memType : 1 = MemType::RAM;
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct Debug
		{
			GlobalAddr instrGlobalAddr;
			uint8_t instr[3];
			uint8_t instrLen = 0;
			GlobalAddr readGlobalAddr[2];
			uint8_t read[2];
			uint8_t readLen = 0;
			GlobalAddr writeGlobalAddr[2];
			uint8_t write[2];
			uint8_t writeLen = 0;
			uint8_t beforeWrite[2];
			inline void Init() { instrLen = readLen = writeLen= 0; }
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct State
		{
			Debug debug;
			Update update;
			Ram* ramP = nullptr;
		};
#pragma pack(pop)

		Memory(const std::string& _pathBootData, const std::string& _pathRamDiskData, const bool _ramDiskClearAfterRestart);
		~Memory();
		void Init();
		void Restart();
		auto GetByte(const Addr _addr,
			const Memory::AddrSpace _addrSpace = Memory::AddrSpace::RAM) -> uint8_t;
		auto CpuReadInstr(const Addr _addr,
			const Memory::AddrSpace _addrSpace = Memory::AddrSpace::RAM,
			const uint8_t _byteNum = 0) -> uint8_t;
		auto CpuRead(const Addr _addr,
			const Memory::AddrSpace _addrSpace = Memory::AddrSpace::RAM,
			const uint8_t _byteNum = 0) -> uint8_t;
		void CpuWrite(const Addr _addr, uint8_t _value,
			const Memory::AddrSpace _addrSpace, const uint8_t _byteNum);
		auto GetScreenBytes(Addr _screenAddrOffset) const -> uint32_t;
		auto GetRam() const -> const Ram*;
		auto GetGlobalAddr(const Addr _addr, const AddrSpace _addrSpace) const -> GlobalAddr;
		auto GetState() const -> const State& { return m_state; };
		auto GetStateP() -> State* { return &m_state; };
		auto GetMappingsP() const -> const Mapping* { return m_mappings; };
		void SetRamDiskMode(uint8_t _diskIdx, uint8_t _data);
		void SetMemType(const MemType _memType);
		void SetRam(const Addr _addr, const std::vector<uint8_t>& _data);
		void SetByteGlobal(const GlobalAddr _addr, const uint8_t _data);
		bool IsException();
		bool IsRomEnabled() const;
		inline void DebugInit() { m_state.debug.Init(); };

	private:

		Ram m_ram;
		Rom m_rom;
		State m_state;
		int m_mappingsEnabled = 0;
		std::string m_pathRamDiskData;
		bool m_ramDiskClearAfterRestart = true;
	};
}