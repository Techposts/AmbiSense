#ifndef ASYNC_TCP_CONFIG_H
#define ASYNC_TCP_CONFIG_H

// Increase task stack size for AsyncTCP
#define CONFIG_ASYNC_TCP_TASK_STACK_SIZE 12288  // Increased from 8192

// Use higher priority for AsyncTCP task
#define CONFIG_ASYNC_TCP_TASK_PRIORITY 10  // Increased from 5

// Increase the number of allowed TCP connections
#define CONFIG_LWIP_MAX_ACTIVE_TCP 32  // Increased from 16

// Extend watchdog timeout for async tasks
#define CONFIG_ASYNC_TCP_USE_WDT 1
#define CONFIG_TASK_WDT_CHECK_IDLE_TASK_CPU0 0

// Additional performance and stability tweaks
#define CONFIG_LWIP_MAX_SOCKETS 32
#define CONFIG_LWIP_TCPIP_RECVMBOX_SIZE 32
#define CONFIG_LWIP_TCP_RECVMBOX_SIZE 32

#endif // ASYNC_TCP_CONFIG_H