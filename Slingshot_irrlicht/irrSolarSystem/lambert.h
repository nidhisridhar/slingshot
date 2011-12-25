#include <irrlicht.h>

using namespace irr;

void LambertCompute(f64 GM, 
					f64 r0[],
					f64 v0[],
					f64 rf[],
					f64 vf[], 
					f64 dt,
					f64 deltaV0[],
					f64 deltaV1[],
					f64 totalV0[],
					f64 totalV1[]);