#ifndef VO_CMPE460_STANDARDTYPES_HPP
#define VO_CMPE460_STANDARDTYPES_HPP

#include <stdint.h>

extern uint32_t strnlen(const char*, uint32_t);

#define FPRIME_OVERRIDE_NATIVE_TYPES
typedef int32_t NATIVE_INT_TYPE;
typedef uint32_t NATIVE_UINT_TYPE;

typedef int32_t TaskIdRepr;

#endif //VO_CMPE460_STANDARDTYPES_HPP
