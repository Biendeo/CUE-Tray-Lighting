#include "LightController.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <list>
#include <numeric>

#include "SystemInfo.h"

// These are just some levels for percentages.
static const double low[] = {0.0, 0.0, 1.0};
static const double safe[] = {0.0, 1.0, 0.0};
static const double safeLevel = 50.0;
static const double warning[] = {1.0, 1.0, 0.0};
static const double warningLevel = 75.0;
static const double critical[] = {1.0, 0.0, 0.0};
static const double criticalLevel = 90.0;

LightController::LightController() {
	shuttingDown = false;

	cpuMemThread = std::thread(&LightController::CPUMemLoop, this);
	timeThread = std::thread(&LightController::TimeLoop, this);

	// Construction should perform a refresh.
	Refresh();
}

LightController::~LightController() {
	// This boolean signals to the cpuMemThread to end. Then we wait for it.
	shuttingDown = true;
	cpuMemThread.join();
}

void LightController::Refresh() {
	// The lock is here purely if the other thread will error at some point.
	cueLock.lock();
	details = CorsairPerformProtocolHandshake();
	cueLock.unlock();
}

void LightController::CPUMemLoop() {
	static const size_t readingCount = 10;
	static const auto updateDelay = std::chrono::milliseconds(100);

	// Our lighting updates will be the average CPU on the back row of the K95, and the average
	// memory on the MM800.
	std::list<double> cpuReadings;
	std::list<double> memReadings;
	for (size_t i = 0; i < readingCount; ++i) {
		cpuReadings.push_back(0.0);
		memReadings.push_back(0.0);
	}
	while (!shuttingDown) {
		cpuReadings.pop_front();
		cpuReadings.push_back(GetCurrentCPUPercentage());
		memReadings.pop_front();
		memReadings.push_back(GetCurrentMemPercentage());
		double cpuAverage = std::accumulate(cpuReadings.begin(), cpuReadings.end(), 0.0) / cpuReadings.size();
		double memAverage = std::accumulate(memReadings.begin(), memReadings.end(), 0.0) / memReadings.size();
		SetCPULights(cpuAverage);
		SetMemLights(memAverage);
		std::this_thread::sleep_for(updateDelay);
	}
}

void LightController::SetCPULights(double value) {
	static const int numLights = 19;
	static const double lightMultiplier = 255 * numLights / 100.0;
	const double* color;
	if (value >= criticalLevel) {
		color = critical;
	} else if (value >= warningLevel) {
		color = warning;
	} else if (value >= safeLevel) {
		color = safe;
	} else {
		color = low;
	}
	for (int id = CLKLP_Zone1; id <= CLKLP_Zone19; ++id) {
		double currentLightValue = std::max(0.0, std::min(100.0 / numLights, value));
		value -= 100.0 / numLights;
		auto ledColor = CorsairLedColor{static_cast<CorsairLedId>(id), static_cast<int>(color[0] * currentLightValue * lightMultiplier), static_cast<int>(color[1] * currentLightValue * lightMultiplier), static_cast<int>(color[2] * currentLightValue * lightMultiplier)};
		cueLock.lock();
		CorsairSetLedsColors(1, &ledColor);
		cueLock.unlock();
	}
}

void LightController::SetMemLights(double value) {
	static const int numLights = 15;
	static const double lightMultiplier = 255 * numLights / 100.0;
	const double* color;
	if (value >= criticalLevel) {
		color = critical;
	} else if (value >= warningLevel) {
		color = warning;
	} else if (value >= safeLevel) {
		color = safe;
	} else {
		color = low;
	}
	for (int id = CLMM_Zone1; id <= CLMM_Zone15; ++id) {
		double currentLightValue = std::max(0.0, std::min(100.0 / numLights, value));
		value -= 100.0 / numLights;
		auto ledColor = CorsairLedColor{static_cast<CorsairLedId>(id), static_cast<int>(color[0] * currentLightValue * lightMultiplier), static_cast<int>(color[1] * currentLightValue * lightMultiplier), static_cast<int>(color[2] * currentLightValue * lightMultiplier)};
		cueLock.lock();
		CorsairSetLedsColors(1, &ledColor);
		cueLock.unlock();
	}
}

void LightController::TimeLoop() {
	// I'll be updating this every 0.5 seconds, once with the escape light on, and one with it off.
	// The clock will update after two iterations to give the effect of a blinking light.
	static const auto blinkDelay = std::chrono::milliseconds(500);

	// First, wait until the next second.
	std::chrono::system_clock::time_point nextSecond = std::chrono::ceil<std::chrono::seconds>(std::chrono::system_clock::now());
	std::this_thread::sleep_until(nextSecond);

	bool blinkLight = true;

	while (!shuttingDown) {
		time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		SetTimeLights(blinkLight, localtime(&currentTime));
		blinkLight = !blinkLight;
		if (!blinkLight) {
			std::this_thread::sleep_for(blinkDelay);
		} else {
			// To be sure that this stays synced, we wait for the second to change the second time.
			std::chrono::system_clock::time_point nextSecond = std::chrono::ceil<std::chrono::seconds>(std::chrono::system_clock::now());
			std::this_thread::sleep_until(nextSecond);
		}
	}
}

void LightController::SetTimeLights(bool blinkLight, tm* time) {
	static const double color1[] = {1.0, 0.0, 0.0};
	static const double color2[] = {1.0, 1.0, 0.0};
	static const double color3[] = {0.0, 1.0, 0.0};
	static const double color4[] = {0.0, 1.0, 1.0};
	static const double color5[] = {0.0, 0.0, 1.0};
	static const double colorOff[] = {0.0, 0.0, 0.0};
	// Since F12 is inconveniently not next to F11 in terms of enum values, I'm manually making sure
	// that I can iterate through them.
	static const CorsairLedId fKeys[] = {CLK_F1, CLK_F2, CLK_F3, CLK_F4, CLK_F5, CLK_F6, CLK_F7, CLK_F8, CLK_F9, CLK_F10, CLK_F11, CLK_F12};

	const double* mainColor;
	const double* backColor;
	//? This can definitely be a bit neater.
	if (time->tm_sec >= 48) {
		mainColor = color5;
		backColor = color4;
	} else if (time->tm_sec >= 36) {
		mainColor = color4;
		backColor = color3;
	} else if (time->tm_sec >= 24) {
		mainColor = color3;
		backColor = color2;
	} else if (time->tm_sec >= 12) {
		mainColor = color2;
		backColor = color1;
	} else {
		mainColor = color1;
		backColor = color5;
	}

	if (blinkLight) {
		auto ledColor = CorsairLedColor{CLK_Escape, static_cast<int>(mainColor[0] * 255), static_cast<int>(mainColor[1] * 255), static_cast<int>(mainColor[2] * 255)};
		CorsairSetLedsColors(1, &ledColor);
	} else {
		auto ledColor = CorsairLedColor{CLK_Escape, 0, 0, 0};
		CorsairSetLedsColors(1, &ledColor);
	}

	int currentKey = time->tm_sec % 12;
	if (currentKey == 0) {
		auto ledColor = CorsairLedColor{fKeys[(currentKey + 11) % 12], static_cast<int>(backColor[0] * 255), static_cast<int>(backColor[1] * 255), static_cast<int>(backColor[2] * 255)};
		CorsairSetLedsColors(1, &ledColor);
	} else {
		auto ledColor = CorsairLedColor{fKeys[(currentKey + 11) % 12], static_cast<int>(mainColor[0] * 255), static_cast<int>(mainColor[1] * 255), static_cast<int>(mainColor[2] * 255)};
		CorsairSetLedsColors(1, &ledColor);
	}
}