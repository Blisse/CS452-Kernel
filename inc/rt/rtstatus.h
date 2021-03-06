#pragma once

#include "types.h"

typedef INT RT_STATUS;

#define STATUS_SUCCESS 0
#define STATUS_FAILURE -1
#define STATUS_INVALID_PARAMETER -2
#define STATUS_BUFFER_OVERFLOW -3
#define STATUS_BUFFER_TOO_SMALL -4
#define STATUS_DEVICE_NOT_READY -5
#define STATUS_NOT_FOUND -6
#define STATUS_STACK_SPACE_OVERFLOW -7
#define STATUS_INVALID_STATE -8
#define STATUS_BUFFER_EMPTY -9

#define likely(x) (__builtin_expect(x, 1))
#define unlikely(x) (__builtin_expect(x, 0))

#define RT_SUCCESS(status) (likely(STATUS_SUCCESS == (status)))
#define RT_FAILURE(status) (STATUS_SUCCESS != (status))
