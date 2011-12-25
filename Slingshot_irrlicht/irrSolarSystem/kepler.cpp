#include <irrlicht.h>
#include "doubleVectorOps.h"
#include "GTOPtoolbox/propagateKEP.h"

using namespace irr;

f64 infinite = 9e99;
f64 undefined = 0e-99;


f64 sgn(f64 x)
{
	if (x < 0.0)
	{
		return -1.0;
	}
	else
	{
		return 1.0;
	}

}  // end sgn

f64 cot(f64 xval)
{
	f64 temp;

	temp = tan( xval );
	if (fabs( temp ) < 0.00000001 )
		return infinite;
	else
		return 1.0 / temp;
}  // procedure cot
// return an angle in the range 0 to 2pi radians
f64 mod2pi( f64 x )
{
	f64 b = x / (2*core::PI64);
	f64 a = (2*core::PI64)*( b - abs(floor(b)) );
	if (a < 0){
		a = (2*core::PI64) + a;
	}
	return a;
}
// ----------------------------------------------------------------------------------------------------------------------
// compute the true anomaly from mean anomaly using iteration
//  M - mean anomaly in radians
//  e - orbit eccentricity
f64 true_anomaly( f64 M, f64 e )
{
	f64 V;
	f64 E;
	f64 E1;
	f64 eps  = 1e-12;

	// initial approximation of eccentric anomaly
	E = M + e * sin(M)*(1.0 + e* cos(M));
	// iterate to improve accuracy

	do
	{
		E1 = E;
		E = E1 - (E1 - e* sin(E1) - M)/((f64)1.0 - e* cos(E1));
		//printf("%.9f > %.9f\n",abs(E - E1),eps);
	}
	while (abs( E - E1 ) > eps);

	// convert eccentric anomaly to true anomaly
	V = 2.0*atan(sqrt((1.0 + e)/(1.0 - e))* tan(0.5*E));

	if (V < 0) {
		// modulo 2pi
		V = V + (2*core::PI64);
	}

	return V;
}


// ----------------------------------------------------------------------------------------------------------------------
// compute helio-centric-radius
f64 heliocentric_radius( f64 a, f64 e, f64 V)
{
	f64 HelioRadius;
	HelioRadius = a * (1.0 - pow(e,2)) / (1.0 + e * cos(V));
	//printf("rSolar = %f\n",HelioRadius);
	return HelioRadius;
}
// ----------------------------------------------------------------------------------------------------------------------
/*
*
*                           function newtonnu
*
*  this function solves keplers equation when the true anomaly is known.
*    the mean and eccentric, parabolic, or hyperbolic anomaly is also found.
*    the parabolic limit at 168ø is arbitrary. the hyperbolic anomaly is also
*    limited. the hyperbolic sine is used because it's not f64 valued.
*
*  author        : david vallado                  719-573-2600   27 may 2002
*
*  revisions
*    vallado     - fix small                                     24 sep 2002
*
*  inputs          description                    range / units
*    ecc         - eccentricity                   0.0  to
*    nu          - true anomaly                   -2pi to 2pi rad
*
*  outputs       :
*    e0          - eccentric anomaly              0.0  to 2pi rad       153.02 ø
*    m           - mean anomaly                   0.0  to 2pi rad       151.7425 ø
*
*  locals        :
*    e1          - eccentric anomaly, next value  rad
*    sine        - sine of e
*    cose        - cosine of e
*    ktr         - index
*
*  coupling      :
*    arcsinh     - arc hyperbolic sine
*    sinh        - hyperbolic sine
*
*  references    :
*    vallado       2007, 85, alg 5
* --------------------------------------------------------------------------- */

void newtonnu(f64 ecc, f64 nu,f64& e0, f64& m)
{
	f64 small, sine, cose;

	// ---------------------  implementation   ---------------------
	e0= 999999.9;
	m = 999999.9;
	small = 0.00000001;

	// --------------------------- circular ------------------------
	if ( fabs( ecc ) < small  )
	{
		m = nu;
		e0= nu;
	}
	else
		// ---------------------- elliptical -----------------------
		if ( ecc < 1.0-small  )
		{
			sine= ( sqrt( 1.0 -ecc*ecc ) * sin(nu) ) / ( 1.0 +ecc*cos(nu) );
			cose= ( ecc + cos(nu) ) / ( 1.0  + ecc*cos(nu) );
			e0  = atan2( sine,cose );
			m   = e0 - ecc*sin(e0);
		}
		else
			// -------------------- hyperbolic  --------------------
			if ( ecc > 1.0 + small  )
			{
				if ((ecc > 1.0 ) && (fabs(nu)+0.00001 < core::PI64-acos(1.0 /ecc)))
				{
					sine= ( sqrt( ecc*ecc-1.0  ) * sin(nu) ) / ( 1.0  + ecc*cos(nu) );
					//e0  = asinh( sine );
					e0 = log(sine + sqrt(sine * sine + 1.0));
					m   = ecc*sinh(e0) - e0;
				}
			}
			else
				// ----------------- parabolic ---------------------
				if ( fabs(nu) < 168.0*core::PI64/180.0  )
				{
					e0= tan( nu*0.5  );
					m = e0 + (e0*e0*e0)/3.0;
				}

				if ( ecc < 1.0  )
				{
					m = fmod( m,2.0 *core::PI64 );
					if ( m < 0.0  )
						m= m + 2.0 *core::PI64;
					e0 = fmod( e0,2.0 *core::PI64 );
				}
}  // procedure newtonnu

/* -----------------------------------------------------------------------------
**
*                           procedure angle
*
*  this procedure calculates the angle between two vectors.  the output is
*    set to 999999.1 to indicate an undefined value.  be sure to check for
*    this at the output phase.
*
*  author        : david vallado                  719-573-2600    1 mar 2001
*
*  inputs          description                    range / units
*    vec1        - vector number 1
*    vec2        - vector number 2
*
*  outputs       :
*    theta       - angle between the two vectors  -core::PI64 to core::PI64
*
*  locals        :
*    temp        - temporary real variable
*    magv1       - magnitude of vec1
*    magv2       - magnitude of vec2
*    small       - value defining a small value
*    undefined   - large number to use in place of a not defined number
*
*  coupling      :
*    dot           dot product of two vectors
*    acos          arc cosine function
*    mag           magnitude of a vector
*
*/
// ----------------------------------------------------------------------------------------------------------------------

f64  angle(f64 vec1[3], f64 vec2[3])
{
	f64 small, magv1, magv2, temp;
	small     = 0.00000001;

	magv1 = mag(vec1);
	magv2 = mag(vec2);

	if (magv1*magv2 > small*small)
	{
		temp= dot(vec1,vec2) / (magv1*magv2);
		if (fabs( temp ) > 1.0)
		{
			temp= sgn(temp) * 1.0;
			return acos( temp );
		}
	}
	else
	{
		return undefined;
	}
}
/* ----------------------------------------------------------------------------------------------------------------------
*
*                           function findc2c3
*
*  this function calculates the c2 and c3 functions for use in the universal
*    variable calculation of z.
*
*  author        : david vallado                  719-573-2600   27 may 2002
*
*  revisions
*                -
*
*  inputs          description                    range / units
*    znew        - z variable                     rad2
*
*  outputs       :
*    c2new       - c2 function value
*    c3new       - c3 function value
*
*  locals        :
*    sqrtz       - square root of znew
*
*  coupling      :
*    sinh        - hyperbolic sine
*    cosh        - hyperbolic cosine
*
*  references    :
*    vallado       2007, 71, alg 1
*/
// ----------------------------------------------------------------------------------------------------------------------
void findc2c3(f64 ZNew, f64* C2New, f64* C3New)
{
	const f64 Small = 0.00000001;       // Small value for tolerances
	f64 SqrtZ;

	if (ZNew > Small)
	{
		SqrtZ  = sqrt(ZNew);
		*C2New  = (1.0 - cos(SqrtZ)) / ZNew;
		*C3New  = (SqrtZ - sin(SqrtZ)) / (SqrtZ * SqrtZ * SqrtZ);
	}
	else
		if (ZNew < -Small)
		{
			SqrtZ  = sqrt(-ZNew);
			*C2New  = (1.0 - cosh(SqrtZ)) / ZNew;
			*C3New  = (sinh(SqrtZ) - SqrtZ) / (SqrtZ * SqrtZ * SqrtZ);
		}
		else
		{
			*C2New = 0.5;
			*C3New = 1.0 / 6.0;
		}
}

/*-----------------------------------------------------------------------------------------------------------------------
*
*                           function kepler
*
*  this function solves keplers problem for orbit determination and returns a
*    future position and velocity vector. The solution uses universal variables.
*
*  author        : david vallado                  719-573-2600   22 jun 2002
*
*  revisions
*    vallado     - fix some mistakes                             13 apr 2004
*
*  inputs          description                    range / units
*    ro          - ijk position vector - initial  km
*    vo          - ijk velocity vector - initial  km / s
*    dtsec       - length of time to propagate    s
*
*  outputs       :
*    r           - ijk position vector            km
*    v           - ijk velocity vector            km / s
*    error       - error flag                     'ok', ...
*
*  locals        :
*    f           - f expression
*    g           - g expression
*    fdot        - f dot expression
*    gdot        - g dot expression
*    xold        - old universal variable x
*    xoldsqrd    - xold squared
*    xnew        - new universal variable x
*    xnewsqrd    - xnew squared
*    znew        - new value of z
*    c2new       - c2(psi) function
*    c3new       - c3(psi) function
*    dtsec       - change in time                 s
*    timenew     - new time                       s
*    rdotv       - result of ro dot vo
*    a           - semi or axis                   km
*    alpha       - reciprocol  1/a
*    sme         - specific mech energy           km2 / s2
*    period      - time period for satellite      s
*    s           - variable for parabolic case
*    w           - variable for parabolic case
*    h           - angular momentum vector
*    temp        - temporary real*8 value
*    i           - index
*
*  coupling      :
*    mag         - magnitude of a vector
*    findc2c3    - find c2 and c3 functions
*
*  references    :
*    vallado       2007, 100-103, alg 8, ex 2-4
*
*/
// ----------------------------------------------------------------------------------------------------------------------

void kepler(f64 ro[3], f64 vo[3], f64 dtseco, f64 mu)
{
	u32 ktr, i, numiter, mulrev;
	f64 h[3], r[3], v[3], f, g, fdot, gdot, rval, xold, xoldsqrd, xnew,
		xnewsqrd, znew, p, c2new, c3new, dtnew, rdotv, a, dtsec,
		alpha, sme, period, s, w, temp,
		magro, magvo, magh, magr;
	char show, errork[10];
	show = 'n';
	f64 re, velkmps, small, twopi, halfpi;

	re  = 6378.137;
	velkmps = 7.905365719014;
	//mu = 398600.4418;
	small = 0.00000001;
	twopi = 2.0 * core::PI64;
	halfpi = core::PI64 * 0.5;
	//mu /= 1000000000;

	//axisSwap(ro, vo);
	// -------------------------  implementation   -----------------
	// set constants and u32ermediate pru32outs
	numiter    =    50;

	if (show =='y')
	{
		printf(" ro %16.8f %16.8f %16.8f ER \n",ro[0]/re,ro[1]/re,ro[2]/re );
		printf(" vo %16.8f %16.8f %16.8f ER/TU \n",vo[0]/velkmps, vo[1]/velkmps, vo[2]/velkmps );
	}

	// --------------------  initialize values   -------------------
	ktr   = 0;
	xold  = 0.0;
	znew  = 0.0;
	strcpy( errork, "      ok");
	dtsec = dtseco;
	mulrev = 0;

	if ( fabs( dtseco ) > small )
	{
		magro = mag( ro );
		magvo = mag( vo );
		rdotv= dot( ro,vo );

		// -------------  find sme, alpha, and a  ------------------
		sme= ( (magvo * magvo) * 0.5  ) - ( mu /magro );
		alpha= -sme * 2.0/mu;

		if ( fabs( sme ) > small )
			a= -mu / ( 2.0 * sme );
		else
			a= infinite;
		if ( abs( alpha ) < small )   // parabola
			alpha= 0.0;

		if (show =='y')
		{
			printf(" sme %16.8f  a %16.8f alp  %16.8f ER \n",sme/(mu/re), a/re, alpha * re );
			printf(" sme %16.8f  a %16.8f alp  %16.8f km \n",sme, a, alpha );
			printf(" ktr      xn        psi           r          xn+1        dtn \n" );
		}

		// ------------   setup initial guess for x  ---------------
		// -----------------  circle and ellipse -------------------
		if ( alpha >= small )
		{
			period= twopi * sqrt( fabs(a * a * a)/mu  );
			// ------- next if needed for 2body multi-rev ----------
			if ( abs( dtseco ) > abs( period ) )
				// including the truncation will produce vertical lines that are parallel
				// (plotting chi vs time)
				//                    dtsec = rem( dtseco,period );
				mulrev = floor(dtseco/period);
			if ( abs(alpha-1.0 ) > small )
				xold = sqrt(mu) * dtsec * alpha;
			else
				// - first guess can't be too close. ie a circle, r=a
				xold = sqrt(mu) * dtsec * alpha * 0.97;
		}
		else
		{
			// --------------------  parabola  ---------------------
			if ( abs( alpha ) < small )
			{
				cross( ro,vo, h );
				magh = mag(h);
				p= magh * magh/mu;
				s= 0.5  * (halfpi - atan( 3.0 * sqrt( mu / (p * p * p) ) * dtsec ) );
				w= atan( pow( tan( s ), (1.0 /3.0 )) );
				xold = sqrt(p) * ( 2.0 * cot(2.0 * w) );
				alpha= 0.0;
			}
			else
			{
				// ------------------  hyperbola  ------------------
				temp= -2.0 * mu * dtsec /
					( a * ( rdotv + sgn(dtsec) * sqrt(-mu * a) *
					(1.0 -magro * alpha) ) );
				xold= sgn(dtsec) * sqrt(-a) * log(temp);
			}
		} // if alpha

		ktr= 1;
		dtnew = -10.0;
		while ((fabs(dtnew/sqrt(mu) - dtsec) >= small) && (ktr < numiter))
		{
			xoldsqrd = xold * xold;
			znew     = xoldsqrd * alpha;

			// ------------- find c2 and c3 functions --------------
			findc2c3( znew, &c2new, &c3new );

			// ------- use a newton iteration for new values -------
			rval = xoldsqrd * c2new + rdotv/sqrt(mu) * xold * (1.0 -znew * c3new) +
				magro * ( 1.0  - znew * c2new );
			dtnew= xoldsqrd * xold * c3new + rdotv/sqrt(mu) * xoldsqrd * c2new +
				magro*xold * ( 1.0  - znew * c3new );

			// ------------- calculate new value for x -------------
			xnew = xold + ( dtsec * sqrt(mu) - dtnew ) / rval;

			// ------------------------------------------------------
			// check if the orbit is an ellipse and xnew > 2pi sqrt(a), the step
			// size must be changed.  this is accomplished by multiplying rval
			// by 10.0 .  note that 10.0  is arbitrary, but seems to produce good
			// results.  the idea is to keep xnew from increasing too rapidily.
			// ------------------------------------------------------
			//  including this doesn't work if you don't mod the dtsec
			//               if ( ( a > 0.0  ) and ( abs(xnew)>twopi*sqrt(a) ) and ( sme < 0.0  ) )
			//                   dx= ( dtsec-dtnew ) / rval  % *7.0   * 10.0
			//                   xnew = xold + dx / 7.0    % /(1.0  + dx)
			//                alternate method to test various values of change
			//                   xnew = xold + ( dtsec-dtnew ) / ( rval*10 chgamt  )

			if (show =='y')
			{
				printf("%3i %11.7f %11.7f %11.7f %11.7f %11.7f \n", ktr,xold,znew,rval,xnew,dtnew);
				printf("%3i %11.7f %11.7f %11.7f %11.7f %11.7f \n", ktr,xold/sqrt(re),znew,rval/re,xnew/sqrt(re),dtnew/sqrt(mu));
			}

			ktr = ktr + 1;
			xold = xnew;
		}  // while

		if ( ktr >= numiter )
		{
			strcpy( errork, "knotconv");
			printf("Fucked UP ! not converged in %2i iterations \n",numiter );
			for (i= 0; i < 3; i++)
			{
				r[i]= ro[i];
				v[i]= vo[i];
			}
		}
		else
		{
			// --- find position and velocity vectors at new time --
			xnewsqrd = xnew * xnew;
			f = 1.0  - ( xnewsqrd * c2new / magro );
			g = dtsec - xnewsqrd * xnew * c3new/sqrt(mu);

			for (i= 0; i < 3; i++)
				r[i]= f * ro[i] + g * vo[i];

			magr = mag( r );
			gdot = 1.0  - ( xnewsqrd * c2new / magr );
			fdot = ( sqrt(mu) * xnew / ( magro * magr ) ) * ( znew * c3new-1.0  );

			for (i= 0; i < 3; i++)
				v[i]= fdot * ro[i] + gdot * vo[i];

			//mag( v );
			temp= f * gdot - fdot * g;
			if ( fabs(temp-1.0 ) > 0.00001  )
				strcpy( errork, "fandg");

			if (show =='y')
			{
				printf("f %16.8f g %16.8f fdot %16.8f gdot %16.8f \n",f, g, fdot, gdot );
				printf("f %16.8f g %16.8f fdot %16.8f gdot %16.8f \n",f, g, fdot, gdot );
				printf("r1 %16.8f %16.8f %16.8f ER \n",r[0]/re,r[1]/re,r[2]/re );
				printf("v1 %16.8f %16.8f %16.8f ER/TU \n",v[0]/velkmps, v[1]/velkmps, v[2]/velkmps );
			}
		}
	} // if fabs
	else
	{
		// ----------- set vectors to incoming since 0 time --------
		for (i= 0; i < 3; i++)
		{
			r[i]= ro[i];
			v[i]= vo[i];
		}

	}
	ro[0] = r[0];
	ro[1] = r[1];
	ro[2] = r[2];

	vo[0] = v[0];
	vo[1] = v[1];
	vo[2] = v[2];

	//axisSwap(ro,vo);


	//       fprintf( fid,"%11.5f  %11.5f %11.5f  %5i %3i ",znew, dtseco/60.0, xold/(rad), ktr, mulrev );
}   // procedure kepler

/* ----------------------------------------------------------------------------------------------------------------------
*
*                           function rv2coe
*
*  this function finds the classical orbital elements given the geocentric
*    equatorial position and velocity vectors.
*
*  author        : david vallado                  719-573-2600   21 jun 2002
*
*  revisions
*    vallado     - fix special cases                              5 sep 2002
*    vallado     - delete extra check in inclination code        16 oct 2002
*    vallado     - add constant file use                         29 jun 2003
*
*  inputs          description                    range / units
*    r           - ijk position vector            km
*    v           - ijk velocity vector            km / s
*
*  outputs       :
*    p           - semilatus rectum               km
*    a           - semimajor axis                 km
*    ecc         - eccentricity
*    incl        - inclination                    0.0  to core::PI64 rad
*    omega       - longitude of ascending node    0.0  to 2pi rad
*    argp        - argument of perigee            0.0  to 2pi rad
*    nu          - true anomaly                   0.0  to 2pi rad
*    m           - mean anomaly                   0.0  to 2pi rad
*    arglat      - argument of latitude      (ci) 0.0  to 2pi rad
*    truelon     - true longitude            (ce) 0.0  to 2pi rad
*    lonper      - longitude of periapsis    (ee) 0.0  to 2pi rad
*
*  locals        :
*    hbar        - angular momentum h vector      km2 / s
*    ebar        - eccentricity     e vector
*    nbar        - line of nodes    n vector
*    c1          - v**2 - u/r
*    rdotv       - r dot v
*    hk          - hk unit vector
*    sme         - specfic mechanical energy      km2 / s2
*    i           - index
*    e           - eccentric, parabolic,
*                  hyperbolic anomaly             rad
*    temp        - temporary variable
*    typeorbit   - type of orbit                  ee, ei, ce, ci
*
*  coupling      :
*    mag         - magnitude of a vector
*    cross       - cross product of two vectors
*    angle       - find the angle between two vectors
*    newtonnu    - find the mean anomaly
*
*  references    :
*    vallado       2007, 121, alg 9, ex 2-5
*/
// ----------------------------------------------------------------------------------------------------------------------
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
 )
{
	f64 small, hbar[3], nbar[3], magr, magv, magn, ebar[3], sme,
		rdotv, temp, c1, hk, twopi, magh, halfpi, e;
	u32 i;
	char typeorbit[3];

	axisSwap(r,v);

	twopi  = 2.0 * core::PI64;
	halfpi = 0.5 * core::PI64;
	small  = 0.00000001;
	//mu     = 398600.4418;

	// -------------------------  implementation   -----------------
	magr = mag( r );
	magv = mag( v );

	// ------------------  find h n and e vectors   ----------------
	cross( r,v, hbar );
	magh = mag( hbar );
	if ( magh > small )
	{
		nbar[0]= -hbar[1];
		nbar[1]=  hbar[0];
		nbar[2]=   0.0;
		magn = mag( nbar );
		c1 = magv*magv - mu /magr;
		rdotv = dot( r,v );
		for (i= 0; i <= 2; i++)
			ebar[i]= (c1*r[i] - rdotv*v[i])/mu;
		*ecc = mag( ebar );

		// ------------  find a e and semi-latus rectum   ----------
		sme= ( magv*magv*0.5  ) - ( mu /magr );
		if ( fabs( sme ) > small )
			*a= -mu  / (2.0 *sme);
		else
			*a= infinite;
		*p = magh*magh/mu;

		// -----------------  find inclination   -------------------
		hk= hbar[2]/magh;
		*incl= acos( hk );

		// --------  determine type of orbit for later use  --------
		// ------ elliptical, parabolic, hyperbolic inclined -------
		strcpy(typeorbit,"ei");
		if ( *ecc < small )
		{
			// ----------------  circular equatorial ---------------
			if  ((*incl<small) | (fabs(*incl-core::PI64)<small))
				strcpy(typeorbit,"ce");
			else
				// --------------  circular inclined ---------------
				strcpy(typeorbit,"ci");
		}
		else
		{
			// - elliptical, parabolic, hyperbolic equatorial --
			if  ((*incl<small) | (fabs(*incl-core::PI64)<small))
				strcpy(typeorbit,"ee");
		}

		// ----------  find longitude of ascending node ------------
		if ( magn > small )
		{
			temp= nbar[0] / magn;
			if ( fabs(temp) > 1.0  )
				temp= sgn(temp);
			*omega= acos( temp );
			if ( nbar[1] < 0.0  )
				*omega= twopi - *omega;
		}
		else
			*omega= undefined;

		// ---------------- find argument of perigee ---------------
		if ( strcmp(typeorbit,"ei") == 0 )
		{
			*argp = angle( nbar,ebar);
			if ( ebar[2] < 0.0  )
				*argp= twopi - *argp;
		}
		else
			*argp= undefined;

		// ------------  find true anomaly at epoch    -------------
		if ( typeorbit[0] == 'e' )
		{
			*nu =  angle( ebar,r);
			if ( rdotv < 0.0  )
				*nu = twopi - *nu;
		}
		else
			*nu = undefined;

		// ----  find argument of latitude - circular inclined -----
		if ( strcmp(typeorbit,"ci") == 0 )
		{
			*arglat = angle( nbar,r );
			if ( r[2] < 0.0  )
				*arglat = twopi - *arglat;
			*m = *arglat;
		}
		else
			*arglat= undefined;

		// -- find longitude of perigee - elliptical equatorial ----
		if  (( *ecc > small ) && (strcmp(typeorbit,"ee") == 0))
		{
			temp= ebar[0] / *ecc;
			if ( fabs(temp) > 1.0  )
				temp= sgn(temp);
			*lonper = acos( temp );
			if ( ebar[1] < 0.0  )
				*lonper = twopi - *lonper;
			if ( *incl > halfpi )
				*lonper = twopi - *lonper;
		}
		else
			*lonper= undefined;

		// -------- find true longitude - circular equatorial ------
		if  (( magr>small ) && ( strcmp(typeorbit,"ce") == 0 ))
		{
			temp= r[0]/magr;
			if ( fabs(temp) > 1.0  )
				temp= sgn(temp);
			*truelon= acos( temp );
			if ( r[1] < 0.0  )
				*truelon= twopi - *truelon;
			if ( *incl > halfpi )
				*truelon= twopi - *truelon;
			*m = *truelon;
		}
		else
			*truelon= undefined;

		// ------------ find mean anomaly for all orbits -----------
		if ( typeorbit[0] == 'e' )
			newtonnu(*ecc, *nu,  e, *m);
	}
	else
	{
		*p    = undefined;
		*a    = undefined;
		*ecc  = undefined;
		*incl = undefined;
		*omega= undefined;
		*argp = undefined;
		*nu   = undefined;
		*m    = undefined;
		*arglat = undefined;
		*truelon= undefined;
		*lonper = undefined;
	}
}  // procedure rv2coe


// ----------------------------------------------------------------------------------------------------------------------
/*	This function takes the 6 kepler orbit elements (all angles in radians)
	and converts them to the radius and velocity vectors.  This code uses
	the equations given in Design 4 of the ASEN 2004 class
	(Also posted on this class's web site).

*    inputs:
*    a           - semimajor axis                 km
*    e           - eccentricity
*    i           - inclination                    0.0  to core::PI64 rad
*    O           - longitude of ascending node    0.0  to 2pi rad
*    w           - argument of perigee            0.0  to 2pi rad
*    f           - true anomaly                   0.0  to 2pi rad
*    mu          - G*Mass Prodcut                 0.0  to 2pi rad
*  references    :
*    http://www.colorado.edu/ASEN/asen3200/code/Kepler2Cart.m
*/
// ----------------------------------------------------------------------------------------------------------------------
void Kepler2Cart(f64 r_v[], f64 v_v[], f64 a, f64 e, f64 i, f64 O, f64 w, f64 f, f64 mu)
{
	f64 d_v[3];

	//Compute the radius mag
	f64 r = a * (1.0 - pow(e,2)) / (1.0 + e * cos(f));
	//printf("rKep = %f\n",r);

	//Find the X-Y-Z cosines for r vector
	//Arg of Latitude
	//f64 al = w+f;
	f64 al = w + f - O;

	f64 x = cos(O) * cos(al) - sin(O) * sin(al) * cos(i);
	f64 y = sin(O) * cos(al) + cos(O) * sin(al) * cos(i);
	f64 z = sin(i) * sin(al);

	//Calculate r vector
	r_v[0] = r * x;
	r_v[2] = r * y;
	r_v[1] = r * z;

	//Orbit parameter p (Not period)
	f64 p = a * (1.0 - pow(e,2));

	//Compute the specific angular momentum (usually h)
	f64 l = sqrt(mu * a * (1.0 - pow(e,2)));

	//Find the velocity Cosines
	d_v[0] = cos(O) * sin(al) + sin(O) * cos(al) * cos(i);
	d_v[2] = sin(O) * sin(al) - cos(O) * cos(al) * cos(i);
	d_v[1] = -sin(i) * cos(al);

	//Find the velocity
	v_v[0] = r_v[0] * l * e * sin(f) / r / p - l / r * d_v[0];
	v_v[1] = r_v[1] * l * e * sin(f) / r / p - l / r * d_v[1];
	v_v[2] = r_v[2] * l * e * sin(f) / r / p - l / r * d_v[2];

}
// ----------------------------------------------------------------------------------------------------------------------
/*	This function takes the 6 kepler orbit elements (all angles in radians)
	and converts them to the radius and velocity vectors.  This code uses
	the equations given in Design 4 of the ASEN 2004 class
	(Also posted on this class's web site).

*    inputs:
*    semi        - semimajor axis                 km
*    e           - eccentricity
*    i           - inclination                    0.0  to core::PI64 rad
*    node        - longitude of ascending node    0.0  to 2pi rad
*    arg         - argument of perigee            0.0  to 2pi rad
*    trueA       - true anomaly                   0.0  to 2pi rad
*    MU          - G*Mass Prodcut                 0.0  to 2pi rad
*  references    :
*    http://www.colorado.edu/ASEN/asen3200/code/Kepler2Cart.m
*/
// ----------------------------------------------------------------------------------------------------------------------
void COEstoRVmean(f64 RPQW[], f64 VPQW[], f64 semi, f64 e, f64 i, f64 node, f64 arg, f64 trueA, f64 MU)
{
	f64 p = semi * (1.0 - pow(e,2));  // p = semi-latus rectum
	//f64 RPQW[2];
	//f64 VPQW[2];

	RPQW[0] = p*cos(trueA) / (1+e*cos(trueA));
	RPQW[1] = p*sin(trueA) / (1+e*cos(trueA));
	RPQW[2] = 0;

	VPQW[0] = -sqrt(MU/p) * sin(trueA);
	VPQW[1] =  sqrt(MU/p) * (e+cos(trueA));
	VPQW[2] =  0;

	f64 center[3];
	center[0] = 0.0;
	center[1] = 0.0;
	center[2] = 0.0;

	rotateAroundZBy(RPQW, -arg, center);
	rotateAroundYBy(RPQW, -i, center);
	rotateAroundZBy(RPQW, -node, center);

	rotateAroundZBy(VPQW, -arg, center);
	rotateAroundYBy(VPQW, -i, center);
	rotateAroundZBy(VPQW, -node, center);
	/*
	RIJK = rot3( rot1( rot3( RPQW',-arg ),-i ),-node );
	VIJK = rot3( rot1( rot3( VPQW',-arg ),-i ),-node );
	*/

}
/*
void coe2rv(double p,
double ecc,
double incl,
double omega,
double argp,
double nu,
double arglat,
double truelon,
double lonper,
double r[3],
double v[3])
{
/* ------------------------------------------------------------------------------
*
*                           function coe2rv
*
*  this function finds the position and velocity vectors in geocentric
*    equatorial (ijk) system given the classical orbit elements.
*
*  author        : david vallado                  719-573-2600    1 mar 2001
*
*  inputs          description                    range / units
*    p           - semilatus rectum               km
*    ecc         - eccentricity
*    incl        - inclination                    0.0 to pi rad
*    omega       - longitude of ascending node    0.0 to 2pi rad
*    argp        - argument of perigee            0.0 to 2pi rad
*    nu          - true anomaly                   0.0 to 2pi rad
*    arglat      - argument of latitude      (ci) 0.0 to 2pi rad
*    lamtrue     - true longitude            (ce) 0.0 to 2pi rad
*    lonper      - longitude of periapsis    (ee) 0.0 to 2pi rad
*
*  outputs       :
*    r           - ijk position vector            km
*    v           - ijk velocity vector            km / s
*
*  locals        :
*    temp        - temporary real*8 value
*    rpqw        - pqw position vector            km
*    vpqw        - pqw velocity vector            km / s
*    sinnu       - sine of nu
*    cosnu       - cosine of nu
*    tempvec     - pqw velocity vector
*
*  coupling      :
*    rot3        - rotation about the 3rd axis
*    rot1        - rotation about the 1st axis
*
*  references    :
*    vallado       2007, 126, alg 10, ex 2-5
* ---------------------------------------------------------------------------




double rpqw[3], vpqw[3], tempvec[3], temp, sinnu, cosnu,
small, mu;

small = 0.0000001;
mu    = 398600.4418;

// --------------------  implementation   ----------------------
//       determine what type of orbit is involved and set up the
//       set up angles for the special cases.
// -------------------------------------------------------------
if ( ecc < small )
{
// ----------------  circular equatorial  ------------------
if ( (incl < small) | ( fabs(incl-pi) < small ) )
{
argp = 0.0;
omega= 0.0;
nu   = truelon;
}
else
{
// --------------  circular inclined  ------------------
argp= 0.0;
nu  = arglat;
}
}
else
{
// ---------------  elliptical equatorial  -----------------
if ( ( incl < small) | (fabs(incl-pi) < small) )
{
argp = lonper;
omega= 0.0;
}
}

// ----------  form pqw position and velocity vectors ----------
cosnu= cos(nu);
sinnu= sin(nu);
temp = p / (1.0 + ecc*cosnu);
rpqw[0]= temp*cosnu;
rpqw[1]= temp*sinnu;
rpqw[2]=     0.0;
if ( fabs(p) < 0.00000001 )
p= 0.00000001;

vpqw[0]=    -sinnu    * sqrt(mu/p);
vpqw[1]=  (ecc + cosnu) * sqrt(mu/p);
vpqw[2]=      0.0;

// ----------------  perform transformation to ijk  ------------
rot3( rpqw   , -argp , tempvec );
rot1( tempvec, -incl , tempvec );
rot3( tempvec, -omega, r     );

rot3( vpqw   , -argp , tempvec );
rot1( tempvec, -incl , tempvec );
rot3( tempvec, -omega, v     );
// procedure coe2rv

}

*/
