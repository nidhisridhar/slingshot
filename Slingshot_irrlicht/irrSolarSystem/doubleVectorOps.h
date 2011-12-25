#include <irrlicht.h>

using namespace irr;

//! Typedef for a f64 3d vector. 
//typedef core::vector3d<f64> vector3dd;

f64 dot(f64 nvector1[3], f64 nvector2[3]);
f64  mag(f64 x[3]);
void cross(f64 vec1[3], f64 vec2[3], f64 outvec[3]);
void getAngle(f64 input[3], f64 angle[3]);
void setLengthDouble(f64 newlength, f64 x[3]);
void makeUnit(f64 x[3]);
void rotationToDirectionDouble(f64 forwards[3], f64 input[3], f64 output[3]);

f64 dot(core::array<f64> nvector1, core::array<f64> nvector2);
f64 mag(core::array<f64> x);
void cross(core::array<f64> vec1, core::array<f64> vec2, core::array<f64> outvec);
core::array<f64> getAngle(core::array<f64> input);


void rotateAroundXBy(core::array<f64> input, f64 degrees, core::array<f64> center);
void rotateAroundYBy(core::array<f64> input, f64 degrees, core::array<f64> center);
void rotateAroundZBy(core::array<f64> input, f64 degrees, core::array<f64> center);
void rotateAroundXBy(f64 input[3], f64 degrees, f64 center[3]);
void rotateAroundYBy(f64 input[3], f64 degrees, f64 center[3]);
void rotateAroundZBy(f64 input[3], f64 degrees, f64 center[3]);

core::vector3df double2V(core::array<f64> in);
core::vector3df double2V(f64 in[3]);

core::array<f64> doubleToIrrArray(f64 in[3]);

void axisSwap(f64 r[3], f64 v[3]);
void helioToLocal(f64 r[3]);

void rotateVectorAroundAxis(core::vector3df & vector, const core::vector3df & axis, f64 radians);
void rotateAroundCentre(core::vector3df &point, core::vector3df center, core::vector3df rotation);
