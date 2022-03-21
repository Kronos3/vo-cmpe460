# Bare Metal
The bare metal kernel image support will include
Circle+ open-source drivers for Raspberry Pi.
FPrime drivers are written to integrate with
this library set.

Deficiencies:
    -  Scheduler will run in on a CPU0 (all tasks run on this core)
    -  All IRQ handling is on CPU0
    -  No tasks preemption or priority level
        - This means long-running tasks that do not yield() will lock the scheduler
    -  No linux kernel modules or libraries
    -  FatFS is the only supported filesystem (1 partition only)
    -  

Advantages:
    - Tiny kernel image
        - FPrime + Circle+ + FSW + OpenCV all link to about 1.2M (with debugging symbols)
    - Microcontroller level latency <50us
    - 

# Realtime Linux
