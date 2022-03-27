set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/syscalls.cc"

        "${CMAKE_CURRENT_LIST_DIR}/File.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/FileSystem.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/IntervalTimer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Mutex.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Queue.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/Task.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/RPIQueue.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/RPIFs.cpp"

        "${PROJECT_SOURCE_DIR}/fprime/Os/IntervalTimerCommon.cpp"
        "${PROJECT_SOURCE_DIR}/fprime/Os/TaskString.cpp"
        "${PROJECT_SOURCE_DIR}/fprime/Os/TaskCommon.cpp"
        "${PROJECT_SOURCE_DIR}/fprime/Os/QueueCommon.cpp"
        "${PROJECT_SOURCE_DIR}/fprime/Os/QueueString.cpp"
        "${PROJECT_SOURCE_DIR}/fprime/Os/IPCQueueCommon.cpp"
        "${PROJECT_SOURCE_DIR}/fprime/Os/SimpleQueueRegistry.cpp"
        "${PROJECT_SOURCE_DIR}/fprime/Os/MemCommon.cpp"
        "${PROJECT_SOURCE_DIR}/fprime/Os/ValidateFileCommon.cpp"
        "${PROJECT_SOURCE_DIR}/fprime/Os/ValidatedFile.cpp"
        "${PROJECT_SOURCE_DIR}/fprime/Os/FileCommon.cpp"
        )

set(MOD_DEPS
        circle
        sched
        Fw/Cfg
        Fw/Types
        Fw/Logger
        Utils/Hash)

register_fprime_module()

add_library(libc_stubs STATIC
        "${CMAKE_CURRENT_LIST_DIR}/libc.cc"
)
