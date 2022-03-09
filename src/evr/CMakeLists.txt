add_custom_command(
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/evrs.cc
                ${CMAKE_CURRENT_BINARY_DIR}/evrs.h
        COMMAND ${CMAKE_CURRENT_LIST_DIR}/../ac/ac_evrs.py
            ${CMAKE_CURRENT_LIST_DIR}/../cfg/evrs.xml
            ${CMAKE_CURRENT_BINARY_DIR}/evrs.cc
            ${CMAKE_CURRENT_BINARY_DIR}/evrs.h
        DEPENDS
            ${CMAKE_CURRENT_LIST_DIR}/../cfg/evrs.xml
            ${CMAKE_CURRENT_LIST_DIR}/../ac/ac_evrs.py
            ${CMAKE_CURRENT_LIST_DIR}/../ac/evr_cc.templ.txt
            ${CMAKE_CURRENT_LIST_DIR}/../ac/evr_h.templ.txt
        )

add_library(evr
        evr.cc
        ${CMAKE_CURRENT_BINARY_DIR}/evrs.cc
        ${CMAKE_CURRENT_BINARY_DIR}/evrs.h
        )
target_include_directories(evr PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(evr fw circle)
