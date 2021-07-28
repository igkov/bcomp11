#include "rev.h"

#if defined( CORTEX )
__asm void rev16(uint16_t *val)
{
	ldrh  r1, [r0]
	rev16 r1, r1
	strh  r1, [r0]
	bx    lr
}
#else
void rev16(uint16_t *val)
{
	uint8_t t;
	uint8_t *v = (uint8_t*)val;
	t    = v[0];
	v[0] = v[1];
	v[1] = t;
}
#endif

#if defined( CORTEX )
__asm void rev32(uint32_t *val)
{
	ldr   r1, [r0]
	rev   r1, r1
	str   r1, [r0]
	bx    lr
}
#else
void rev32(uint32_t *val)
{
	uint8_t t;
	uint8_t *v = (uint8_t*)val;
	t    = v[0];
	v[0] = v[1];
	v[1] = v[2];
	v[2] = v[3];
	v[3] = t;
}
#endif

