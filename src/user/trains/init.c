#include <rtosc/assert.h>
#include <rtos.h>
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

    // Setup the display
    DisplayCreateTask();
    ClockCreateTask();
    PerformanceCreateTask();
    InputParserCreateTask();

    Log("Initializing, please wait (10s)");

    // Setup the track
    TrainServerCreate();
    SwitchServerCreate();

    Log("Waiting for junk sensor data");

    // Wait 10 seconds for junk sensor data to arrive, then flush the junk data
    IO_DEVICE com1Device;
    VERIFY(SUCCESSFUL(Open(UartDevice, ChannelCom1, &com1Device)));
    VERIFY(SUCCESSFUL(Delay(900)));
    VERIFY(SUCCESSFUL(FlushInput(&com1Device)));

    Log("Flushed junk sensor data");

    // Initialize remaining tasks
    SchedulerCreateTask();
    SensorServerCreateTask();
    LocationServerCreateTask();    

    Log("Initialization complete");
}
