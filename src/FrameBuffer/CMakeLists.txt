set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/FrameBuffer.cc"
        "${CMAKE_CURRENT_LIST_DIR}/FrameBuffer.h"
)

set(MOD_DEPS Fw/Buffer)
register_fprime_module()
