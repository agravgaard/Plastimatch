/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmsys_config.h"
#include "plm_endian.h"
#include "plm_int.h"

/* Switch buffer endian between big and little (2 byte types) */
static void
endian2_swap (void* buf, unsigned long len)
{
    uint8_t *cbuf = (uint8_t*) buf;

    for (unsigned long i = 0; i < len; i++) {
	uint8_t tmp;
	tmp = cbuf[2*i+0];
	cbuf[2*i+0] = cbuf[2*i+1];
	cbuf[2*i+1] = tmp;
    }
}

/* Switch buffer endian between big and little (4 byte types) */
static void
endian4_swap (void* buf, unsigned long len)
{
    uint8_t *cbuf = (uint8_t*) buf;

    for (unsigned long i = 0; i < len; i++) {
	uint8_t tmp[4];
	tmp[0] = cbuf[4*i+0];
	tmp[1] = cbuf[4*i+1];
	tmp[2] = cbuf[4*i+2];
	tmp[3] = cbuf[4*i+3];
	cbuf[4*i+0] = tmp[3];
	cbuf[4*i+1] = tmp[2];
	cbuf[4*i+2] = tmp[1];
	cbuf[4*i+3] = tmp[0];
    }
}

/* Switch buffer from big to native (2 byte types) */
void
endian2_big_to_native (void* buf, unsigned long len)
{
#if PLM_BIG_ENDIAN
    /* do nothing */
#else
    endian2_swap (buf, len);
#endif
}

/* Switch buffer from native to big (2 byte types) */
void
endian2_native_to_big (void* buf, unsigned long len)
{
#if PLM_BIG_ENDIAN
    /* do nothing */
#else
    endian2_swap (buf, len);
#endif
}

/* Switch buffer from little to native (2 byte types) */
void
endian2_little_to_native (void* buf, unsigned long len)
{
#if PLM_BIG_ENDIAN
    endian2_swap (buf, len);
#endif
}

/* Switch buffer from native to little (2 byte types) */
void
endian2_native_to_little (void* buf, unsigned long len)
{
#if PLM_BIG_ENDIAN
    endian2_swap (buf, len);
#endif
}

/* Switch buffer from big to native (4 byte types) */
void
endian4_big_to_native (void* buf, unsigned long len)
{
#if PLM_BIG_ENDIAN
    /* do nothing */
#else
    endian4_swap (buf, len);
#endif
}

/* Switch buffer from native to big (4 byte types) */
void
endian4_native_to_big (void* buf, unsigned long len)
{
#if PLM_BIG_ENDIAN
    /* do nothing */
#else
    endian4_swap (buf, len);
#endif
}

/* Switch buffer from little to native (4 byte types) */
void
endian4_little_to_native (void* buf, unsigned long len)
{
#if PLM_BIG_ENDIAN
    endian4_swap (buf, len);
#endif
}

/* Switch buffer from native to little (4 byte types) */
void
endian4_native_to_little (void* buf, unsigned long len)
{
#if PLM_BIG_ENDIAN
    endian4_swap (buf, len);
#endif
}
