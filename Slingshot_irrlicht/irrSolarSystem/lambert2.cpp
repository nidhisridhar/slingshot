#include <irrlicht.h>

using namespace irr;


/*------------------------------------------------------------------------------
|
|                           PROCEDURE FINDC2C3
|
|  This PROCEDURE calcuLates the C2 and C3 functions for use in the Universal
|    Variable calcuLation of z.
|
|  Author        : David Vallado                  303-344-6037    1 Mar 2001
|                                                                4 Feb 1992
|  Inputs          Description                    Range / Units
|    ZNew        - Z variable                     rad2
|
|  Outputs       :
|    C2New       - C2 FUNCTION value
|    C3New       - C3 FUNCTION value
|
|  Locals        :
|    SqrtZ       - Square root of ZNew
|
|  Coupling      :
|    SINH        - Hyperbolic Sine
|    COSH        - Hyperbolic Cosine
|
|  References    :
|    Vallado       2001, 70-71, Alg 1
|
 -----------------------------------------------------------------------------*/
void FindC2C3(f64 ZNew, f64& C2New, f64& C3New)
{
  const f64 Small = 0.00000001;       // Small value for tolerances
  f64 SqrtZ;

  if (ZNew > Small)
  {
    SqrtZ  = sqrt(ZNew);
    C2New  = (1.0 - cos(SqrtZ)) / ZNew;
    C3New  = (SqrtZ - sin(SqrtZ)) / (SqrtZ * SqrtZ * SqrtZ);
  }
  else
    if (ZNew < -Small)
    {
      SqrtZ  = sqrt(-ZNew);
      C2New  = (1.0 - cosh(SqrtZ)) / ZNew;
      C3New  = (sinh(SqrtZ) - SqrtZ) / (SqrtZ * SqrtZ * SqrtZ);
    }
    else
    {
      C2New = 0.5;
      C3New = 1.0 / 6.0;
    }
}

/* Utility functions for LambertBattin, etc */
static f64 k(f64 v)
{
  f64 d[21] =
       {
            1.0 /     3.0,     4.0 /    27.0,
            8.0 /    27.0,     2.0 /     9.0,
           22.0 /    81.0,   208.0 /   891.0,
          340.0 /  1287.0,   418.0 /  1755.0,
          598.0 /  2295.0,   700.0 /  2907.0,
          928.0 /  3591.0,  1054.0 /  4347.0,
         1330.0 /  5175.0,  1480.0 /  6075.0,
         1804.0 /  7047.0,  1978.0 /  8091.0,
         2350.0 /  9207.0,  2548.0 / 10395.0,
         2968.0 / 11655.0,  3190.0 / 12987.0,
         3658.0 / 14391.0
       };
  f64 del, delold, term, termold, temp, sum1;
  s32 i;

  /* ---- Process Forwards ---- */
  sum1    = d[0];
  delold  = 1.0;
  termold = d[0];
  i = 1;
  while ((i <= 20) && (fabs(termold) > 0.000001))
  {
    del  = 1.0 / ( 1.0 - d[i] * v * delold);
    term = termold * (del - 1.0);
    sum1 = sum1 + term;
    i++;
    delold  = del;
    termold = term;
  }
  return sum1;
}

static f64 See(f64 v)
{
  f64 c[21] =
       {
           0.2,
           9.0 /   35.0,   16.0 /   63.0,
          25.0 /   99.0,   36.0 /  143.0,
          49.0 /  195.0,   64.0 /  255.0,
          81.0 /  323.0,  100.0 /  399.0,
         121.0 /  483.0,  144.0 /  575.0,
         169.0 /  675.0,  196.0 /  783.0,
         225.0 /  899.0,  256.0 / 1023.0,
         289.0 / 1155.0,  324.0 / 1295.0,
         361.0 / 1443.0,  400.0 / 1599.0,
         441.0 / 1763.0,  484.0 / 1935.0
       };
  f64 term, termold, del, delold, sum1, temp, eta, SQRTOpv;
  s32 i;

  SQRTOpv = sqrt(1.0 + v);
  eta     = v / pow(1.0 + SQRTOpv, 2);

  /* ---- Process Forwards ---- */
  delold  = 1.0;
  termold = c[0];  // * eta
  sum1    = termold;
  i = 1;
  while ((i <= 20) && (fabs(termold) > 0.000001))
  {
    del  = 1.0 / (1.0 + c[i] * eta * delold);
    term = termold * (del - 1.0);
    sum1 = sum1 + term;
    i++;
    delold  = del;
    termold = term;
  }

  return ((1.0 / (8.0 * (1.0 + SQRTOpv))) * (3.0 + sum1 / (1.0 + eta * sum1)));
}

/* ----------------------- Lambert techniques -------------------- */
/*------------------------------------------------------------------------------
|
|                           PROCEDURE LAMBERBATTIN
|
|  This PROCEDURE solves Lambert's problem using Battins method. The method is
|    developed in Battin (1987).
|
|  Author        : David Vallado                  303-344-6037    1 Mar 2001
|
|  Inputs          Description                    Range / Units
|    Ro          - IJK Position vector 1          ER
|    R           - IJK Position vector 2          ER
|    DM          - direction of motion            'L','S'
|    DtTU        - Time between R1 and R2         TU
|
|  OutPuts       :
|    Vo          - IJK Velocity vector            ER / TU
|    V           - IJK Velocity vector            ER / TU
|    Error       - Error flag                     'ok',...
|
|  Locals        :
|    i           - Index
|    Loops       -
|    u           -
|    b           -
|    Sinv        -
|    Cosv        -
|    rp          -
|    x           -
|    xn          -
|    y           -
|    l           -
|    m           -
|    CosDeltaNu  -
|    SinDeltaNu  -
|    DNu         -
|    a           -
|    Tan2w       -
|    RoR         -
|    h1          -
|    h2          -
|    Tempx       -
|    eps         -
|    denom       -
|    chord       -
|    k2          -
|    s           -
|    f           -
|    g           -
|    fDot        -
|    am          -
|    ae          -
|    be          -
|    tm          -
|    gDot        -
|    arg1        -
|    arg2        -
|    tc          -
|    AlpE        -
|    BetE        -
|    AlpH        -
|    BetH        -
|    DE          -
|    DH          -
|
|  Coupling      :
|    ARCSIN      - Arc sine FUNCTION
|    ARCCOS      - Arc cosine FUNCTION
|    MAG         - Magnitude of a vector
|    ARCSINH     - Inverse hyperbolic sine
|    ARCCOSH     - Inverse hyperbolic cosine
|    SINH        - Hyperbolic sine
|    POWER       - Raise a base to a POWER
|    ATAN2       - Arc tangent FUNCTION that resolves quadrants
|
|  References    :
|    Vallado       2001, 464-467, Ex 7-5
|
-----------------------------------------------------------------------------*/
void LambertBattin
(
 core::vector3df Ro, core::vector3df R, char dm, char OverRev, f64 DtTU,
 core::vector3df* Vo, core::vector3df* V, char* Error
 )
{
	const f64 Small = 0.000001;

	core::vector3df RCrossR;
	s32   i, Loops;
	f64   u, b, Sinv,Cosv, rp, x, xn, y, L, m, CosDeltaNu, SinDeltaNu,DNu, a,
		tan2w, RoR, h1, h2, Tempx, eps, Denom, chord, k2, s, f, g, FDot, am,
		ae, be, tm, GDot, arg1, arg2, tc, AlpE, BetE, AlpH, BetH, DE, DH;

	strcpy(Error, "ok");
	CosDeltaNu = Ro.dotProduct(R) / (Ro.getLength() * R.getLength());
	RCrossR    = Ro.crossProduct(R);
	SinDeltaNu = RCrossR.getLength() / (Ro.getLength() * R.getLength());
	DNu        = atan2(SinDeltaNu, CosDeltaNu);

	RoR   = R.getLength() / Ro.getLength();
	eps   = RoR - 1.0;
	tan2w = 0.25 * eps * eps / (sqrt(RoR) + RoR *(2.0 + sqrt(RoR)));
	rp    = sqrt(Ro.getLength()*R.getLength()) * (pow(cos(DNu * 0.25), 2) + tan2w);

	if (DNu < core::PI64)
		L = (pow(sin(DNu * 0.25), 2) + tan2w ) /
		(pow(sin(DNu * 0.25), 2) + tan2w + cos(DNu * 0.5));
	else
		L = (pow(cos(DNu * 0.25), 2) + tan2w - cos(DNu * 0.5)) /
		(pow(cos(DNu * 0.25), 2) + tan2w);

	m     = DtTU * DtTU / (8.0 * rp * rp * rp);
	xn    = 0.0;   // 0 for par and hyp
	chord = sqrt(Ro.getLength() * Ro.getLength() + R.getLength() * R.getLength() -
		2.0 * Ro.getLength() * R.getLength() * cos(DNu));
	s     = (Ro.getLength() + R.getLength() + chord) * 0.5;

	Loops = 1;
	while (1 == 1)
	{
		x     = xn;
		Tempx = See(x);
		Denom = 1.0 / ((1.0 + 2.0 * x + L) * (3.0 + x * (1.0 + 4.0 * Tempx)));
		h1    = pow(L + x, 2) * ( 1.0 + (1.0 + 3.0 * x) * Tempx) * Denom;
		h2    = m * ( 1.0 + (x - L) * Tempx) * Denom;

		/* ------------------------ Evaluate CUBIC ------------------ */
		b  = 0.25 * 27.0 * h2 / pow(1.0 + h1, 3);
		u  = -0.5 * b / (1.0 + sqrt(1.0 + b));
		k2 = k(u);

		y  = ((1.0 + h1) / 3.0) * (2.0 + sqrt(1.0 + b) /
			(1.0 - 2.0 * u * k2 * k2));
		xn = sqrt(pow((1.0 - L) * 0.5, 2) + m / (y * y)) - (1.0 + L) * 0.5;

		Loops++;

		if ((fabs(xn - x) < Small) || (Loops > 30))
			break;
	}

	a =  DtTU * DtTU / (16.0 * rp * rp * xn * y * y);
	
	/* -------------------- Find Eccentric anomalies ---------------- */
	/* -------------------------- Hyperbolic ------------------------ */
	if (a < -Small)
	{
		arg1 = sqrt(s / (-2.0 * a));
		arg2 = sqrt((s - chord) / (-2.0 * a));
		/* -------- Evaluate f and g functions -------- */
		
		//Visual Studio misses Hyperbolic Arc
		/*
		AlpH = 2.0 * asinh(arg1);
		BetH = 2.0 * asinh(arg2);
		*/
		AlpH = 2.0 * log(arg1 + sqrt(arg1 * arg1 + 1.0));
		BetH = 2.0 * log(arg2 + sqrt(arg2 * arg2 + 1.0));

		DH   = AlpH - BetH;
		f    = 1.0 - (a / Ro.getLength()) * (1.0 - cosh(DH));
		GDot = 1.0 - (a / R.getLength()) * (1.0 - cosh(DH));
		FDot = -sqrt(-a) * sinh(DH) / (Ro.getLength() * R.getLength());
	}
	else
	{
		/* ------------------------- Elliptical --------------------- */
		if (a > Small)
		{
			arg1 = sqrt(s / (2.0 * a));
			arg2 = sqrt((s - chord) / (2.0 * a));
			Sinv = arg2;
			Cosv = sqrt(1.0 - (Ro.getLength()+R.getLength() - chord) / (4.0 * a));
			BetE = 2.0 * acos(Cosv);
			BetE = 2.0 * asin(Sinv);
			if (DNu > core::PI64)
				BetE = -BetE;

			Cosv = sqrt(1.0 - s / (2.0 * a));
			Sinv = arg1;

			am = s * 0.5;
			ae = core::PI64;
			be = 2.0 * asin(sqrt((s - chord) / s));
			tm = sqrt(am * am * am) * (ae - (be - sin(be)));
			if (DtTU > tm)
				AlpE = 2.0 * core::PI64 - 2.0 * asin(Sinv);
			else
				AlpE = 2.0 * asin(Sinv);
			DE   = AlpE - BetE;
			f    = 1.0 - (a / Ro.getLength()) * (1.0 - cos(DE));
			GDot = 1.0 - (a / R.getLength()) * (1.0 - cos(DE));
			g    = DtTU - sqrt(a * a * a) * (DE - sin(DE));
			FDot = -sqrt(a) * sin(DE) / (Ro.getLength() * R.getLength());
		}
		else
		{
			/* ------------------------- Parabolic -------------------- */
			arg1 = 0.0;
			arg2 = 0.0;
			strcpy(Error, "a = 0 ");
			//if (FileOut != NULL)
				//fprintf(FileOut, " a parabolic orbit \n");
		}
	}
	
	/*
	for (u32 i = 0; i <= 2; i++)
	{
		Vo[i] = (R[i] - f * Ro[i])/ g;
		V[i] = (GDot * R[i] - Ro[i])/ g;
	}
	*/

	Vo->X = (R.X - f * Ro.X)/ g;
	Vo->Y = (R.Y - f * Ro.Y)/ g;
	Vo->Z = (R.Z - f * Ro.Z)/ g;

	V->X = (GDot * R.X - Ro.X)/ g;
	V->Y = (GDot * R.Y - Ro.Y)/ g;
	V->Z = (GDot * R.Z - Ro.Z)/ g;
	
	/*
	if (strcmp(Error, "ok") == 0)
		Testamt = f * GDot - FDot * g;
	else
		Testamt = 2.0;
	*/

	//if (FileOut != NULL)
		//fprintf(FileOut, "%8.5f %3d\n", Testamt, Loops);

	//BigT = sqrt(8.0 / (s * s * s)) * DtTU;
}
/*------------------------------------------------------------------------------
|
|                           PROCEDURE LAMBERTUNIV
|
|  This PROCEDURE solves the Lambert problem for orbit determination and returns
|    the velocity vectors at each of two given position vectors.  The solution
|    uses Universal Variables for calculation and a bissection technique for
|    updating psi.
|
|  Algorithm     : Setting the initial bounds:
|                  Using -8Pi and 4Pi2 will allow single rev solutions
|                  Using -4Pi2 and 8Pi2 will allow multi-rev solutions
|                  The farther apart the initial guess, the more iterations
|                    because of the iteration
|                  Inner loop is for special cases. Must be sure to exit both!
|
|  Author        : David Vallado                  303-344-6037    1 Mar 2001
|
|  Inputs          Description                    Range / Units
|    R1          - IJK Position vector 1          ER
|    R2          - IJK Position vector 2          ER
|    DM          - direction of motion            'L','S'
|    DtTU        - Time between R1 and R2         TU
|
|  OutPuts       :
|    V1          - IJK Velocity vector            ER / TU
|    V2          - IJK Velocity vector            ER / TU
|    Error       - Error flag                     'ok', ...
|
|  Locals        :
|    VarA        - Variable of the iteration,
|                  NOT the semi or axis!
|    Y           - Area between position vectors
|    Upper       - Upper bound for Z
|    Lower       - Lower bound for Z
|    CosDeltaNu  - Cosine of true anomaly change  rad
|    F           - f expression
|    G           - g expression
|    GDot        - g DOT expression
|    XOld        - Old Universal Variable X
|    XOldCubed   - XOld cubed
|    ZOld        - Old value of z
|    ZNew        - New value of z
|    C2New       - C2(z) FUNCTION
|    C3New       - C3(z) FUNCTION
|    TimeNew     - New time                       TU
|    Small       - Tolerance for roundoff errors
|    i, j        - index
|
|  Coupling
|    MAG         - Magnitude of a vector
|    DOT         - DOT product of two vectors
|    FINDC2C3    - Find C2 and C3 functions
|
|  References    :
|    Vallado       2001, 459-464, Alg 55, Ex 7-5
|
-----------------------------------------------------------------------------*/
void LambertUniv
(
 core::vector3df Ro, core::vector3df R, char Dm, char OverRev, f64 DtTU,
 core::vector3df& Vo, core::vector3df& V, char* Error)
{
	const f64 TwoPi   = 2.0 * core::PI64;
	const f64 Small   = 0.0000001;
	const u32 NumIter = 40;

	u32 Loops, i, YNegKtr;
	f64 VarA, Y, Upper, Lower, CosDeltaNu, F, G, GDot, XOld, XOldCubed, FDot,
		PsiOld, PsiNew, C2New, C3New, dtNew;

	/* --------------------  Initialize values   -------------------- */
	strcpy(Error, "ok");
	PsiNew = 0.0;
	Vo = core::vector3df(0,0,0);
	V = core::vector3df(0,0,0);

	CosDeltaNu = Ro.dotProduct(R) / (Ro.getLength() * R.getLength());
	if (Dm == 'L')
		VarA = -sqrt(Ro.getLength() * R.getLength() * (1.0 + CosDeltaNu));
	else
		VarA =  sqrt(Ro.getLength() * R.getLength() * (1.0 + CosDeltaNu));

	/* ----------------  Form Initial guesses   --------------------- */
	PsiOld = 0.0;
	PsiNew = 0.0;
	XOld   = 0.0;
	C2New  = 0.5;
	C3New  = 1.0 / 6.0;

	/* -------- Set up initial bounds for the bissection ------------ */
	if (OverRev == 'N')
	{
		Upper = TwoPi * TwoPi;
		Lower = -4.0 * TwoPi;
	}
	else
	{
		Upper = -0.001 + 4.0 * TwoPi * TwoPi; // at 4, not alw work, 2.0, makes
		Lower =  0.001+TwoPi*TwoPi;           // orbit bigger, how about 2 revs??xx
	}

	/* --------  Determine IF the orbit is possible at all ---------- */
	if (fabs(VarA) > Small)
	{
		Loops   = 0;
		YNegKtr = 1; // y neg ktr
		while (1 == 1)
		{
			if (fabs(C2New) > Small)
				Y = Ro.getLength() + R.getLength() - (VarA * (1.0 - PsiOld * C3New) / sqrt(C2New));
			else
				Y = Ro.getLength() + R.getLength();
			/* ------- Check for negative values of y ------- */
			if ((VarA > 0.0) && (Y < 0.0))
			{
				YNegKtr = 1;
				while (1 == 1)
				{
					PsiNew = 0.8 * (1.0 / C3New) *
						(1.0 - (Ro.getLength() + R.getLength()) * sqrt(C2New) / VarA);

					/* ------ Find C2 and C3 functions ------ */
					FindC2C3(PsiNew, C2New, C3New);
					PsiOld = PsiNew;
					Lower  = PsiOld;
					if (fabs(C2New) > Small)
						Y = Ro.getLength() + R.getLength() -
						(VarA * (1.0 - PsiOld * C3New) / sqrt(C2New));
					else
						Y = Ro.getLength() + R.getLength();
					/*
					if (Show == 'Y')
						if (FileOut != NULL)
							fprintf(FileOut, "%3d %10.5f %10.5f %10.5f %7.3f %9.5f %9.5f\n",
							Loops, PsiOld, Y, XOld, dtNew, VarA, Upper, Lower);
					*/
					YNegKtr++;
					if ((Y >= 0.0) || (YNegKtr >= 10))
						break;
				}
			}

			if (YNegKtr < 10)
			{
				if (fabs(C2New) > Small)
					XOld = sqrt(Y / C2New);
				else
					XOld = 0.0;
				XOldCubed = XOld * XOld * XOld;
				dtNew     = XOldCubed * C3New + VarA * sqrt(Y);

				/* ----  Readjust upper and lower bounds ---- */
				if (dtNew < DtTU)
					Lower = PsiOld;
				if (dtNew > DtTU)
					Upper = PsiOld;
				PsiNew = (Upper + Lower) * 0.5;
				/*
				if (Show == 'Y')
					if (FileOut != NULL)
						fprintf(FileOut, "%3d %10.5f %10.5f %10.5f %7.3f %9.5f %9.5f\n",
						Loops, PsiOld, Y, XOld, dtNew, VarA, Upper, Lower);
				*/
				/* -------------- Find c2 and c3 functions ---------- */
				FindC2C3(PsiNew, C2New, C3New);
				PsiOld = PsiNew;
				Loops++;

				/* ---- Make sure the first guess isn't too close --- */
				if ((fabs(dtNew - DtTU) < Small) && (Loops == 1))
					dtNew = DtTU - 1.0;
			}

			if ((fabs(dtNew - DtTU) < Small) || (Loops > NumIter) || (YNegKtr > 10))
				break;
		}

		if ((Loops >= NumIter) || (YNegKtr >= 10))
		{
			strcpy(Error, "GNotConv");
			if (YNegKtr >= 10)
				strcpy(Error, "Y negative");
		}
		else
		{
			/* ---- Use F and G series to find Velocity Vectors ----- */
			F    = 1.0 - Y / Ro.getLength();
			GDot = 1.0 - Y / R.getLength();
			G    = 1.0 / (VarA * sqrt(Y)); // 1 over G
			FDot = sqrt(Y) * (-R.getLength() - Ro.getLength() + Y) / (R.getLength() * Ro.getLength() * VarA);
			/*
			for (u32 i = 0; i <= 2; i++)
			{
				Vo[i] = (R[i] - F * Ro[i]) * G;
				V[i] = (GDot * R[i] - Ro[i]) * G;
			}
			*/
			Vo.X = (R.X - F * Ro.X) * G;
			Vo.Y = (R.Y - F * Ro.Y) * G;
			Vo.Z = (R.Z - F * Ro.Z) * G;

			V.X = (GDot * R.X - Ro.X) * G;
			V.Y = (GDot * R.Y - Ro.Y) * G;
			V.Z = (GDot * R.Z - Ro.Z) * G;
		}
	}
	else
		strcpy(Error, "impos 180ø");

	/*
	---- For Fig 6-14 dev with testgau.pas ----
	IF Error = 'ok' THEN Write( FileOut,PsiNew:14:7,DtTU*13.44685:14:7 )
	ELSE Write( FileOut,' 9999.0 ':14,DtTU*13.44685:14:7 );
	*/
}