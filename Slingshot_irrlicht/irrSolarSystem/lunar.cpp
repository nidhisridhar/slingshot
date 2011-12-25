#include <irrlicht.h>

using namespace irr;

const f64 rads = core::DEGTORAD64;
// ----------------------------------------------------------------------------------------------------------------------
// returns an angle in degrees in the range 0 to 360 */
f64 range(f64 x) 
{
	f64 a, b;
	b = x / 360;
	a = 360 * (b - floor(b));
	if (a < 0)
		a = 360 + a;
	return(a);
}

// ----------------------------------------------------------------------------------------------------------------------
// returns an angle in rads in the range 0 to two pi */
f64 rangerad(f64 x) 
{
	f64 a, b;
	b = x / (2.0*core::PI64);
	a = (2.0*core::PI64) * (b - floor(b));
	if (a < 0)
		a = (2.0*core::PI64) + a;
	return(a);
}
// ----------------------------------------------------------------------------------------------------------------------
/*
Author: Keith Burnett (kburnett@btinternet.com)
moonpos() takes days from J2000.0 and returns ecliptic coordinates
of moon in the pointers. Note call by reference.
This function is within a couple of arcminutes most of the time,
and is truncated from the Meeus Ch45 series, themselves truncations of
ELP-2000. Returns moon distance in earth radii.
Terms have been written out explicitly rather than using the
table based method as only a small number of terms is
retained.
The current moonpos() function is good to 2 arcmin 'most of the time', 
and 5 arcmin worst case (odd spikes in the error signal over a complete saros)
*/
void moonpos(f64 d, core::vector3df* pos, f64* lambda) 
{
	f64 beta; 
	f64 rvec;
	f64 dl, dB, dR, L, D, M, M1, F, e, lm, bm, rm, t;
	
	t = d / 36525;

	L = range(218.3164591  + 481267.88134236  * t) * rads;
	D = range(297.8502042  + 445267.1115168  * t) * rads;
	M = range(357.5291092  + 35999.0502909  * t) * rads;
	M1 = range(134.9634114  + 477198.8676313  * t - .008997 * t * t) * rads;
	F = range(93.27209929999999  + 483202.0175273  * t - .0034029  * t * t) * rads;
	e = 1 - .002516 * t;

	dl =      6288774 * sin(M1);
	dl +=     1274027 * sin(2 * D - M1);
	dl +=      658314 * sin(2 * D);
	dl +=      213618 * sin(2 * M1);
	dl -=  e * 185116 * sin(M);
	dl -=      114332 * sin(2 * F) ;
	dl +=       58793 * sin(2 * D - 2 * M1);
	dl +=   e * 57066 * sin(2 * D - M - M1) ;
	dl +=       53322 * sin(2 * D + M1);
	dl +=   e * 45758 * sin(2 * D - M);
	dl -=   e * 40923 * sin(M - M1);
	dl -=       34720 * sin(D) ;
	dl -=   e * 30383 * sin(M + M1) ;
	dl +=       15327 * sin(2 * D - 2 * F) ;
	dl -=       12528 * sin(M1 + 2 * F);
	dl +=       10980 * sin(M1 - 2 * F);
	lm = rangerad(L + dl / 1000000 * rads);

	dB =   5128122 * sin(F);
	dB +=   280602 * sin(M1 + F);
	dB +=   277693 * sin(M1 - F);
	dB +=   173237 * sin(2 * D - F);
	dB +=    55413 * sin(2 * D - M1 + F);
	dB +=    46271 * sin(2 * D - M1 - F);
	dB +=    32573 * sin(2 * D + F);
	dB +=    17198 * sin(2 * M1 + F);
	dB +=     9266 * sin(2 * D + M1 - F);
	dB +=     8822 * sin(2 * M1 - F);
	dB += e * 8216 * sin(2 * D - M - F);
	dB +=     4324 * sin(2 * D - 2 * M1 - F);
	bm = dB / 1000000 * rads;

	dR =    -20905355 * cos(M1);
	dR -=     3699111 * cos(2 * D - M1);
	dR -=     2955968 * cos(2 * D);
	dR -=      569925 * cos(2 * M1);
	dR +=   e * 48888 * cos(M);
	dR -=        3149 * cos(2 * F);
	dR +=      246158 * cos(2 * D - 2 * M1);
	dR -=  e * 152138 * cos(2 * D - M - M1) ;
	dR -=      170733 * cos(2 * D + M1);
	dR -=  e * 204586 * cos(2 * D - M);
	dR -=  e * 129620 * cos(M - M1);
	dR +=      108743 * cos(D);
	dR +=  e * 104755 * cos(M + M1);
	dR +=       79661 * cos(M1 - 2 * F);
	rm = 385000.56  + dR / 1000;

	*lambda = lm * (180. / (core::PI64));
	beta = bm * (180. / (core::PI64));
	rvec = rm;
	core::vector3df dir = core::vector3df(beta,*lambda,0).rotationToDirection();
	dir.setLength(rvec);
	*pos = core::vector3df(dir.Z,dir.Y,dir.X);
}