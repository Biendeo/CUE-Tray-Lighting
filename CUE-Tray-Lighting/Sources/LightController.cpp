#include "LightController.h"

#include <algorithm>
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

	updateThread = std::thread(&LightController::UpdateLoop, this);

	// Construction should perform a refresh.
	Refresh();
}

LightController::~LightController() {
	// This boolean signals to the updateThread to end. Then we wait for it.
	shuttingDown = true;
	updateThread.join();
}

void LightController::Refresh() {
	// The lock is here purely if the other thread will error at some point.
	cueLock.lock();
	details = CorsairPerformProtocolHandshake();
	cueLock.unlock();
}

void LightController::UpdateLoop() {
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
		CorsairSetLedsColors(1, &ledColor);
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
		CorsairSetLedsColors(1, &ledColor);
	}
}