#pragma once

#include <CUESDK.h>
#include <mutex>
#include <thread>

class LightController {
	public:
	LightController();
	~LightController();

	/// <summary>
	/// Refreshes the Corsair handshake to refind devices. This can be useful if the program
	/// somehow didn't find the hardware.
	/// </summary>
	void Refresh();

	private:
	CorsairProtocolDetails details;
	std::thread cpuMemThread;
	std::thread timeThread;

	std::mutex cueLock;
	bool shuttingDown;

	void CPUMemLoop();
	void SetCPULights(double value);
	void SetMemLights(double value);

	void TimeLoop();
	void SetTimeLights(bool tickLight, tm* time);
};

