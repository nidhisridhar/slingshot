/*

Astronomical calculation routines

*/

#include <irrlicht.h>
using namespace irr;
/*  UCTTOJ  --  Convert GMT date and time to astronomical
Julian time (i.e. Julian date plus day fraction,
expressed as a f64).  */
f64 ucttoj(s32 year, s32 mon, s32 mday, s32 hour, s32 min, s32 sec)
{

	// Algorithm as given in Meeus, Astronomical Algorithms, Chapter 7, page 61

	s32 a, b, m;
	s32 y;
	f64 j;

	m = mon + 1;
	y = year;

	if (m <= 2) {
		y--;
		m += 12;
	}

	/* Determine whether date is in Julian or Gregorian calendar based on
	canonical date of calendar reform. */

	if ((year < 1582) || ((year == 1582) && ((mon < 9) || (mon == 9 && mday < 5)))) {
		b = 0;
	} else {
		a = ((s32) (y / 100));
		b = 2 - a + (a / 4);
	}

	j = (((s32) (365.25 * (y + 4716))) + ((s32) (30.6001 * (m + 1))) +
		mday + b - 1524.5) +
		((sec + 60L * (min + 60L * hour)) / 86400.0);
	return j;
}


/*  JYEAR  --  Convert  Julian  date  to  year,  month, day, which are
returned via integer pointers to integers (note that year
is a s32).  */
void jyear(f64 td, s32 *yy, s32 *mm, s32 *dd)
{
	f64 z, f, a, alpha, b, c, d, e;

	td += 0.5;
	z = floor(td);
	f = td - z;

	if (z < 2299161.0) {
		a = z;
	} else {
		alpha = floor((z - 1867216.25) / 36524.25);
		a = z + 1 + alpha - floor(alpha / 4);
	}

	b = a + 1524;
	c = floor((b - 122.1) / 365.25);
	d = floor(365.25 * c);
	e = floor((b - d) / 30.6001);

	*dd = (s32) (b - d - floor(30.6001 * e) + f);
	*mm = (s32) ((e < 14) ? (e - 1) : (e - 13));
	*yy = (s32) ((*mm > 2) ? (c - 4716) : (c - 4715));
	*yy += 6712;
}

/*JHMS  --  Convert Julian time to hour, minutes, and seconds.  */
void jhms(f64 j, s32 *h, s32 *m, s32 *s)
{
	s32 ij;

	j += 0.5;                         /* Astronomical to civil */
	ij = (s32) ((j - floor(j)) * 86400.0);
	*h = (s32) (ij / 3600L);
	*m = (s32) ((ij / 60L) % 60L);
	*s = (s32) (ij % 60L);
}

/*JWDAY  --  Determine day of the week for a given Julian day.  */
s32 jwday(f64 j)
{
	return ((s32) (j + 1.5)) % 7;
}

/*DELT  --  find the difference between universal and ephemeris time 
(delT = ET - UT)*/
f64 delT(const f64 jd)
{

    // Valid from 1825 to 2000, Montenbruck & Pfelger (2000), p 188
    const f64 T = (jd - 2451545)/36525;
    const s32 i = (s32) floor(T/0.25);

    const f64 c[7][4] = {
        { 10.4, -80.8,  413.9,  -572.3 },
        {  6.6,  46.3, -358.4,    18.8 },
        { -3.9, -10.8, -166.2,   867.4 },
        { -2.6, 114.1,  327.5, -1467.4 },
        { 24.2,  -6.3,   -8.2,   483.4 },
        { 29.3,  32.5,   -3.8,   550.7 },
        { 45.3, 130.5, -570.5,  1516.7 }
    };

    f64 t = T - i * 0.25;

    s32 ii = i + 7;
    if (ii < 0)
    {
        t = 0;
        ii = 0;
    }
    else if (ii > 6)
    {
        ii = 6;
        t = 0.25;
    }

    const f64 delT = c[ii][0] + t * (c[ii][1]
                                        + t * (c[ii][2]
                                               + t * c[ii][3]));


    return(delT);
}