#ifndef FIXED_H
#define FIXED_H
#include <gba_types.h>

typedef s32 FIXED;
#define FIX_SHIFT 8
#define FIX_SCALE 256

inline FIXED int2fx(int d) {
	return d<<FIX_SHIFT;
}

inline int fx2int(FIXED fx) {
	return fx/FIX_SCALE;
}

inline FIXED fxadd(FIXED fa, FIXED fb) {
	return fa + fb;
}

inline FIXED fxsub(FIXED fa, FIXED fb) {
	return fa - fb;
}

inline FIXED fxmul(FIXED fa, FIXED fb) {
    return (fa*fb)>>FIX_SHIFT;
}

inline FIXED fxdiv(FIXED fa, FIXED fb) {
    return ((fa)*FIX_SCALE) / fb;
}

inline int intfxround(FIXED fx) {
	return (fx+128)/FIX_SCALE; // + ((fx & 0x000000FF) > 128) ? 1 : 0;

}

#endif