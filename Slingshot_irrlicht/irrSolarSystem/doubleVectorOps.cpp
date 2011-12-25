#include <irrlicht.h>
#include <vector>

using namespace std;
using namespace irr;

f64 dot(f64 nvector1[3], f64 nvector2[3])
{
	return nvector1[0]*nvector2[0] + nvector1[1]*nvector2[1] + nvector1[2]*nvector2[2];
}
f64  mag(f64 x[3])
{
	return sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
}  // end mag
void cross(f64 vec1[3], f64 vec2[3], f64 outvec[3])
{
	 outvec[0]= vec1[1]*vec2[2] - vec1[2]*vec2[1];
	 outvec[1]= vec1[2]*vec2[0] - vec1[0]*vec2[2];
	 outvec[2]= vec1[0]*vec2[1] - vec1[1]*vec2[0];
}

void makeUnit(f64 x[3])
{
	f64 length = x[0]*x[0] + x[1]*x[1] + x[2]*x[2];
	
	if (core::equals(length, 0.0)) // this check isn't an optimization but prevents getting NAN in the sqrt.
		return;
	
	length = core::reciprocal_squareroot(length);

	x[0] = (x[0] * length);
	x[1] = (x[1] * length);
	x[2] = (x[2] * length);
	
}

//! Sets the length of the vector to a new value
void setLengthDouble(f64 newlength, f64 x[3])
{
	makeUnit(x);
	x[0] = (x[0] * newlength);
	x[1] = (x[1] * newlength);
	x[2] = (x[2] * newlength);
}

//! Builds a direction vector from (this) rotation vector.
/** This vector is assumed to be a rotation vector composed of 3 Euler angle rotations, in degrees.
The implementation performs the same calculations as using a matrix to do the rotation.
\param[in] forwards  The direction representing "forwards" which will be rotated by this vector.
If you do not provide a direction, then the +Z axis (0, 0, 1) will be assumed to be forwards.
\return A direction vector calculated by rotating the forwards direction by the 3 Euler angles
(in degrees) represented by this vector. */
void rotationToDirectionDouble(f64 forwards[3], f64 input[3], f64 output[3]) 
{
	const f64 cr = cos( core::DEGTORAD64 * input[0] );
	const f64 sr = sin( core::DEGTORAD64 * input[0] );
	const f64 cp = cos( core::DEGTORAD64 * input[1] );
	const f64 sp = sin( core::DEGTORAD64 * input[1] );
	const f64 cy = cos( core::DEGTORAD64 * input[2] );
	const f64 sy = sin( core::DEGTORAD64 * input[2] );

	const f64 srsp = sr*sp;
	const f64 crsp = cr*sp;

	const f64 pseudoMatrix[] = {
		( cp*cy ), 
		( cp*sy ), 
		( -sp ),
		( srsp*cy-cr*sy ), 
		( srsp*sy+cr*cy ), 
		( sr*cp ),
		( crsp*cy+sr*sy ), 
		( crsp*sy-sr*cy ), 
		( cr*cp )
	};

	
		output[0] = (forwards[0] * pseudoMatrix[0] +
					forwards[1] * pseudoMatrix[3] +
					forwards[2] * pseudoMatrix[6]);
			
		output[1] = (forwards[0] * pseudoMatrix[1] +
					forwards[1] * pseudoMatrix[4] +
					forwards[2] * pseudoMatrix[7]);
			
		output[2] = (forwards[0] * pseudoMatrix[2] +
					forwards[1] * pseudoMatrix[5] +
					forwards[2] * pseudoMatrix[8]);
}
//original
/*
vector3d<T> getHorizontalAngle() const
{
	vector3d<T> angle;

	const f64 tmp = (atan2((f64)X, (f64)Z) * RADTODEG64);
	angle.Y = (T)tmp;

	if (angle.Y < 0)
		angle.Y += 360;
	if (angle.Y >= 360)
		angle.Y -= 360;

	const f64 z1 = core::squareroot(X*X + Z*Z);

	angle.X = (T)(atan2((f64)z1, (f64)Y) * RADTODEG64 - 90.0);

	if (angle.X < 0)
		angle.X += 360;
	if (angle.X >= 360)
		angle.X -= 360;

	return angle;
}
*/
void getAngle(f64 input[3], f64 angle[3])
{
	const f64 tmp = (atan2(input[0], input[2]) * core::RADTODEG64);
	angle[1] = tmp;

	if (angle[1] < 0)
		angle[1] += 360;
	if (angle[1] >= 360)
		angle[1] -= 360;

	const f64 z1 = core::squareroot(input[0]*input[0] + input[2]*input[2]);

	angle[0] = (atan2(z1, input[1]) * core::RADTODEG64 - 90.0);

	if (angle[0] < 0)
		angle[0] += 360;
	if (angle[0] >= 360)
		angle[0] -= 360;
}
core::array<f64> getAngle(core::array<f64> input)
{
	f64 angle[3];

	const f64 tmp = (atan2(input[0], input[2]) * core::RADTODEG64);
	angle[1] = tmp;

	if (angle[1] < 0)
		angle[1] += 360;
	if (angle[1] >= 360)
		angle[1] -= 360;

	const f64 z1 = core::squareroot(input[0]*input[0] + input[2]*input[2]);

	angle[0] = (atan2(z1, input[0]) * core::RADTODEG64 - 90.0);

	if (angle[0] < 0)
		angle[0] += 360;
	if (angle[0] >= 360)
		angle[0] -= 360;
	
	core::array<f64> angleA;
	angleA.push_back(angle[0]);
	angleA.push_back(angle[1]);
	angleA.push_back(0.0);

	return angleA;
}

f64 dot(core::array<f64> nvector1, core::array<f64> nvector2)
{
	return nvector1[0]*nvector2[0] + nvector1[1]*nvector2[1] + nvector1[2]*nvector2[2];
}
f64  mag(core::array<f64> x)
{
	return sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
}  // end mag
void cross(core::array<f64> vec1, core::array<f64> vec2, core::array<f64> outvec)
{
	 outvec[0]= vec1[1]*vec2[2] - vec1[2]*vec2[1];
	 outvec[1]= vec1[2]*vec2[0] - vec1[0]*vec2[2];
	 outvec[2]= vec1[0]*vec2[1] - vec1[1]*vec2[0];
}

//! Rotates the vector by a specified number of degrees around the Y axis and the specified center.
/** \param degrees Number of degrees to rotate around the Y axis.
\param center The center of the rotation. */
void rotateAroundYBy(core::array<f64> input, f64 degrees, core::array<f64> center)
{
	degrees *= core::DEGTORAD64;
	f64 cs = cos(degrees);
	f64 sn = sin(degrees);
	input[0] = input[0] - center[0];
	input[2] = input[2] - center[2];
	
	input[0] = input[0] * cs - input[2] * sn;
	input[1] = input[1];
	input[2] = input[0] * sn + input[2] * cs;
	
	input[0] = input[0] + center[0];
	input[2] = input[2] + center[2];
}

//! Rotates the vector by a specified number of degrees around the Z axis and the specified center.
/** \param degrees: Number of degrees to rotate around the Z axis.
\param center: The center of the rotation. */
void rotateAroundZBy(core::array<f64> input, f64 degrees, core::array<f64> center)
{
	degrees *= core::DEGTORAD64;
	f64 cs = cos(degrees);
	f64 sn = sin(degrees);
	input[0] = input[0] - center[0];
	input[1] = input[1] - center[1];
	
	input[0] = input[0] * cs - input[1] * sn;
	input[1] = input[0] * sn + input[1] * cs;
	input[2] = input[2];
	
	input[0] = input[0] + center[0];
	input[1] = input[1] + center[1];
}

//! Rotates the vector by a specified number of degrees around the X axis and the specified center.
/** \param degrees: Number of degrees to rotate around the X axis.
\param center: The center of the rotation. */
void rotateAroundXBy(core::array<f64> input, f64 degrees, core::array<f64> center)
{
	degrees *= core::DEGTORAD64;
	f64 cs = cos(degrees);
	f64 sn = sin(degrees);
	input[2] = input[2] - center[2];
	input[1] = input[1] - center[1];
	
	input[0] = input[0];
	input[1] = input[1] * cs - input[2] * sn;
	input[2] = input[1] * sn + input[2] * cs;
	
	input[2] = input[2] + center[2];
	input[1] = input[1] + center[1];
}

//! Rotates the vector by a specified number of degrees around the Y axis and the specified center.
/** \param degrees Number of degrees to rotate around the Y axis.
\param center The center of the rotation. */
void rotateAroundYBy(f64 input[3], f64 degrees, f64 center[3])
{
	degrees *= core::DEGTORAD64;
	f64 cs = cos(degrees);
	f64 sn = sin(degrees);
	input[0] = input[0] - center[0];
	input[2] = input[2] - center[2];
	
	input[0] = input[0] * cs - input[2] * sn;
	input[1] = input[1];
	input[2] = input[0] * sn + input[2] * cs;
	
	input[0] = input[0] + center[0];
	input[2] = input[2] + center[2];
}

//! Rotates the vector by a specified number of degrees around the Z axis and the specified center.
/** \param degrees: Number of degrees to rotate around the Z axis.
\param center: The center of the rotation. */
void rotateAroundZBy(f64 input[3], f64 degrees, f64 center[3])
{
	degrees *= core::DEGTORAD64;
	f64 cs = cos(degrees);
	f64 sn = sin(degrees);
	input[0] = input[0] - center[0];
	input[1] = input[1] - center[1];
	
	input[0] = input[0] * cs - input[1] * sn;
	input[1] = input[0] * sn + input[1] * cs;
	input[2] = input[2];
	
	input[0] = input[0] + center[0];
	input[1] = input[1] + center[1];
}

//! Rotates the vector by a specified number of degrees around the X axis and the specified center.
/** \param degrees: Number of degrees to rotate around the X axis.
\param center: The center of the rotation. */
void rotateAroundXBy(f64 input[3], f64 degrees, f64 center[3])
{
	degrees *= core::DEGTORAD64;
	f64 cs = cos(degrees);
	f64 sn = sin(degrees);
	input[2] = input[2] - center[2];
	input[1] = input[1] - center[1];
	
	input[0] = input[0];
	input[1] = input[1] * cs - input[2] * sn;
	input[2] = input[1] * sn + input[2] * cs;
	
	input[2] = input[2] + center[2];
	input[1] = input[1] + center[1];
}


//------------------------------------------------------------------------------------------------
core::vector3df double2V(core::array<f64> in)
{
	core::vector3df returnV;
	returnV = core::vector3df(in[0],in[1],in[2]);
	return returnV;
}
core::vector3df double2V(f64 in[3])
{
	core::vector3df returnV;
	returnV = core::vector3df(in[0],in[1],in[2]);
	return returnV;
}


core::array<f64> doubleToIrrArray(f64 in[3])
{
	core::array<f64> out;
	out.push_back(in[0]);
	out.push_back(in[1]);
	out.push_back(in[2]);
	return out;
}

void axisSwap(f64 r[3], f64 v[3])
{

	f64 axisSwapR[3];
	f64 axisSwapV[3];

	//doh
	axisSwapR[0] = r[0];
	axisSwapR[1] = r[1];
	axisSwapR[2] = r[2];

	axisSwapV[0] = v[0];
	axisSwapV[1] = v[1];
	axisSwapV[2] = v[2];
	
	r[0] = axisSwapR[0];
	r[1] = axisSwapR[2];
	r[2] = axisSwapR[1];

	v[0] = axisSwapV[0];
	v[1] = axisSwapV[2];
	v[2] = axisSwapV[1];

}
void helioToLocal(f64 r[3])
{
	f64 axisSwapR[3];
	//doh
	axisSwapR[0] = r[0];
	axisSwapR[1] = r[1];
	axisSwapR[2] = r[2];

	r[0] = axisSwapR[0];
	r[1] = axisSwapR[2];
	r[2] = axisSwapR[1];
}
void rotateAroundCentre(core::vector3df &point, core::vector3df center, core::vector3df rotation)
{
	point -= center;
	core::matrix4 m;
	m.setRotationDegrees(rotation);
	m.rotateVect(point);
	point += center;
}
void rotateVectorAroundAxis(core::vector3df & vector, const core::vector3df & axis, f64 radians)
{
	core::quaternion MrQuaternion;
	core::matrix4 MrMatrix;
	(void)MrQuaternion.fromAngleAxis(radians, axis);
	MrQuaternion.getMatrix(MrMatrix,core::vector3df(0,0,0));
	MrMatrix.rotateVect(vector);
}
