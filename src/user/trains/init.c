#include <user/trains.h>

#include "clock.h"
#include "display.h"
#include "input_parser.h"
#include "performance.h"
#include "sensor_server.h"
#include "switch_server.h"
#include "train_server.h"

VOID
InitTrainTasks
    (
        VOID
    )
{
    DisplayCreateTask();
    TrainServerCreate();
    SwitchServerCreate();
    SensorServerCreateTask();
    InputParserCreateTask();
    ClockCreateTask();
    PerformanceCreateTask();
}
