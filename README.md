# Microcontroller Real Time Operating System

This operating system is designed for SINGLE CORE microcontrollers and uses the [fixed-priority preemptive scheduling algorithm](https://en.wikipedia.org/wiki/Fixed-priority_pre-emptive_scheduling). Stated briefly as follows,

* The scheduler (tries to) makes sure that it runs the highest priority ready thread.
* A higher priority thread will always preempt one with a lower priority outside a critical section. 
* If multiple highest-priority threads with the same priority exist, the CPU time will be shared between them. 
* If a higher priority thread is waiting for a resource that is currently unavailable, the operating system will put it to sleep and run other threads until the resource becomes available or specified time-out has been reached. In both cases the thread will be readied and a reschedule will be triggered immediately.

__This RTOS has not been fully evaluated for performance and reliability__. It is licensed under MIT license which is free for commercial and open/private uses and does not require source code disclosure (under certain conditions stated in LICENSE). Before planning to use it, please see LICENSE.

If you believe you have found a bug, please consider reporting it in [ISSUES](https://github.com/jdoe95/mrtos/issues) so that a fix can be worked out and the software can be made more robust, although this is not required. If you decided to, include the following information as much as possible, after making sure to protect your private code and information:

1. How to replicate the bug/how it is discovered
1. Version affected (tag, git-describe, or commit ID)
1. Version or versions known to be good (tag, git-describe, or commit ID)


## Features

### Multithreading

1. Dynamic thread creation and deletion
1. Static thread creation and deletion using existing buffer (as RTOS module)
1. Thread suspend/resume
1. Dynamic priority
1. Sleeping
1. Yielding
1. Critical section

### Dynamic memory

1. Dynamic memory allocation/deallocation using [Next Fit](https://www.geeksforgeeks.org/program-next-fit-algorithm-memory-management/)
1. Block/Pool/Thread memory statistics

### Inter-process communication

#### Queue
1. Dynamic creation and deletion
1. Static creation and deletion using existing buffer (as RTOS module)
1. Buffer reset (clear all data)
1. Peek/Nonblocking peek (reading data without affecting the queue )
1. Receive/Nonblocking receive (reading data)
1. Send/Nonblocking send
1. Send ahead/Nonblocking send ahead (sending high priority messages)

#### Mutex (recursive)
1. Dynamic creation and deletion
1. Static creation and deletion using existing buffer (as RTOS module)
1. Peek mutex without affecting the lock status
1. Lock/Nonblocking lock
1. Unlock
1. Lock status


#### Semaphore (multi-valued)
1. Dynamic creation and deletion
1. Static creation and deletion using existing buffer (as RTOS module)
1. Counter reset
1. Post operation
1. Wait/Nonblocking wait operations
1. Peek/Nonbloking peek without affecting semaphore status
 
## How to port
### Using supported platforms
If you are using one of the following platforms, clone the corresponding repository
into a folder called ``mrtos-portable``, under the same directory as ``mrtos``, using the following command:

```
git clone <repository> mrtos-portable
```

* [ARM Cortex M0, ARM Cortex M0+](https://github.com/jdoe95/mrtos-portable-cortexm0plus)


### Porting to unsupported platforms

Clone the mrtos source code under your project using

```bash
git@github.com:jdoe95/mrtos.git
```
Create another folder under the project named ``mrtos-portable``, and then create the following files
under that folder

```
rtos_portable.h
rtos_portable.c
```
where ``rtos_portable.h`` should contain the configurations and function prototypes and ``rtos_portable.c`` will contain function bodies. It is important that you add the path to folder ``mrtos_portable`` to the compiler include directory '``-Imrtos_portable``', so that the RTOS source code can find the portable header.

Define the following macros in ``rtos_portable.h``

* ``OSPORT_BYTE_T`` the byte type of the platform, usually ``uint8_t`` in ``stdint.h``
* ``OSPORT_UINT_T`` an unsigned integer type, used for array indexing, timestamp, and priority. Usually ``uint16_t`` or ``uint32_t`` in ``stdint.h``
* ``OSPORT_UINTPTR_T`` an unsigned integer type, guaranteed to be able to hold an address, used for ``os_handle_t``, usually uintptr_t in ``stdint.h``
* ``OSPORT_BOOL_T`` a boolean type, usually ``bool`` in ``stdbool.h``

* ``OSPORT_IDLE_STACK_SIZE`` the stack size of the idle thread, usually the size of exactly one full stack frame.

* ``OSPORT_NUM_PRIOS`` number of prorities. Use as few as possible. Usually 8. Prioirty 0 will be the highest priority and 6 will be the lowest priority. Priority 7 will be reserved for the idle thread. Do not create anything on this priority.

* ``OSPORT_MEM_ALIGN`` the largest memory pool alignment requirement. On some platforms, it is required for certain data types to be aligned to a certain memory address, and unaligned access can generate faults in the CPU or cause performance issues. For example, some platforms require that 8-byte data to be aligned to a 1-byte address boundary, 16-bit data and 32-bit data to be aligned to a 4-byte boundary. For this case, the value will be 4, because it will be the largest alignment requirement. The memory pool is also used to allocate process stacks. 

* ``OSPORT_MEM_SMALLEST`` The smallest memory (number of bytes) allocated to a thread at a time. To minimize fragmentation, the OS will always allocate more memory than this value to a thread.

* ``OSPORT_ENABLE_DEBUG`` Use 1 to enable the assertion macros. If you believe there's a bug in the operating system, turn this on to allow the OS to capture the bug before it causes a chain of errors.

* ``OSPORT_IDLE_FUNC`` The __function name__ of the idle function. It will be created as an idle thread. On most platforms this is simply a function that executes an empty, dead loop. Sometimes, it is desirable to put the CPU to sleep in the IDLE function, done by using platform-dependent methods.

* ``OSPORT_START()`` The function that clears the main stack context and sets up the CPU in a certain mode and loads the first thread.

* ``OSPORT_INIT_STACK()`` The function that initializes the process stack so that it contain the initial stack frame (called a fake context) to be loaded onto the CPU. 

* ``OSPORT_BREAKPOINT()`` The breakpoint function that halts the debugger. Only used when ``OSPORT_ENABLE_DEBUG``. If not defined, it will be replaced by a dead loop.

* ``OSPORT_DISABLE_INT()`` The function that disables the interrupt.

* ``OSPORT_ENABLE_INT()`` The function that enables the interrupt.

* ``OSPORT_CONTEXTSW_REQ()`` The function that generates a context switch request. Usually the context switcher is implemented as the lowest priority interrupt.

In ``rtos_portable.c`` you should have

1. Functions declared in ``rtos_portable.h``
1. A context switcher implemented as the lowest priority interrupt.
2. A timer interrupt that calls ``os_handle_heartbeat()`` periodically after the operating system starts.




