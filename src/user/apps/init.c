#include <user/users.h>

#include <rtosc/assert.h>
#include <rtkernel.h>

#include "calibration.h"

VOID
InitAppsTasks()
{
    CalibrationCreateTask();
}
