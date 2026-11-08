#pragma once

/// @file Utils.hpp
/// @brief Small helpers shared by the task monitor and the example tasks.

#include "TargetPort.hpp"

#include <cstdint>

/// @brief Block the calling task for a number of milliseconds
/// @param x - time in milliseconds
#define DELAY_MS(x) DELAY(MS_TO_TICKS(x))

static constexpr uint32_t MS_PER_SEC = 1000;    ///< Milliseconds per second
