#ifndef TOUCH_TASK_H
#define TOUCH_TASK_H

#include <Arduino.h>
#include "ClockTask.h"

class TouchTask {
public:
    static void start(ClockTask* clockInstance);
private:
    static void taskEntry(void* pvParameters);
};

#endif