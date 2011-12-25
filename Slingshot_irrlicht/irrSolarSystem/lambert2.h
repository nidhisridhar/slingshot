#include <irrlicht.h>

using namespace irr;

void LambertBattin(
 core::vector3df Ro, core::vector3df R, char dm, char OverRev, f64 DtTU,
 core::vector3df* Vo, core::vector3df* V, char* Error);

void LambertUniv(
 core::vector3df Ro, core::vector3df R, char Dm, char OverRev, f64 DtTU,
 core::vector3df* Vo, core::vector3df* V, char* Error);