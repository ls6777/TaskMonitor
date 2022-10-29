# TaskMonitor
Embedded Task Monitor

## Overview
Task Monitor intended for embedded systems, including small micro controller environments running a lightweight RTOS (e.g. FreeRTOS). The Task monitor ensures that all other tasks are running as expected in the system. If there's a failure in one specific task, then the task monitor can take an action based on what the user would like to do.  Default is to ASSERT and allow the watchdog to lapse to reset the device.  However, it could easily be updated to restart the task in violation.

## Features
- Monitor any number of tasks
- Each task can have a unique timeout period
- Task Monitor will send out a message for a task to respond
- OR a task can send a message to the Task Monitor at anytime without have to respond to a specific message
- Task Monitor automatically determines which task is checking in (No need to track or send Task IDs)
- Watchdog only needs to be managed in a single place
- Ideal for event driven and message driven tasks
- Small footprint
- Can be used for very small embedded systems (e.g. FreeRTOS on 8-bit micro if desired)
- RTOS agnostic (Can be used with any RTOS with minor updates)
- Messages will only go out once per period for that task timeout (e.g. task timeout is 100ms, then messages sent out will happen at most once every 100ms, regardless of how quickly the response is received)
- Unsolicited messages received for task checkin will automatically reset the timing, so a message won't go out to a task unnecessarily

## Targets
I started this work on an STM32 board, but was required to switch to the ESP32 platform before completing the project.  I felt it was valuable to provide both as different examples of doing essentially the same thing.  The ESP32 is the most complete. However, this can be adapted to use with any target and RTOS.
