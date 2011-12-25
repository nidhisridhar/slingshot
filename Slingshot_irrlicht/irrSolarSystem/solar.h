#include <string>
//#include <direct.h>
#include <iostream>
#include <irrlicht.h>

using namespace irr;

class SolarSystem
{
public:
	SolarSystem();
	s32 year;
	s32 month;
	s32 day;
	s32 dayM;
	s32 hour;
	s32 minute;
	s32 second;
	f64 OrbitalElements[9][6];
	f64 doublePositions[10][3];
	f64 doubleVelocities[10][3];
	f64 offsetDouble[3];

	f64 gregToJ2000( s32 y, s32 m, s32 d, s32 hour, s32 mins, s32 secs);
	f64 gregToJ( s32 y, s32 m, s32 d, s32 hour, s32 mins, s32 secs);
	core::stringw J2000ToGreg(f64 JD);

	f64 deltaDate( s32 y1, s32 m1, s32 d1, s32 hour1, s32 mins1, s32 secs1, s32 y2, s32 m2, s32 d2, s32 hour2, s32 mins2, s32 secs2 );
	f64 deltaMidDay();

	void updateAtDay(f64 d, s32 p);
	core::array<scene::SMeshBuffer*> calculateFieldLines(s32 year, s32 dayM, s32 hour, s32 minute, s32 second);
	~SolarSystem();

private:
	void calculateMeanElements(s32 i, f64 d );
};
