#ifndef ASYNC_TCP_CONFIG_H
#define ASYNC_TCP_CONFIG_H

// This file needs to be placed in the same directory as AsyncTCP.h
// or in a location where the compiler can find it when AsyncTCP.h is included

// Increase task stack size for AsyncTCP
#define CONFIG_ASYNC_TCP_TASK_STACK_SIZE 8192 

// Use higher priority for AsyncTCP task
#define CONFIG_ASYNC_TCP_TASK_PRIORITY 5

// Increase the number of allowed TCP connections
#define CONFIG_LWIP_MAX_ACTIVE_TCP 16

// Other potentially relevant options
#define CONFIG_ASYNC_TCP_USE_WDT 1

#endif // ASYNC_TCP_CONFIG_H