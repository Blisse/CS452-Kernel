#pragma once

#include "defs.h"
#include "limits.h"
#include "params.h"
#include "pointers.h"
#include "rtstatus.h"
#include "types.h"
#include "vararg.h"

#ifndef NLOCAL
#include <stdio.h>
#define PRINTF(format, ...) printf(format, __VA_ARGS__)
#else
#define PRINTF(format, ...)
#endif
