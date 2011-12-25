#include <irrlicht.h>
#include "doubleVectorOps.h"

using namespace irr;

f64 s = 0.0;
f64 c = 0.0;
f64 tof = 0.0;
f64 mu = 0.0;
bool aflag = false;
bool bflag = false;
bool debug_print = false;

f64 getalpha(f64 a)
{
	f64 alpha = 2.0 * asin(sqrt(s / (2.0 * a)));
	if (aflag)
	{
		alpha = 2.0 * core::PI64 - alpha;
	}
	return alpha;
}

f64 getbeta(f64 a)
{
	f64 beta = 2.0 * asin(sqrt((s - c) / (2.0 * a)));
	if (bflag)
	{
		beta = -1.0 * beta;
	}
	return beta;
}

f64 getdt(f64 a, f64 alpha, f64 beta)
{
	f64 sa = sin(alpha);
	f64 sb = sin(beta);
	f64 dt = pow(a, 1.5) * (alpha - sa - beta + sb) / sqrt(mu);
	return dt;
}

/** Evaluate the delta-t function f
* @param a semi-major axis.
* @return
*/
f64 evaluate(f64 a)
{
	f64 alpha = getalpha(a);
	f64 beta = getbeta(a);
	f64 out = tof - getdt(a,alpha,beta);
	return out;
}

/** Find the solution using RegulaFalsi.
 * @param x1 lower limit on x.
 * @param x2 upper limit on x.
 * @param dxmin
 * @return
 */
f64 regulaFalsi(f64 x1, f64 x2) {
	f64 xlow;
	f64 xhigh;
	f64 del;
	f64 out = 0.0;
	f64 f;
	
	u32 err = 0;
	f64 dxmin = 1.0e-15;
	f64 accuracy = 1.0e-6;

	/*NO IDEA !

	f64 fl = this.func.evaluate(x1);
	f64 fh = this.func.evaluate(x2);

	//from JAT ZeroFinder:
	
	* The ZeroFinder Class provides a way to solve a scalar f(x) = 0.
	* These functions have been translated from Numerical Recipes.
	* Currently there are: Regula Falsi, Secant and Ridder's methods.
	* The function f is passed via the ScalarFunction interface.

	//from JAT Scalarfunction:

	* The ScalarFunction interface provides the mechanism for passing a method
	* that evaluates a function to a solver.
	*/
	
	//HOPE THIS WORKS ?!?
	f64 fl = evaluate(x1);
	f64 fh = evaluate(x2);

	f64 test = fl * fh;

	if (test > 0.0) {
		if (fl == 0.0) return x1;
		if (fh == 0.0) return x2;
		err++;
		printf("Root must be bracketed in ZeroFinder\n");
	}

	if (fl < 0.0) 
	{
		xlow = x1;
		xhigh = x2;
	} 
	else 
	{
		xlow = x2;
		xhigh = x1;
		f64 temp = fl;
		fl = fh;
		fh = temp;
	}

	f64 dx = xhigh - xlow;

	for(int i = 1; i < 10000; i++) 
	{
		out = xlow + dx * fl / (fl - fh);
		//Same as above, hope it works...
		//f = this.func.evaluate(out);
		f = evaluate(out);

		if (f < 0.0) {
			del = xlow - out;
			xlow = out;
			fl = f;
		} else {
			del = xhigh - out;
			xhigh = out;
			fh = f;
		}

		dx = xhigh - xlow;

		if ((abs(del) < dxmin) || (abs(f) < accuracy)) {
			return out;
		}
	}

	printf("Regula Falsi exceeded 10000 iterations\n");
	return out;
}

/** Computes the delta-v's required to go from r0,v0 to rf,vf.
* @return Total delta-v (magnitude) required.
* @param dt Time of flight
* @param r0 Initial position vector.
* @param v0 Initial velocity vector.
* @param rf Desired final position vector.
* @param vf Desired final velocity vector.
* //pointers to return results
* @param deltaV0 computed Initial delta-V.
* @param deltaV1 computed Final delta-V.
* @param totalV0 computed Initial total-V.
* @param totalV0 computed Final total-V.
*/
void LambertCompute(f64 GM, 
					f64 r0[],
					f64 v0[],
					f64 rf[],
					f64 vf[], 
					f64 dt,
					f64 deltaV0[],
					f64 deltaV1[],
					f64 totalV0[],
					f64 totalV1[])
{
	//helioToLocal(v0);
	//helioToLocal(vf);

	s = 0.0;
	c = 0.0; //chord Length between r0 and rf
	aflag = false;
	bflag = false;
	
	f64 tp = 0.0;
	f64 magr0 = mag(r0);
	f64 magrf = mag(rf);
	
	//GM is expected in km^3/s^2
	mu = GM / 1000000000.0;
	//time of flight expected in seconds
	dt *= 86400;
	tof = dt;
	
	core::array<f64> dr;
	dr.push_back(r0[0] - rf[0]);
	dr.push_back(r0[1] - rf[1]);
	dr.push_back(r0[2] - rf[2]);

	c = mag(dr);
	s = (magr0 + magrf + c) / 2.0;
	f64 amin = s / 2.0;
	
	if(debug_print)
		printf("amin = %.9E\n", amin);
	
	f64 dtheta = acos(dot(r0,rf) / (magr0 * magrf));

	//dtheta = 2.0 * core::PI64 - dtheta;

	if(debug_print)
		printf("dtheta = %.9E\n", dtheta);
	
	if (dtheta < core::PI64)
	{
		tp = sqrt(2.0 / (mu)) * (pow(s, 1.5) - pow(s - c, 1.5)) / 3.0;
	}
	if (dtheta > core::PI64)
	{
		tp = sqrt(2.0 / (mu)) * (pow(s, 1.5) + pow(s - c, 1.5)) / 3.0;
		bflag = true;
	}
	if (dtheta == core::PI64)
	{
		printf(" dtheta = 180.0. Do a Hohmann\n");
		return;
	}

	if(debug_print)
		printf("tp = %.9f\n", tp);

	f64 betam = getbeta(amin);
	f64 tm = getdt(amin, core::PI64, betam);

	if(debug_print)
		printf("tm = %.9E\n", tm);

	f64 ahigh = 1000.0 * amin;
	f64 npts = 3000.0;
	
	if(debug_print)
		printf("dt = %.9E seconds\n", dt);

	if(debug_print)
		printf("************************************************\n");

	if (dt < tp)
	{
		printf(" No elliptical path possible \n");
		return;
	}

	if (dt > tm)
	{
		aflag = true;
	}

	f64 fm = evaluate(amin);
	f64 ftemp = evaluate(ahigh);

	if ((fm * ftemp) >= 0.0)
	{
		printf(" initial guesses do not bound \n");
		return;
	}

	//ZeroFinder regfalsi = new ZeroFinder(this, 10000, 1.0E-6, 1.0E-15);

	f64 sma = regulaFalsi(amin, ahigh);

	f64 alpha = getalpha(sma);
	f64 beta = getbeta(sma);
	
	//printf("\n\nbeta: %.9f\n\n",beta*core::RADTODEG64);

	f64 de = alpha - beta;
	
	f64 f = 1.0 - (sma / magr0) * (1.0 - cos(de));
	f64 g = dt - sqrt(sma * sma * sma / mu) * (de - sin(de));

	core::array<f64> newv0;
	core::array<f64> newvf;

	newv0.push_back((rf[0] - f * r0[0]) / g);
	newv0.push_back((rf[1] - f * r0[1]) / g);
	newv0.push_back((rf[2] - f * r0[2]) / g);
	
	//if it wont work for you, take away the -1.0 multiplications
	//I guess my coordinate system is little screwed...
	deltaV0[0] = ((newv0[0] - v0[0]));
	deltaV0[1] = ((newv0[1] - v0[1]));
	deltaV0[2] = ((newv0[2] - v0[2]));

	totalV0[0] = (newv0[0]);
	totalV0[1] = (newv0[1]);
	totalV0[2] = (newv0[2]);
	
	/*
	f64 DV0X = deltaV0[0];
	f64 DV0Y = deltaV0[1];
	f64 DV0Z = deltaV0[2];

	f64 TV0X = totalV0[0];
	f64 TV0Y = totalV0[1];
	f64 TV0Z = totalV0[2];

	f64 NV0mag = mag(newv0);
	*/

	//if(debug_print)
		//printf("deltav-0 X=%.9f, Y=%.9f, Z=%.9f\n",deltaV0[0],deltaV0[1],deltaV0[2]);

	f64 dv0 = mag(deltaV0);

	f64 fdot = -1.0 * (sqrt(mu * sma) / (magr0 * magrf)) * sin(de);
	f64 gdot = 1.0 - (sma / magrf) * (1.0 - cos(de));

	newvf.push_back(fdot * r0[0] + gdot * newv0[0]);
	newvf.push_back(fdot * r0[1] + gdot * newv0[1]);
	newvf.push_back(fdot * r0[2] + gdot * newv0[2]);
	
	//Same here:
	//if it wont work for you, take away the -1.0 multiplications
	//I guess my coordinate system is little screwed...
	deltaV1[0] = ((vf[0] - newvf[0]));
	deltaV1[1] = ((vf[1] - newvf[1]));
	deltaV1[2] = ((vf[2] - newvf[2]));
	
	totalV1[0] = (newvf[0]);
	totalV1[1] = (newvf[1]);
	totalV1[2] = (newvf[2]);

	//if(debug_print)
		//printf("deltav-f X=%.9f, Y=%.9f, Z=%.9f\n",deltaV1[0],deltaV1[1],deltaV1[2]);

	f64 dvf = mag(deltaV1);
	f64 totaldv = dv0 + dvf;

	printf("\n\nInitial DeltaV dv0 = %.9f\nFinal DeltaV   dvf = %.9f\nTotal DeltaV    dv = %.9f\nSemi Major Axis    = %.9f\n\n",dv0,dvf,totaldv,sma);
		
	/*debug
	printf("************************************************\n");
	printf("alpha = %.9f\n",alpha);
	printf("beta = %.9f\n",beta);
	printf("de = %.9f\n",de);
	printf("f = %.9f\n",f);
	printf("g = %.9f\n",g);
	printf("mu = %.9E\n",mu);

	f64 firstPartOfG = sqrt(sma * sma * sma / mu);
	f64 secondPartOfG = (de - sin(de));

	printf("firstPart = %.9f\n",firstPartOfG);
	printf("secondPart = %.9f\n",secondPartOfG);
	
	f64 firstTimesSecond = firstPartOfG * secondPartOfG;
	f64 timeMinusfirstTimesSecond = dt - test;

	printf("firstTimesSecond = %.9E\n",firstTimesSecond);
	printf("timeMinusfirstTimesSecond = %.9f\n",timeMinusfirstTimesSecond);

	printf("start X=%.9f, Y=%.9f, Z=%.9f\n",r0[0],r0[1],r0[2]);
	printf("desti X=%.9f, Y=%.9f, Z=%.9f\n\n",rf[0],rf[1],rf[2]);
	
	printf("start Length: %.9Ef\n",r0.getLength());
	printf("desti Length: %.9Ef\n\n",rf.getLength());
	*/
}