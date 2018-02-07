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
	std::thread updateThread;

	std::mutex cueLock;
	bool shuttingDown;

	void UpdateLoop();
	void SetCPULights(double value);
	void SetMemLights(double value);
};

