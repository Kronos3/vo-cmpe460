####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
# Note: using PROJECT_NAME as EXECUTABLE_NAME
####
set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/TestCam.cc"
        #        "${CMAKE_CURRENT_LIST_DIR}/backtrace.c"
        )
# Note: supply non-explicit dependencies here
set(MOD_DEPS

        )



set(EXECUTABLE_NAME TestCam)
register_fprime_executable()

target_link_libraries(TestCam_exe PUBLIC pthread)
