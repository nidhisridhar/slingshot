// ------------------------------------------------------------------------ //
// This source file is part of the 'ESA Advanced Concepts Team's			//
// Space Mechanics Toolbox' software.                                       //
//                                                                          //
// The source files are for research use only,                              //
// and are distributed WITHOUT ANY WARRANTY. Use them on your own risk.     //
//                                                                          //
// Copyright (c) 2004-2006 European Space Agency                            //
// ------------------------------------------------------------------------ //

#ifndef PROPAGATEKEP_H
#define PROPAGATEKEP_H


#include "Astro_Functions.h"

//The universal gravitational constant (G), in units of (km^3 kg^-1 s^-2).
const double GconstKM = 6.67428e-20;  
//The universal gravitational constant (G), in units of (m^3 kg^-1 s^-2).
const double Gconst = 6.67428e-11; 
//The GM Product with the Sun (km^3 kg^-1 s^-2).
const double mu = GconstKM * 1989100000e21;
//The GM Product with the Sun (m^3 kg^-1 s^-2).
const double mu2 = Gconst * 1989100000e21; 

//1.32712428e11

const double AU = 149598000;

void propagateKEPInOut( double *, double *, double, double);
void propagateKEP(const double *, const double *, double, double,
				  double *, double *);

void IC2par(const double*, const double*, double, double*);

void par2IC(const double*, double, double*, double*);

// Returns the cross product of the vectors X and Y.
// That is, z = X x Y.  X and Y must be 3 element
// vectors.
void cross(const double*, const double*, double*);

#endif




