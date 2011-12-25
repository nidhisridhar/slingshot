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
#include <irrlicht.h>
#include "satellite.h"
#include "kepler.h"
#include "doubleVectorOps.h"

using namespace irr;

//Constructor
Satellite::Satellite()
//: year(0), month(0), day(0), dayM(0), hour(0), minute(0), second(0)
{
}

//Destructor
Satellite::~Satellite()
{
}

void Satellite::updateAtDay(f64 d)
{
}