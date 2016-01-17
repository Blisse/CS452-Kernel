#pragma once

#include "types.h"

#define offset_of(type, member) ((UINT) &(((type*)0)->member))
#define container_of(ptr, type, member) ((type*) (((CHAR*) ptr) - offset_of(type, member)))
#define ptr_add(ptr, bytes) ((PVOID) (((CHAR*) (ptr)) + (bytes)))
