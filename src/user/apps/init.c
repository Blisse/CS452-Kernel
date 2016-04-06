#include <user/users.h>

#include <rtosc/assert.h>
#include <rtkernel.h>

#include "final_demo.h"
#include "calibration.h"

VOID
InitAppsTasks()
{
    // CalibrationCreateTask();
    FinalDemoCreateTask();
}
