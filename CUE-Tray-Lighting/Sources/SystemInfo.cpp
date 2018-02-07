#include "SystemInfo.h"

#include <windows.h>
#include <tchar.h>
#include <pdh.h>

static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;

void SetupCPUReadings() {
	PdhOpenQuery(NULL, NULL, &cpuQuery);
	PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
	PdhCollectQueryData(cpuQuery);
}

double GetCurrentCPUPercentage() {
	PDH_FMT_COUNTERVALUE counterVal;

	PdhCollectQueryData(cpuQuery);
	PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
	return counterVal.doubleValue;
}

double GetCurrentMemPercentage() {
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	return static_cast<double>(memInfo.ullTotalPhys - memInfo.ullAvailPhys) / memInfo.ullTotalPhys * 100.0;
}