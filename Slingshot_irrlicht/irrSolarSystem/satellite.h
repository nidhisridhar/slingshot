#include <string>
//#include <direct.h>
#include <iostream>
#include <irrlicht.h>

using namespace irr;

class Satellite
{
public:
	Satellite();
	void updateAtDay(f64 d);
	f64 OrbitalElementsAtEpoch[9];
	f64 OrbitalElementsNow[9];
	f64 doublePositions[3];
	f64 doubleVelocities[3];
	f64 offsetDouble[3];
	~Satellite();

private:

};
