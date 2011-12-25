#include <irrlicht.h>

using namespace irr;

/*
//The universal gravitational constant (G), in units of (km^3 kg^-1 s^-2).
const f64 GconstKM = 6.67428e-20;  
//The universal gravitational constant (G), in units of (m^3 kg^-1 s^-2).
const f64 Gconst = 6.67428e-11; 
//The GM Product with the Sun (km^3 kg^-1 s^-2).
const f64 mu = GconstKM * 1989100000e21;
//The GM Product with the Sun (m^3 kg^-1 s^-2).
const f64 mu2 = Gconst * 1989100000e21; 

const f64 AU = 149598000;
*/
const f64 rads = core::DEGTORAD64;
const f64 degs = core::RADTODEG64;

f64 heliocentric_radius( f64 a, f64 e, f64 V);
f64 true_anomaly( f64 M, f64 e );
f64 mod2pi( f64 x );

void kepler(f64 ro[3], 
			f64 vo[3], 
			f64 dtseco, 
			f64 mu);

void rv2coe(f64 r[3], 
			f64 v[3],
			f64 mu,
			f64* p, 
			f64* a, 
			f64* ecc, 
			f64* incl, 
			f64* omega, 
			f64* argp,
			f64* nu,
			f64* m, 
			f64* arglat, 
			f64* truelon, 
			f64* lonper
 );
void Kepler2Cart(f64 r_v[],
				 f64 v_v[], 
				 f64 a, 
				 f64 e, 
				 f64 i,
				 f64 w, 
				 f64 O,
				 f64 f,
				 f64 mu
);

void COEstoRVmean(f64 RPQW[], 
				  f64 VPQW[], 
				  f64 semi, 
				  f64 e, 
				  f64 i, 
				  f64 node, 
				  f64 arg, 
				  f64 trueA, 
				  f64 MU
);