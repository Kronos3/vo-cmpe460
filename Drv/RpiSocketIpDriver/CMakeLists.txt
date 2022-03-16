####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
####
set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/RpiSocketIpDriverComponentImpl.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/RpiSocketHelper.cpp"
        )

set(MOD_DEPS
        Fw/Logger
        net)

register_fprime_module()
