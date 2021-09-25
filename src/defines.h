#pragma once

#include <cassert>
#include <cstdint>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

/**
 * Basic types
 */

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using bool8 = uint8_t;
using bool32 = uint32_t;

using float32 = float;
using float64 = double;

#define TRUE 1
#define FALSE 0

#define ENGINE_API
