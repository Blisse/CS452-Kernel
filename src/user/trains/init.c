#include <user/trains.h>

#include "clock.h"
#include "input_parser.h"
#include "performance.h"
#include "switch_server.h"
#include "train_server.h"

VOID
InitTrainTasks
    (
        VOID
    )
{
    TrainServerCreate();
    SwitchServerCreate();
    InputParserCreateTask();
    ClockCreateTask();
    PerformanceCreateTask();
}
