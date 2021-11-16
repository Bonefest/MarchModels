#pragma once

#include "defines.h"

#define ARRAY_SIZE(array) sizeof((array))/sizeof((array[0]))

// --- [Reverting byte order] -------------------------------------------------

// Helpers
template <typename T, uint32 Size = sizeof(T)>
union TypeToUnsignedInteger;

template <typename T>
union TypeToUnsignedInteger<T, 1>
{  
  T t;
  uint8 ui;
};

template <typename T>
union TypeToUnsignedInteger<T, 2>
{  
  T t;
  uint16 ui;
};

template <typename T>
union TypeToUnsignedInteger<T, 4>
{  
  T t;
  uint32 ui;
};

template <typename T>
union TypeToUnsignedInteger<T, 8>
{  
  T t;
  uint64 ui;
};


template <typename T>
TypeToUnsignedInteger<T> typeToUnsignedInteger(T data)
{
  TypeToUnsignedInteger<T> ttui;
  ttui.t = data;

  return ttui;
}

// Reverting function
template <typename T>
T revbytes(T data)
{
  TypeToUnsignedInteger ttui = typeToUnsignedInteger(data);
  ttui.ui = revbytes(ttui.ui);

  return ttui.t;
}

// Reverting function specializations
template <>
uint8 revbytes<uint8>(uint8 data)
{
  return data;
}

template <>
uint16 revbytes<uint16>(uint16 data)
{
  return (data >> 8) | (data << 8);
}

template <>
uint32 revbytes<uint32>(uint32 data)
{
  return (data >> 24) |
         ((data & 0x00FF0000) >> 8)  |
         ((data & 0x0000FF00) << 8)  |
         (data << 24);
}


template <>
uint64 revbytes<uint64>(uint64 data)
{
  return (data >> 56) |
         ((data & 0x00FF000000000000) >> 40) |
         ((data & 0x0000FF0000000000) >> 24) |
         ((data & 0x000000FF00000000) >> 8 ) |
         ((data & 0x00000000FF000000) << 8 ) |
         ((data & 0x0000000000FF0000) << 24) |
         ((data & 0x000000000000FF00) << 40) |
         (data << 56);
}

