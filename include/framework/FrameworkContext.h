#pragma once

#include "framework/DataTypes.h"

#if defined(_WIN32)
  #if defined(DEVESCAPE_EXPORTS)
    #define DEVESCAPE_API __declspec(dllexport)
  #else
    #define DEVESCAPE_API __declspec(dllimport)
  #endif
#else
  #define DEVESCAPE_API
#endif

// Framework context is already defined in DataTypes.h
// This file is for future extensions
