#pragma once

/// <summary>
/// Must be called once before calling GetCurrentCPUPercentage().
/// </summary>
void SetupCPUReadings();

/// <summary>
/// Returns the current CPU usage as a percentage (between the range [0, 100]).
/// </summary>
double GetCurrentCPUPercentage();

/// <summary>
/// Returns the current physical memory usage as a percentage (between the range [0, 100]).
/// </summary>
double GetCurrentMemPercentage();