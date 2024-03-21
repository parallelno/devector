#pragma once
#ifndef DEV_HARDWARE_H
#define DEV_HARDWARE_H

#include <mutex>
#include <string>

#include "Types.h"
#include "I8080.h"
#include "Memory.h"
#include "IO.h"
#include "Display.h"
#include "Utils/Utils.h"
#include "Utils/Result.h"
#include "Utils/TQueue.h"
#include "Utils/JsonUtils.h"
#include <thread>
#include <atomic>

namespace dev 
{
	class Hardware
	{
		I8080 m_cpu;
		Memory m_memory;
		IO m_io;
		Display m_display;

		enum class Status : int {
			RUN,
			STOP,
			EXIT
		};

	public:
		using CheckBreakFunc = std::function<bool(GlobalAddr)>;

		enum class Req: int {
			NONE = 0,
			SET_MEM,
			RUN,
			STOP,
			IS_RUNNING,
			EXIT,
			RESET,
			RESTART,
			EXECUTE_INSTR,
			EXECUTE_FRAME,
			GET_REGS,
			GET_REG_PC,
			GET_BYTE_RAM,
			GET_WORD_STACK,
			GET_DISPLAY_DATA,
			GET_MEMORY_MODES,
			GET_GLOBAL_ADDR_RAM,
		};

        Hardware();
		~Hardware();
		auto Request(const Req _req, const nlohmann::json& _dataJ = {}) -> Result <nlohmann::json>;
		auto GetFrame(const bool _vsync) -> const Display::FrameBuffer*;
		auto GetRam8K(const Addr _addr) -> const Memory::OutRam*;

		void AttachCheckBreak(CheckBreakFunc* _funcP);
		void AttachDebugOnReadInstr(I8080::DebugOnReadInstrFunc* _funcP);
		void AttachDebugOnRead(I8080::DebugOnReadFunc* _funcP);
		void AttachDebugOnWrite(I8080::DebugOnWriteFunc* _funcP);

	private:
		std::atomic <CheckBreakFunc*> m_checkBreak;
		std::thread m_executionThread;
		std::atomic<Status> m_status;
		TQueue <std::pair<Req, nlohmann::json>> m_reqs; // a request type
		TQueue <nlohmann::json> m_reqRes;				// it's a result of a request sent back 

		void Init();
		void Run();
		void ExecuteInstruction();
		void ReqHandling(const bool _waitReq = false);
		void Reset();
		void Restart();
		void SetMem(const nlohmann::json& _dataJ);
		auto GetRegs() const -> nlohmann::json;
		auto GetByte(const nlohmann::json _addr, const Memory::AddrSpace _addrSpace) -> nlohmann::json;
		auto GetWord(const nlohmann::json _addr, const Memory::AddrSpace _addrSpace) -> nlohmann::json;
	};
}
#endif // !DEV_HARDWARE_H