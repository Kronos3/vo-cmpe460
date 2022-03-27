#ifndef VO_CMPE460_STANDARDTYPES_HPP
#define VO_CMPE460_STANDARDTYPES_HPP

#ifdef __circle__
#define _GCC_STDINT_H
#define _SYS__STDINT_H
#include <stdint.h>

typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;

typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

typedef int64_t intmax_t;
typedef uint64_t uintmax_t;

static_assert(sizeof(int64_t) == 8, "int64_t size");
static_assert(sizeof(uint64_t) == 8, "int64_t size");

extern uint32_t strnlen(const char*, uint32_t);

#define FPRIME_OVERRIDE_NATIVE_TYPES
typedef int32_t NATIVE_INT_TYPE;
typedef uint32_t NATIVE_UINT_TYPE;

typedef int32_t TaskIdRepr;

#else
#include <stdint.h>
#endif

#endif //VO_CMPE460_STANDARDTYPES_HPP
