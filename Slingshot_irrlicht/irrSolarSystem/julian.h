/*

Astronomical calculation routines

*/

#include <irrlicht.h>
using namespace irr;

f64 ucttoj(s32 year, s32 mon, s32 mday, s32 hour, s32 min, s32 sec);
void jyear(f64 td, s32 *yy, s32 *mm, s32 *dd);
void jhms(f64 j, s32 *h, s32 *m, s32 *s);
s32 jwday(f64 j);
f64 delT(f64 jd);