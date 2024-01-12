/* 
  NOTE: This is the interface between the editor and AutoDraw.
*/

#pragma once

#include "kv.h"

DLL_EXPORT b32 adMainFcoder(char *autodraw_path);

// NOTE: state
extern b32 ad_test_boolean;
extern gbMutex var_mutex;