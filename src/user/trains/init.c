#include <rtosc/assert.h>
#include <rtos.h>
#include <user/trains.h>

#include "conductor.h"
#include "location_server.h"
#include "physics.h"
#include "scheduler.h"
#include "sensor_server.h"
#include "switch_server.h"
#include "track_server.h"
#include "track_reserver.h"
#include "train_server.h"

VOID
InitTrainTasks()
{
    // Initialize libraries
    PhysicsInit();

    // Setup the track
    TrackServerCreate();
    TrainServerCreate();
    SwitchServerCreate();
    TrackReserverCreate();

    // Initialize remaining tasks
    SensorServerCreateTask();
    LocationServerCreateTask();
    ConductorServerCreateTask();
    SchedulerCreateTask();
}
