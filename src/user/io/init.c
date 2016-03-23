#include <user/io.h>

#include "clock.h"
#include "display.h"
#include "input_parser.h"
#include "performance.h"

VOID
InitIoTasks()
{
    DisplayCreateTask();
    ClockCreateTask();
    PerformanceCreateTask();
    InputParserCreateTask();
}
