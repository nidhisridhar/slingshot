/*
The Irrlicht Engine License

Copyright © 2002-2005 Nikolaus Gebhardt

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.



Original source code by Stephen R. Schmitt written in zeno. Ported to mel and c++ by Tobias Houfek.
http://home.att.net/~srschmitt/planetorbits.html

*/
#include <string>
#include <ctime>
//#include <direct.h>
#include <iostream>
#include <irrlicht.h>
#include "solar.h"
#include "Geopack-2008.h"
#include "t96_01.h"
#include "kepler.h"
#include "doubleVectorOps.h"
#include "GTOPtoolbox/Pl_Eph_An.h"

//#include "ts05.h"

using namespace irr;

const f64 hourSeconds = 3600.0;
const f64 secondsM = 60.0;
const f64 dayH = 24.0;
const f64 yearD = 365.00;

//Constructor
SolarSystem::SolarSystem()
: year(0), month(0), day(0), dayM(0), hour(0), minute(0), second(0)
{
	year = 0;
	month = 0;
	day = 0;
	dayM = 0;
	hour = 0;
	minute = 0;
	second = 0;
	
	doublePositions[9][0] = 0.0;
	doublePositions[9][1] = 0.0;
	doublePositions[9][2] = 0.0;

	doubleVelocities[9][0] = 0.0;
	doubleVelocities[9][1] = 0.0;
	doubleVelocities[9][2] = 0.0;
}

//Destructor
SolarSystem::~SolarSystem()
{
}


//TODO move to Geomagnetic.cpp
// ----------------------------------------------------------------------------------------------------------------------
// trace field Lines
core::array<scene::SMeshBuffer*> SolarSystem::calculateFieldLines(s32 year, 
																  s32 dayM, 
																  s32 hour, 
																  s32 minute, 
																  s32 second)
{
	//DEFINE THE UNIVERSAL TIME AND PREPARE THE COORDINATE TRANSFORMATION PARAMETERS
	//BY INVOKING THE SUBROUTINE RECALC_08
	integer fyear = (integer)year;
	integer fdayM = (integer)dayM;
	integer fhour = (integer)hour;
	integer fminute = (integer)minute;
	integer fsecond = (integer)second;

	//IF ONLY QUESTIONABLE INFORMATION (OR NO INFORMATION AT ALL) IS AVAILABLE 
	//ON THE SOLAR WIND SPEED, OR, IF THE STANDARD GSM AND/OR SM COORDINATES ARE 
	//INTENDED TO BE USED, THEN SET VGSEX=-400.0 AND VGSEY=VGSEZ=0. IN THIS CASE, 
	//THE GSW COORDINATE SYSTEM BECOMES IDENTICAL TO THE STANDARD GSM. 
	real vgsex = -400.f;
	real vgsey = 0.0f;
	real vgsez = 0.0f;

	/*PARMOD -  A 10-ELEMENT ARRAY CONTAINING INPUT PARAMETERS NEEDED FOR A UNIQUE */
	/*SPECIFICATION OF THE EXTERNAL FIELD MODEL. THE CONCRETE MEANING OF THE COMPONENTS */
	/*OF PARMOD DEPENDS ON A SPECIFIC VERSION OF THAT MODEL. */
	//------------------------------------------------------------------------------------------
	/* DATA-BASED MODEL CALIBRATED BY */
	/*(1) SOLAR WIND PRESSURE PDYN (NANOPASCALS), */
	/*(2) DST (NANOTESLA), */
	/*(3) BYIMF, AND */
	/*(4) BZIMF (NANOTESLA). */
	/* THESE INPUT PARAMETERS SHOULD BE PLACED IN THE FIRST 4 ELEMENTS */
	/* OF THE ARRAY PARMOD(10). */
	/*
	real parmod[10];
	parmod[0] = 3.f;
	parmod[1] = -20.f;
	parmod[2] = 3.f;
	parmod[3] = -5.f;
	*/
	real parmod[10];
	parmod[0] = 2.f;
	parmod[1] = -10.f;
	parmod[2] = 0.f;
	parmod[3] = 0.f;
	recalc_08__(&fyear, &fdayM, &fhour, &fminute, &fsecond, &vgsex, &vgsey, &vgsez );

	core::array<scene::SMeshBuffer*> fieldLineMeshBufferA;

	for(s32 longitude = -180; longitude <= 150; longitude += 30)
	{
		for(s32 latitude = 7; latitude <= 84; latitude += 7)
		{
			real re = 1.f;
			real colat = (90.f - (real)latitude) * .01745329f;
			real xlon = (real)longitude * .01745329f;

			static integer j = 1;

			//output
			real xgeo = 0.f;
			real ygeo = 0.f;
			real zgeo = 0.f;

			//CONVERT SPHERICAL COORDS INTO CARTESIAN
			sphcar_08__(&re, &colat, &xlon, &xgeo, &ygeo, &zgeo, &j);
			//-----------------------------------------------------------------------------

			//output
			real xgsw = 0.f;
			real ygsw = 0.f;
			real zgsw = 0.f;

			//TRANSFORM GEOGRAPHICAL GEOCENTRIC COORDS INTO SOLAR WIND MAGNETOSPHERIC ONES
			geogsw_08__(&xgeo, &ygeo, &zgeo, &xgsw, &ygsw, &zgsw, &j);
			//-----------------------------------------------------------------------------

			//TRACE THE FIELD LINE:
			//SPECIFY TRACING PARAMETERS

			/*LMAX - MAXIMAL LENGTH OF THE ARRAYS XX,YY,ZZ, IN WHICH COORDINATES OF THE FIELD */
			/*LINE POINTS ARE STORED. LMAX SHOULD BE SET EQUAL TO THE ACTUAL LENGTH OF */
			/*THE ARRAYS, DEFINED IN THE MAIN PROGRAM AS ACTUAL ARGUMENTS OF THIS SUBROUTINE. */
			//------------------------------------------------------------------------------------------------
			/*  LMAX IS THE UPPER LIMIT ON THE NUMBER OF FIELD LINE POINTS RETURNED BY THE TRACER. */
			/*  IT CAN BE SET ARBITRARILY LARGE, DEPENDING ON THE SPECIFICS OF A PROBLEM UNDER STUDY. */
			/*  IN THIS EXAMPLE, LMAX IS TENTATIVELY SET EQUAL TO 500. */
			static integer lmax = 500;

			/*ERR - PERMISSIBLE STEP ERROR. A REASONABLE ESTIMATE PROVIDING A SUFFICIENT ACCURACY FOR MOST */
			/*APPLICATIONS IS ERR=0.0001. SMALLER/LARGER VALUES WILL RESULT IN LARGER/SMALLER NUMBER */
			/*OF STEPS AND, HENCE, OF OUTPUT FIELD LINE POINTS. NOTE THAT USING MUCH SMALLER VALUES */
			/*OF ERR MAY REQUIRE USING A DOUBLE PRECISION VERSION OF THE ENTIRE PACKAGE. */
			//------------------------------------------------------------------------------------------------
			//PERMISSIBLE STEP ERROR SET AT ERR=0.0001
			static real err = 0.0001f;

			/*RLIM - RADIUS OF A SPHERE (IN RE), DEFINING THE OUTER BOUNDARY OF THE TRACING REGION; */
			/*IF THE FIELD LINE REACHES THAT BOUNDARY FROM INSIDE, ITS OUTBOUND TRACING IS */
			/*TERMINATED AND THE CROSSING POINT COORDINATES XF,YF,ZF ARE CALCULATED. */
			//------------------------------------------------------------------------------------------------
			//LIMIT THE TRACING REGION WITHIN R=60 Re
			static real rlim = 250.f;

			/*IOPT - A MODEL INDEX; CAN BE USED FOR SPECIFYING A VERSION OF THE EXTERNAL FIELD */
			/*MODEL (E.G., A NUMBER OF THE KP-INDEX INTERVAL). ALTERNATIVELY, ONE CAN USE THE ARRAY */
			/*PARMOD FOR THAT PURPOSE (SEE BELOW); IN THAT CASE IOPT IS JUST A DUMMY PARAMETER. */
			//------------------------------------------------------------------------------------------------
			//IN THIS EXAMPLE IOPT IS JUST A DUMMY PARAMETER,
			//WHOSE VALUE DOES NOT MATTER
			integer iopt = 0;

			/*DIR - SIGN OF THE TRACING DIRECTION: IF DIR=1.0 THEN THE TRACING IS MADE ANTIPARALLEL */
			/*TO THE TOTAL FIELD VECTOR (E.G., FROM NORTHERN TO SOUTHERN CONJUGATE POINT); */
			/*IF DIR=-1.0 THEN THE TRACING PROCEEDS IN THE OPPOSITE DIRECTION, THAT IS, PARALLEL TO */
			/*THE TOTAL FIELD VECTOR. */
			//------------------------------------------------------------------------------------------------
			//TRACE THE LINE WITH A FOOTPOINT IN THE NORTHERN HEMISPHERE, THAT IS,
			//ANTIPARALLEL TO THE MAGNETIC FIELD 
			real dir = 1.f;

			/*DSMAX - UPPER LIMIT ON THE STEPSIZE (SETS A DESIRED MAXIMAL SPACING BETWEEN */
			/*THE FIELD LINE POINTS) */
			//------------------------------------------------------------------------------------------------
			//MAXIMAL SPACING BETWEEN THE FIELD LINE POINTS SET EQUAL TO 1 RE
			static real dsmax = 1.f;

			/*R0 -  RADIUS OF A SPHERE (IN RE), DEFINING THE INNER BOUNDARY OF THE TRACING REGION */
			/*(USUALLY, EARTH'S SURFACE OR THE IONOSPHERE, WHERE R0~1.0) */
			/*IF THE FIELD LINE REACHES THAT SPHERE FROM OUTSIDE, ITS INBOUND TRACING IS */
			/*TERMINATED AND THE CROSSING POINT COORDINATES XF,YF,ZF  ARE CALCULATED. */
			//------------------------------------------------------------------------------------------------
			//LANDING POINT WILL BE CALCULATED ON THE SPHERE R=1,
			//I.E. ON THE EARTH'S SURFACE
			static real r0 = 1.f;

			/*EXNAME - NAME OF A SUBROUTINE PROVIDING COMPONENTS OF THE EXTERNAL MAGNETIC FIELD */
			/*(E.G., T89, OR T96_01, ETC.). */
			/*INNAME - NAME OF A SUBROUTINE PROVIDING COMPONENTS OF THE INTERNAL MAGNETIC FIELD */
			/*(EITHER DIP_08 OR IGRF_GSW_08). */

			//EXTERNAL MODELS
			//t96_01__ - Example conf.
			//t04_s__

			//INTERNAL MODELS
			//igrf_gsw_08__ - Example conf.
			//dip_08__

			//------------------------------------------------------------------------------------------------
			//------------------------------------------------------------------------------------------------
			//output
			//XF,YF,ZF - GSW COORDINATES OF THE ENDPOINT OF THE TRACED FIELD LINE.
			//XX,YY,ZZ - ARRAYS OF LENGTH LMAX, CONTAINING COORDINATES OF THE FIELD LINE POINTS.
			//L - ACTUAL NUMBER OF FIELD LINE POINTS, GENERATED BY THIS SUBROUTINE.

			/*  IN THE GSW SYSTEM, X AXIS IS ANTIPARALLEL TO THE OBSERVED DIRECTION OF THE SOLAR WIND FLOW. */
			/*  TWO OTHER AXES, Y AND Z, ARE DEFINED IN THE SAME WAY AS FOR THE STANDARD GSM, THAT IS, */
			/*  Z AXIS ORTHOGONAL TO X AXIS, POINTS NORTHWARD, AND LIES IN THE PLANE DEFINED BY THE X- */
			/*  AND GEODIPOLE AXIS. THE Y AXIS COMPLETES THE RIGHT-HANDED SYSTEM. */

			/*  THE GSW SYSTEM BECOMES IDENTICAL TO THE STANDARD GSM IN THE CASE OF */
			/*   A STRICTLY RADIAL SOLAR WIND FLOW. */

			real xf = 0.f;
			real yf = 0.f;
			real zf = 0.f;

			real xx[500];
			real yy[500];
			real zz[500];

			integer l = 0;

			trace_08__(&xgsw, &ygsw, &zgsw, &dir, &dsmax, &err, &rlim, &r0, &iopt, parmod,
				(S_fp)t96_01__, (S_fp)igrf_gsw_08__,
				&xf, &yf, &zf, 
				xx, yy, zz,
				&l, &lmax);

			//printf("%d\n",(s32)l);
			scene::SMeshBuffer *fieldLineMeshBuffer = new scene::SMeshBuffer();
			s32 colorIncrement = 255/l;
			s32 currentColor = 255;
			bool centre = false;
			for(s32 p=0; p<l; p++)
			{
				fieldLineMeshBuffer->Indices.push_back(p);
				if(p != 0 && p != l-1)
					fieldLineMeshBuffer->Indices.push_back(p);


				//igrf_gsw_08__(real *xgsw, real *ygsw, real *zgsw, real *hxgsw, real *hygsw, real *hzgsw);


				fieldLineMeshBuffer->Vertices.push_back(video::S3DVertex( 
					((xx[p]*6371.0f)/100000)*1.01,
					((yy[p]*6371.0f)/100000)*1.01,
					((zz[p]*6371.0f)/100000)*1.01,
					1,1,1,
					video::SColor(0,0,currentColor,255),
					1,1));
				if(currentColor < 255 && !centre)
				{
					currentColor -= colorIncrement*2;
					centre = true;
				}	
				else
				{
					currentColor += colorIncrement*2;
				}
			}
			fieldLineMeshBufferA.push_back(fieldLineMeshBuffer);
		}
	}
	return fieldLineMeshBufferA;
}
// ----------------------------------------------------------------------------------------------------------------------
// secondsBetweenTwoDates
f64 SolarSystem::deltaDate( s32 y1, s32 m1, s32 d1, s32 hour1, s32 mins1, s32 secs1, s32 y2, s32 m2, s32 d2, s32 hour2, s32 mins2, s32 secs2 )
{
	f64 day1 = SolarSystem::gregToJ2000(y1,m1,d1,hour1,mins1,secs1);
	f64 day2 = SolarSystem::gregToJ2000(y2,m2,d2,hour2,mins2,secs2);
	f64 deltaDate = (core::max_<f64>(day1,day2) - core::min_<f64>(day1,day2))*86400.0;
	return deltaDate;
}
// ----------------------------------------------------------------------------------------------------------------------
// secondsSinceMidDay
f64 SolarSystem::deltaMidDay()
{
	s32 hourM = 12;
	s32 minuteM = 0;
	s32 secondM = 0;

	f64 day1 = SolarSystem::gregToJ2000(year,
										month,
										day,
										hourM,
										minuteM,
										secondM);
	
	f64 day2 = SolarSystem::gregToJ2000(year,
										month,
										day,
										hour,
										minute,
										second);
	
	
	f64 deltaDate = (day1-day2)+0.5;
	//printf("%f - %f - day1 minus day2 = %f\n",day1,day2,deltaDate);
	return deltaDate;
}


// ----------------------------------------------------------------------------------------------------------------------
// days since/from J2000 (12:00 UT, Jan 1, 2000)
f64 SolarSystem::gregToJ2000( s32 y, s32 m, s32 d, s32 hour, s32 mins, s32 secs){
	/*
	NOTE: We're calculating days, later we will calculate centuries... from http://fer3.com/arc/m2.aspx?i=022034&y=200502

	The common units of time are hopelessly complicated.  There are too many of them; years, months, days, hours, minutes, 
	and seconds.  Time must be changed into a single unit.  The chosen unit is the number of centuries after noon 1 January 2000.  
	Time has a value of -.5 at 1200 on 1 January 1950, 0 at 1200 on 1 January 2000, and +.5 at 1200 on 1 January 2050.

	Van Flandern and Pulkkinen give a short formula for converting common UTC (or GMT) time to UTC centuries that is valid from 
	March 1900 to February 2100.
	Tu = (367*yr-trunc(7*(yr+trunc((mo+9)/12))/4)+trunc(275*mo/9)
	+day+(hr+(min+(sec/60))/60)/24  
	-730531.5)
	/36525

	The formula fails outside those dates because 1900 and 2100 are both years divisible by 4 which are not leap years.  The function 
	trunc is the integer part of the number within the brackets; any fractional part is dropped.  To have an accuracy of one second, 
	the value of time must have 11 significant digits because there are over 6 billion seconds in two centuries.
	The way the formula works is a little vague and is best explained working backwards.  At the end of the formula, 36525 converts the 
	units from days to centuries.  The 730531.5 makes the formula have a value of 0 at noon on 1 January 2000.  
	The term "day+(hr+(min+(sec/60))/60)/24" converts the date of the month and time into the number of days since the beginning of the 
	last day of the previous month.  The remaining part of the formula handles the changing number of days in the months and accounts 
	for leap years.  You can get an idea of how it works by solving it for the first day of each month for four consecutive years writing 
	down the numbers from within each set of parentheses.
	*/
	f64 yD = f64(y);
	f64 mD = f64(m);
	f64 dD = f64(d);
	f64 hD = f64(hour);
	f64 miD = f64(mins);
	f64 sD = f64(secs);

	f64 rv;
	//rv = (367.0f * y - floor(7.0f*(y + floor((m + 9.0f)/12.0f))/4.0f) + floor(275.0f*m/9.0f) + d - 730531.5f + h/24.0f);
	rv = (367.0 * yD - floor(7.0*(yD + floor((mD + 9.0)/12.0))/4.0) + floor(275.0*mD/9.0) 
		+ dD +(hD+(miD+(sD/secondsM))/secondsM)/dayH
		- 730531.5);

	return rv;
}

// ----------------------------------------------------------------------------------------------------------------------
// days since/from J2000 (12:00 UT, Jan 1, 2000)
f64 SolarSystem::gregToJ( s32 y, s32 m, s32 d, s32 hour, s32 mins, s32 secs)
{
	/*
	NOTE: We're calculating days, later we will calculate centuries... from http://fer3.com/arc/m2.aspx?i=022034&y=200502

	The common units of time are hopelessly complicated.  There are too many of them; years, months, days, hours, minutes, 
	and seconds.  Time must be changed into a single unit.  The chosen unit is the number of centuries after noon 1 January 2000.  
	Time has a value of -.5 at 1200 on 1 January 1950, 0 at 1200 on 1 January 2000, and +.5 at 1200 on 1 January 2050.

	Van Flandern and Pulkkinen give a short formula for converting common UTC (or GMT) time to UTC centuries that is valid from 
	March 1900 to February 2100.
	Tu = (367*yr-trunc(7*(yr+trunc((mo+9)/12))/4)+trunc(275*mo/9)
	+day+(hr+(min+(sec/60))/60)/24  
	-730531.5)
	/36525

	The formula fails outside those dates because 1900 and 2100 are both years divisible by 4 which are not leap years.  The function 
	trunc is the integer part of the number within the brackets; any fractional part is dropped.  To have an accuracy of one second, 
	the value of time must have 11 significant digits because there are over 6 billion seconds in two centuries.
	The way the formula works is a little vague and is best explained working backwards.  At the end of the formula, 36525 converts the 
	units from days to centuries.  The 730531.5 makes the formula have a value of 0 at noon on 1 January 2000.  
	The term "day+(hr+(min+(sec/60))/60)/24" converts the date of the month and time into the number of days since the beginning of the 
	last day of the previous month.  The remaining part of the formula handles the changing number of days in the months and accounts 
	for leap years.  You can get an idea of how it works by solving it for the first day of each month for four consecutive years writing 
	down the numbers from within each set of parentheses.
	*/
	f64 yD = f64(y);
	f64 mD = f64(m);
	f64 dD = f64(d);
	f64 hD = f64(hour);
	f64 miD = f64(mins);
	f64 sD = f64(secs);

	f64 rv;
	//rv = (367.0f * y - floor(7.0f*(y + floor((m + 9.0f)/12.0f))/4.0f) + floor(275.0f*m/9.0f) + d - 730531.5f + h/24.0f);
	rv = (367.0 * yD - floor(7.0*(yD + floor((mD + 9.0)/12.0))/4.0) + floor(275.0*mD/9.0) 
		+ dD +(hD+(miD+(sD/secondsM))/secondsM)/dayH
		- 730531.5 + 2400000.5 + 51544.5);

	return rv;
}

// ----------------------------------------------------------------------------------------------------------------------
// convert back to Gregorian Date
core::stringw SolarSystem::J2000ToGreg(f64 JD)
{
	
	s32 ja, jb, jc, jd, je;

	f64 t;
	f64 julianDay = 2451545.0 + JD;
	f64 dayremainder = JD - floor(JD);

	ja = core::round32(julianDay);
	t = dayH * (julianDay - ja + 0.5);

	if (ja >= 2299161) {
		jb = floor(((ja - 1867216.0) - 0.25) / 36524.25);
		ja = ja + 1 + jb - floor(jb / 4.0);
	}

	jb = ja + 1524;
	jc = floor(6680.0 + ((jb - 2439870.0) - 122.1) / 365.25);
	jd = yearD * jc + floor(jc / 4.0);
	je = floor((jb - jd) / 30.6001);
	day = jb - jd - floor(30.6001 * je);
	month = je - 1;
	if (month > 12)
		month = month - 12;
	year = jc - 4715;
	if (month > 2)
		year = year - 1;
	if (year < 0)
		year = year - 1;
	hour = floor(dayremainder*dayH);
	minute = floor( (dayremainder*dayH*secondsM)-(hour*secondsM) );
	second = floor( (dayremainder*dayH*hourSeconds)-(hour*hourSeconds+minute*secondsM) );
	hour += 12.0;	
	if(hour>23.0){
		hour -= dayH;
	}
	
	core::stringw result = "";
	result += year;
	result += "/";
	if(month<10)
		result += "0";
	result += month;
	result += "/";
	if(day<10)
		result += "0";
	result += day;
	result += " - ";
	if(hour<10)
		result += "0";
	result += hour;
	result += ":";
	if(minute<10)
		result += "0";
	result += minute;
	result += ":";
	if(second<10)
		result += "0";
	result += second;

	//tired of date systems
	//approximation used for magnetic field
	dayM = (s32)floor( ((month-1)+(day/30.0f)) * 30.4166667f );

	return result;
}
// ----------------------------------------------------------------------------------------------------------------------
// compute all coordinates at day
void SolarSystem::updateAtDay(f64 d, s32 p)
{	
	s32 max = 0;
	s32 min = 0;

	if(p == -1)
	{
		min = 0;
		max = 9;
	}
	else
	{
		min = p;
		max = p+1;
	}

	doublePositions[9][0] = offsetDouble[0];
	doublePositions[9][1] = offsetDouble[1];
	doublePositions[9][2] = offsetDouble[2];

	for(s32 i=min; i<max; i++)
	{
		f64 RPQW[3];
		f64 VPQW[3];
		f64 keps[6];

		//0 a - mean distance (AU) = SemiMajorAxis +/- AU/Ct
		//1 e - eccentricity = Eccentricity +/- e/Ct
		//2 i - inclination = Inclination +/- i"/Ct
		//3 O - longitude of ascending node = Ascending Node +/- Omega"/Ct
		//4 w - longitude of perihelion = Long of Periocenter +/- ~omega"/Ct
		//5 V - eccentric anomaly
		//6 R - helio-centric-radius

		Planet_Ephemerides_Analytical_kep (d,i+1,RPQW,VPQW,keps);
		
		OrbitalElements[i][0] = keps[0];
		OrbitalElements[i][1] = keps[1];
		OrbitalElements[i][2] = keps[2];
		OrbitalElements[i][3] = keps[3];
		OrbitalElements[i][4] = keps[4];
		OrbitalElements[i][5] = keps[5];
		
		doubleVelocities[i][0] = VPQW[0];
		doubleVelocities[i][1] = VPQW[2];
		doubleVelocities[i][2] = VPQW[1];

		doublePositions[i][0] = RPQW[0]+offsetDouble[0];
		doublePositions[i][1] = RPQW[2]+offsetDouble[1];
		doublePositions[i][2] = RPQW[1]+offsetDouble[2];

	}
	
}










