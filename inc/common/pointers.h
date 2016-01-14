#pragma once

#define offset_of(type, member) ((UINT) &(((type*)0)->member))
#define container_of(ptr, type, member) ((type*) (((STRING) ptr) - offset_of(type, member)))
#define ptr_add(ptr, bytes) (((PVOID) (ptr)) + bytes)
