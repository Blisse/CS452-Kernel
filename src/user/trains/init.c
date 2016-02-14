#include <user/trains.h>

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
}
