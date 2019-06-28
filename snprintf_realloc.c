#include <stdarg.h>
#include <stdlib.h>

/**
 * Call <tt>snprintf(*str, *size, format, ...)</tt> and reallocate the buffer
 * pointed to by <tt>*str</tt> as appropriate to contain the entire result.  On
 * exit, <tt>*str</tt> and <tt>*size</tt> will contain a pointer to the
 * <tt>realloc</tt>ed buffer and its maximum usable size, respectively.
 *
 * The reallocation scheme attempts to reduce the reallocation calls when the
 * same <tt>str</tt> and <tt>size</tt> arguments are used repeatedly.  It is
 * valid to pass <tt>*str == NULL</tt> and <tt>*size == 0</tt> and then have
 * the buffer allocated to perfectly fit the result.
 *
 * @param[in,out] str    Pointer to the buffer in which to write the result.
 * @param[in,out] size   Pointer to the initial buffer size.
 * @param[in]     format Format specifier to use in <tt>sprintf</tt> call.
 * @param[in]     ...    Variable number of arguments corresponding
 *                       to \c format.
 *
 * @return On success, the number of characters (not including the trailing
 *         '\0') written to <tt>*str</tt>.  On error, a negative value
 *         is returned, <tt>*str</tt> is <tt>free</tt>d and <tt>*size</tt>
 *         is set to zero.
 */
int snprintf_realloc(char **str, size_t *size, const char *format, ...);

int
snprintf_realloc(char **str, size_t *size, const char *format, ...)
{
    int retval, needed;
    va_list ap;
    va_start(ap, format);
    while (   0     <= (retval = vsnprintf(*str, *size, format, ap)) // Error?
           && *size <  (needed = retval + 1)) {                      // Space?
        va_end(ap);
        *size *= 2;                                                  // Space?
        if (*size < needed) *size = needed;                          // Space!
        char *p = realloc(*str, *size);                              // Alloc
        if (p) {
            *str = p;
        } else {
            free(*str);
            *str  = NULL;
            *size = 0;
            return -1;
        }
        va_start(ap, format);
    }
    va_end(ap);
    return retval;
}

// From https://github.com/imb/fctx
#include "fct.h"

FCT_BGN()
{
    FCT_FIXTURE_SUITE_BGN(snprintf_realloc)
    {
        const char s[] = "1234";
        char *ptr;
        size_t size;

        FCT_SETUP_BGN()
        {
            ptr  = NULL;
            size = 0;
        }
        FCT_SETUP_END();

        FCT_TEARDOWN_BGN()
        {
            if (ptr) free(ptr);
        }
        FCT_TEARDOWN_END();

        FCT_TEST_BGN(snprintf_realloc)
        {
            // Initial should cause malloc-like behavior
            fct_chk_eq_int(4, snprintf_realloc(&ptr, &size, "%s", s));
            fct_chk(ptr);
            fct_chk_eq_int(size, 5);
            fct_chk_eq_str(ptr, s);

            // Repeated size should not cause any new buffer allocation
            {
                char *last_ptr = ptr;
                fct_chk_eq_int(4, snprintf_realloc(&ptr, &size, "%s", s));
                fct_chk(last_ptr == ptr);
                fct_chk_eq_int(size, 5);
                fct_chk_eq_str(ptr, s);
            }

            // Request requiring more than twice the space should
            // realloc memory to fit exactly.
            fct_chk_eq_int(12, snprintf_realloc(&ptr, &size, "%s%s%s",
                                                s, s, s));
            fct_chk_eq_int(size, 13);
            fct_chk_eq_str(ptr, "123412341234");

            // Request requiring less than twice the space should
            // cause a doubling of the buffer size.
            fct_chk_eq_int(16, snprintf_realloc(&ptr, &size, "%s%s%s%s",
                                                s, s, s, s));
            fct_chk_eq_int(size, 26);
            fct_chk_eq_str(ptr, "1234123412341234");
        }
        FCT_TEST_END();
    }
    FCT_FIXTURE_SUITE_END();
}
FCT_END()
