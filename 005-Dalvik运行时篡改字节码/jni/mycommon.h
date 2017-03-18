#ifndef DALVIK_COMMON_H_
#define DALVIK_COMMON_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define RTLD_LAZY 1
static union { char c[4]; unsigned long mylong; }endian_test = {{ 'l', '?', '?', 'b' } };
#define ENDIANNESS  ((char)endian_test.mylong)

//#if ENDIANNESS == "l"
#define HAVE_LITTLE_ENDIAN
//#else
//#define HAVE_BIG_ENDIAN
//#endif

#if defined(HAVE_ENDIAN_H)
# include <endian.h>
#else /*not HAVE_ENDIAN_H*/
# define __BIG_ENDIAN 4321
# define __LITTLE_ENDIAN 1234
# if defined(HAVE_LITTLE_ENDIAN)
#  define __BYTE_ORDER __LITTLE_ENDIAN
# else
#  define __BYTE_ORDER __BIG_ENDIAN
# endif
#endif /*not HAVE_ENDIAN_H*/

#if !defined(NDEBUG) && defined(WITH_DALVIK_ASSERT)
# undef assert
# define assert(x) \
((x) ? ((void)0) : (ALOGE("ASSERT FAILED (%s:%d): %s", \
__FILE__, __LINE__, #x), *(int*)39=39, (void)0) )
#endif

#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))

#define LIKELY(exp) (__builtin_expect((exp) != 0, true))
#define UNLIKELY(exp) (__builtin_expect((exp) != 0, false))

#define ALIGN_UP(x, n) (((size_t)(x) + (n) - 1) & ~((n) - 1))
#define ALIGN_DOWN(x, n) ((size_t)(x) & -(n))
#define ALIGN_UP_TO_PAGE_SIZE(p) ALIGN_UP(p, SYSTEM_PAGE_SIZE)
#define ALIGN_DOWN_TO_PAGE_SIZE(p) ALIGN_DOWN(p, SYSTEM_PAGE_SIZE)

#define CLZ(x) __builtin_clz(x)

/*
 * If "very verbose" logging is enabled, make it equivalent to ALOGV.
 * Otherwise, make it disappear.
 *
 * Define this above the #include "Dalvik.h" to enable for only a
 * single file.
 */
/* #define VERY_VERBOSE_LOG */
#if defined(VERY_VERBOSE_LOG)
# define LOGVV  ALOGV
# define IF_LOGVV() IF_ALOGV()
#else
# define LOGVV(...) ((void)0)
# define IF_LOGVV() if (false)
#endif


/*
 * These match the definitions in the VM specification.
 */
typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;
typedef int8_t  s1;
typedef int16_t s2;
typedef int32_t s4;
typedef int64_t s8;

/*
 * Storage for primitive types and object references.
 *
 * Some parts of the code (notably object field access) assume that values
 * are "left aligned", i.e. given "JValue jv", "jv.i" and "*((s4*)&jv)"
 * yield the same result.  This seems to be guaranteed by gcc on big- and
 * little-endian systems.
 */

#define OFFSETOF_MEMBER(t, f) \
  (reinterpret_cast<char*>(   \
 &reinterpret_cast<t*>(16)->f) -  \
   reinterpret_cast<char*>(16))

#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

union JValue {
#if defined(HAVE_LITTLE_ENDIAN)
    u1  z;
    s1  b;
    u2  c;
    s2  s;
    s4  i;
    s8  j;
    float   f;
    double  d;
    void* l;
#endif
#if defined(HAVE_BIG_ENDIAN)
    struct {
        u1_z[3];
        u1z;
    };
    struct {
        s1_b[3];
        s1b;
    };
    struct {
        u2_c;
        u2c;
    };
    struct {
        s2_s;
        s2s;
    };
    s4  i;
    s8  j;
    float   f;
    double  d;
    void*   l;
#endif
};

/*
 * Array objects have these additional fields.
 *
 * We don't currently store the size of each element.  Usually it's implied
 * by the instruction.  If necessary, the width can be derived from
 * the first char of obj->clazz->descriptor.
 */
typedef struct   {
   void*clazz;
   u4  lock;
   u4  length;
   u1*  contents;
}ArrayObject ;

#endif  // DALVIK_COMMON_H_
