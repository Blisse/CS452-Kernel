#include <user/trains.h>

#include "clock.h"
#include "display.h"
#include "input_parser.h"
#include "location_server.h"
#include "performance.h"
#include "physics.h"
#include "scheduler.h"
#include "sensor_server.h"
#include "switch_server.h"
#include "train_server.h"

VOID
InitTrainTasks
    (
        VOID
    )
{
    // Initialize libraries
    PhysicsInit();
    TrackInit(TrackB);

    // Initialize tasks
    DisplayCreateTask();
    SchedulerCreateTask();
    SensorServerCreateTask();
    LocationServerCreateTask();
    TrainServerCreate();
    SwitchServerCreate();
    InputParserCreateTask();
    ClockCreateTask();
    PerformanceCreateTask();
}
