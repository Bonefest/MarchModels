#pragma once

#include <nlohmann/json.hpp>

#include "common.h"

template <typename T, int32 size>
nlohmann::json vecToJson(const vec<T, size>& vect)
{
  nlohmann::json result;
  for(int32 i = 0; i < size; ++i)
  {
    result[std::to_string(i)] = vect[i];
  }

  return result;
}

template <typename T, int32 size>
vec<T, size> jsonToVec(const nlohmann::json& jsonData)
{
  vec<T, size> result;
  for(int32 i = 0; i < size; ++i)
  {
    result[i] = jsonData.value(std::to_string(i), T{});
  }

  return result;
}
