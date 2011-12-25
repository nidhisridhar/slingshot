/*
The Irrlicht Engine License

Copyright � 2002-2005 Nikolaus Gebhardt

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

//Solar System Simulator
//written by Tobias Houfek
//you have to tell your compiler to link against libf2c

#include <irrlicht.h>
#include <string>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <cmath>
#include <ctime>
#include <stdio.h>
#include <vector>

#include "solar.h"
#include "lunar.h"
#include "kepler.h"
#include "doubleVectorOps.h"
#include "CRTTSkyBoxSceneNode.h"
#include "IndexedPrimitiveNode.h"
#include "bsc.h"
#include "bsc2.h"
#include "Geopack-2008.h"
#include "f2c.h"

#include "cubeSphere.h"
#include "AstroFileReader.h"

#include "GTOPtoolbox/trajobjfuns.h"
#include "GTOPtoolbox/Lambert.h"
#include "GTOPtoolbox/propagateKEP.h"

#define isnan(x) ((x) != (x))

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#endif

using namespace irr;
using namespace gui;
//using namespace irrklang;

IrrlichtDevice* device;
SIrrlichtCreationParameters param;
video::IVideoDriver* driver;
scene::ISceneManager* smgr;
scene::ICameraSceneNode* flyCam;
scene::ICameraSceneNode* splitSCam;
scene::ICameraSceneNode* mayaCam;
scene::ICameraSceneNode* starCam;
scene::ICameraSceneNode* moonCam;

scene::ILightSceneNode* light;

IGUIEnvironment* env;
ICursorControl* cursor;
IGUIFont* fnt;

IGUIStaticText* planningGUI;
IGUIScrollBar* timestepper;
IGUIButton* editbutton;
IGUIButton* burnbutton;
IGUIEditBox* burnUntil;

IGUIStaticText* flightGUI;
IGUIStaticText* info;
IGUIStaticText* info2;

IGUIImage* flightaid;

SolarSystem* SS = new SolarSystem();

//altitude for geo synchronous orbit with earth
//should be 35786 km <-Wikipedia
//debugger says altitude 35862.678092090246	f64
//precision and testing says ~ 36460.0f;
//numbers are in a really realistic range.... :)
u32 startplanet = 2;
f64 startAlt = 35786;
f64 getAway =  35786;

//Day += (1/86400)*timescale; <-- is to small for f64
const f64 incrementF = 1.1574074074074074074074074074074e-5;

const f64 dayV = 24.0;
const f64 minSecV = 60.0;
const f64 scaleF = 100000.0;


s32 normalSpec;

//view 0 = main
//view 1 = miniMap
//view 2 = starCam
//view 3 = moonCam
u32 view = 0;

bool updateFieldLines = true;
bool win = false;
bool planningM = false;
bool projected = true;
bool updateVar = false;
bool trailAbsolute = false;
bool displayFieldLines = false;
bool SOIchanged = false;
bool showSOI = false;
bool jumped = true;
bool planesVisible = false;

f32 jumpWait = 0;

bool up=false,down=false,leftBool=false,rightBool=false,keyQ=false,keyE=false,
leftB=false,rightB=false,mouseW=false,upA=false,
downA=false,rightA=false,leftA=false,
key1=false,key2=false,key3=false,key4=false,key5=false,key6=false,key7=false,key8=false,key9=false,key0=false,
keyF1=false,keyF2=false,keyF3=false,key_shift=false,key_ctrl=false,key_p=false,editB=false,burnB=false,keyM=false,
key_hyphen=false,keyT=false,KeyF=false,KeyI=false;

core::position2d<f32> CursorPos;
core::vector2df deltaC;
int Syear=2012,Smonth=12,Sday=21,Shour=12,Sminute=0,Ssecond=0;

u32 introseconds = 0;
u32 introcount = 0;

u32 Width;
u32 Height;
u32 trailcount = 0;
u32 trailfirst = 0;
s32 closestIndex;
s32 closestPlanet;
u32 oldIndex;
const u32 totalTrailSize = 800;

u32 projectionCounter;
u32 timestepperPos;

u32 timefactor = 1;
u32 currentCount = 0;

f64 lastWheelMovement;
f64 Day = 0;
f64 timescale = 10000;
f64 delta;
f64 fuckUPdelta = 3.6;
f64 fpsincrement;

f64 sunDistanceKM;
f64 gforce;
f64 speed;
f64 oSpeed;

f64 CspeedUD = 0.15;
f64 CspeedLR = 0.15;
f64 CspeedFB = 0.25;
f64 camSpeedr = 0;

f64 closestDistance;
f64 closestPDistance;

f32 camOrbitDistance;
f32 initialCamOrbitDistance;

scene::CRTTSkyBoxSceneNode* sky;
scene::ISceneNode* closestBody;
scene::ISceneNode* oldBody;


scene::ISceneNode* departureMarker;
scene::ISceneNode* arrivalMarker;

scene::IBillboardSceneNode* flightmarker;
scene::IBillboardSceneNode* flightmarker2;

scene::ISceneNode* allstars;
scene::ISceneNode* FieldLineNode;
scene::ISceneNode* FieldLineNodeGSW;

scene::ISceneNode * forceA;
scene::ISceneNode * velA;
scene::ISceneNode * tanA;

scene::ISceneNode* orbitsM;
scene::ISceneNode* axis;
scene::ISceneNode* checkmesh;

core::array<scene::ISceneNode*> planets;
core::array<scene::ISceneNode*> planetsR;
core::array<scene::ISceneNode*> moons;
core::array<scene::ISceneNode*> moonsR;
core::array<scene::ISceneNode*> SOINodes;
core::array<scene::ISceneNode*> cassiniPoints;
core::array<scene::IBillboardSceneNode*> perihelions;
core::array<scene::IBillboardSceneNode*> ascendingNodes;
core::array< core::array<scene::ISceneNode*> > OrbitPlanes;
core::array<scene::ISceneNode*> currentOrbitalPlane;

core::array<core::vector3df> orbitalPlaneNormals;
core::vector3df toSun;
core::vector3df toClosestBody;

core::array<core::vector3df> absTrail;
core::array<scene::ISceneNode*> trail;
CIndexedPrimitiveNode* trailLine;
CIndexedPrimitiveNode* trailLineAbs;


video::ITexture *bluedot;
video::ITexture *greendot;

core::dimension2df dotsize = core::dimension2df(0.003,0.003);

core::array<f64> planetSize;
core::array<f64> pMass;
core::array<f64> pRot;
core::array<f64> mDist;
core::array<f64> mSize;
core::array<f64> mFOrb;
core::array<f64> mEcc;
core::array<f64> mMass;
core::array<f64> orbitT;
core::array<f64> pMeanDist;
core::array<f64> pMeanDist2;
core::array<f64> pSOI;

core::array<core::stringw> randomFacts;
core::array<core::stringw> intro;
core::array<core::stringw> planetNames;
core::array<video::SColor> OrbitCol;

core::array< core::array<f64> > projectionPosition;
core::array< core::array<f64> > projectionVelocity;
core::array< core::array<f64> > projectionRelativeVel;
core::array< core::array<f64> > projectionRelativePosition;
core::array< core::array<f64> > projectionKeps;
core::array<f64> projectionDistance;
core::array<f64> projectionDay;
core::array<u32> projectionClosestIndex;

//cassini test
f64 cassini_r[4][3];
f64 cassini_v[4][3];
f64 cassiniStartDay;

scene::ISceneNode* flightpathLine;
scene::ISceneNode* debugPlane;

f64 projectedPosition[3];
f64 projectedRelativePosition[3];
f64 projectedVelocity[3];
f64 projectedRelativeVel[3];
f64 projectedKeps[6];
f64 projectedDistance = 0;
f64 projectedDay = 0;

f64 doublePosition[3];
f64 doubleVelocity[3];
f64 doubleRelativePosition[3];
f64 doubleRelativeVelocity[3];
f64 currentKeps[6];

f64 offsetDouble[3];
core::vector3df offset = core::vector3df(0,0,0);

core::stringw date;
f32 burnS;

core::vector3df camPos = core::vector3df(0,0,0);
core::vector3df velocity = core::vector3df(0,0,0);
core::vector3df orbVelocity = core::vector3df(0,0,0);
core::vector3df pull = core::vector3df(0,0,0);
core::vector3df thrusters = core::vector3df(0,0,0);

bool interplanetary = false;
f64 lDTOPeriod = 0;
f64 deltaV0[3];
f64 deltaV1[3];
f64 totalV0[3];
f64 totalV1[3];

f64 startDay = 0;
f64 arrivalDay = 0;

core::vector3df startPos = core::vector3df(0,0,0);

class MyShaderCallBack : public video::IShaderConstantSetCallBack
{
public:

	virtual void OnSetConstants(video::IMaterialRendererServices* services, s32 userData)
	{
		video::IVideoDriver* driver = services->getVideoDriver();

		// set inverted world matrix
		// if we are using highlevel shaders (the user can select this when
		// starting the program), we must set the constants by name.

		core::matrix4 invWorld = driver->getTransform(video::ETS_WORLD);
		invWorld.makeInverse();

		services->setVertexShaderConstant("mInvWorld", invWorld.pointer(), 16);

		// set clip matrix

		core::matrix4 worldViewProj;
		worldViewProj = driver->getTransform(video::ETS_PROJECTION);
		worldViewProj *= driver->getTransform(video::ETS_VIEW);
		worldViewProj *= driver->getTransform(video::ETS_WORLD);

		services->setVertexShaderConstant("mWorldViewProj", worldViewProj.pointer(), 16);

		core::vector3df posLight2 = light->getAbsolutePosition();
		services->setVertexShaderConstant("mLight2Pos", reinterpret_cast<f32*>(&posLight2), 3);

		// set camera position

		core::vector3df pos = device->getSceneManager()->getActiveCamera()->getAbsolutePosition();
		services->setVertexShaderConstant("mLightPos", reinterpret_cast<f32*>(&pos), 3);


		// set light color

		video::SColorf col(0.0f,1.0f,1.0f,0.0f);
		services->setVertexShaderConstant("mLightColor", reinterpret_cast<f32*>(&col), 4);
		video::SColorf fvAmbient(0.368f,0.368f,0.368f,1.0f);
		services->setVertexShaderConstant("fvAmbient", reinterpret_cast<f32*>(&fvAmbient), 4);
		video::SColorf fvSpecular(0.790f,0.488f,0.488f,1.0f);
		services->setVertexShaderConstant("fvSpecular", reinterpret_cast<f32*>(&fvSpecular), 4);
		video::SColorf fvDiffuse(0.886f,0.885f,0.885f,1.0f);
		services->setVertexShaderConstant("fvDiffuse", reinterpret_cast<f32*>(&fvDiffuse), 4);
		f32 fSpecularPower = 50.0f;
		services->setVertexShaderConstant("fSpecularPower", reinterpret_cast<f32*>(&fSpecularPower), 1);

		// set transposed world matrix

		core::matrix4 world = driver->getTransform(video::ETS_WORLD);
		world = world.getTransposed();

		services->setVertexShaderConstant("mTransWorld", world.pointer(), 16);

		//irr::core::vector3df camPos = camera->getAbsolutePosition();
		//services->setVertexShaderConstant("camPos", reinterpret_cast<f32*>(&camPos), 3);
	}
};
class MyeventReceiver:public IEventReceiver
{
public:

	virtual bool OnEvent(const SEvent& event)
	{
		if (event.EventType == EET_GUI_EVENT)
		{
			if(event.GUIEvent.EventType == EGET_SCROLL_BAR_CHANGED)
			{
				s32 id = event.GUIEvent.Caller->getID();
				if (id == 104)
				{
					timestepperPos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
					//printf("%d\n",timestepperPos);
				}
			}
			if(event.GUIEvent.EventType == EGET_BUTTON_CLICKED)
			{
				s32 id = event.GUIEvent.Caller->getID();
				if (id == 107)
				{
					editB = true;
					printf("editB\n");
				}
				if (id == 108)
				{
					burnB = true;
					printf("burnB\n");
				}
			}

		}
		else if(event.EventType == EET_KEY_INPUT_EVENT)
		{
			if(event.KeyInput.PressedDown == true)
			{
				if(event.KeyInput.Key==KEY_KEY_A)
				{
					leftBool=true;
					//printf("A pressed\n");
				}
				if(event.KeyInput.Key==KEY_KEY_D)
					rightBool=true;
				if(event.KeyInput.Key==KEY_KEY_W)
					up=true;
				if(event.KeyInput.Key==KEY_KEY_S)
					down=true;
				if(event.KeyInput.Key==KEY_KEY_Q)
					keyQ=true;
				if(event.KeyInput.Key==KEY_KEY_E)
					keyE=true;
				if(event.KeyInput.Key==KEY_UP)
					upA=true;
				if(event.KeyInput.Key==KEY_DOWN)
					downA=true;
				if(event.KeyInput.Key==KEY_RIGHT)
					rightA=true;
				if(event.KeyInput.Key==KEY_LEFT)
					leftA=true;
				if(event.KeyInput.Key==KEY_KEY_1)
					key1=true;
				if(event.KeyInput.Key==KEY_KEY_2)
					key2=true;
				if(event.KeyInput.Key==KEY_KEY_3)
					key3=true;
				if(event.KeyInput.Key==KEY_KEY_4)
					key4=true;
				if(event.KeyInput.Key==KEY_KEY_5)
					key5=true;
				if(event.KeyInput.Key==KEY_KEY_6)
					key6=true;
				if(event.KeyInput.Key==KEY_KEY_7)
					key7=true;
				if(event.KeyInput.Key==KEY_KEY_8)
					key8=true;
				if(event.KeyInput.Key==KEY_KEY_9)
					key9=true;
				if(event.KeyInput.Key==KEY_KEY_0)
					key0=true;
				if(event.KeyInput.Key==KEY_F1)
					keyF1=true;
				if(event.KeyInput.Key==KEY_F2)
					keyF2=true;
				if(event.KeyInput.Key==KEY_F3)
					keyF3=true;
				if(event.KeyInput.Key==KEY_KEY_M)
					keyM=true;
				if(event.KeyInput.Key==KEY_KEY_P)
				{
					if(planningM)
					{
						planningM = false;
						//hide planning GUI
						planningGUI->setVisible(false);
						if(flightpathLine)
						{
							flightpathLine->remove();
						}
					}
					else
					{
						planningM = true;
						projected = false;
						//show planning GUI
						planningGUI->setVisible(true);
					}
				}
				if(event.KeyInput.Key==KEY_KEY_T)
				{
					if(trailAbsolute)
					{
						trailAbsolute = false;
					}
					else
					{
						trailAbsolute = true;
					}
				}
				if(event.KeyInput.Key==KEY_KEY_O)
				{
					if(planesVisible)
					{
						planesVisible = false;
					}
					else
					{
						planesVisible = true;
					}
				}
				if(event.KeyInput.Key==KEY_KEY_F)
				{
					if(displayFieldLines)
					{
						displayFieldLines = false;
					}
					else
					{
						displayFieldLines = true;
					}
				}
				if(event.KeyInput.Key==KEY_KEY_I)
				{
					if(showSOI)
					{
						showSOI = false;
					}
					else
					{
						showSOI = true;
					}
				}
				if(event.KeyInput.Key==KEY_COMMA)
				{
					f32 increase = 0.1;

					if(timescale >= 20.0)
					{
						increase = 16;
					}
					else
					{
						increase = 0.1;
					}
					if(timefactor > 1)
					{
						timefactor -= 1;
					}
					updateVar = true;
					timescale -= increase;
					fpsincrement = incrementF * timescale;
					printf("interval = %f\n",fpsincrement*86400);


				}
				if(event.KeyInput.Key==KEY_PERIOD)
				{
					f32 increase = 0.1;
					
					if(timescale >= 20.0)
					{
						increase = 16;
					}
					else
					{
						increase = 0.1;
					}
					
					if(timescale >= 4800.0)
					{
						timefactor += 1;
					}
					updateVar = true;
					timescale += increase;
					fpsincrement = incrementF * timescale;
					printf("interval = %f\n",fpsincrement*86400);

				}
				if(event.KeyInput.Key==KEY_F8)
				{
					//OrbitPlanes[0][1]->setRotation(OrbitPlanes[0][1]->getRotation()+core::vector3df(0,0.5,0));
					//printf("%f\n",OrbitPlanes[0][1]->getRotation().Y);
				}
				if(event.KeyInput.Key==KEY_F9)
				{
					//OrbitPlanes[0][1]->setRotation(OrbitPlanes[0][1]->getRotation()-core::vector3df(0,0.5,0));
					//printf("%f\n",OrbitPlanes[0][1]->getRotation().Y);
				}
				if(win)
				{
					if(event.KeyInput.Key==KEY_SHIFT || event.KeyInput.Key==KEY_LSHIFT || event.KeyInput.Key==KEY_RSHIFT)
					{
						key_shift=true;
						//printf("win shift pressed\n");
					}
					if(event.KeyInput.Key==KEY_CONTROL)
					{
						key_ctrl=true;
						//printf("win control pressed\n");
					}
				}
				else
				{
					if(event.KeyInput.Key==KEY_LSHIFT)
					{
						key_shift=true;
						//printf("shift pressed\n");
					}
					if(event.KeyInput.Key==KEY_LCONTROL)
					{
						key_ctrl=true;
						//printf("control pressed\n");
					}
				}
			}

			if(event.KeyInput.PressedDown == false) {
				if(event.KeyInput.Key==KEY_KEY_A)
					leftBool=false;
				if(event.KeyInput.Key==KEY_KEY_D)
					rightBool=false;
				if(event.KeyInput.Key==KEY_KEY_W)
					up=false;
				if(event.KeyInput.Key==KEY_KEY_S)
					down=false;
				if(event.KeyInput.Key==KEY_KEY_Q)
					keyQ=false;
				if(event.KeyInput.Key==KEY_KEY_E)
					keyE=false;
				if(event.KeyInput.Key==KEY_UP)
					upA=false;
				if(event.KeyInput.Key==KEY_DOWN)
					downA=false;
				if(event.KeyInput.Key==KEY_RIGHT)
					rightA=false;
				if(event.KeyInput.Key==KEY_LEFT)
					leftA=false;
				if(event.KeyInput.Key==KEY_KEY_1)
					key1=false;
				if(event.KeyInput.Key==KEY_KEY_2)
					key2=false;
				if(event.KeyInput.Key==KEY_KEY_3)
					key3=false;
				if(event.KeyInput.Key==KEY_KEY_4)
					key4=false;
				if(event.KeyInput.Key==KEY_KEY_5)
					key5=false;
				if(event.KeyInput.Key==KEY_KEY_6)
					key6=false;
				if(event.KeyInput.Key==KEY_KEY_7)
					key7=false;
				if(event.KeyInput.Key==KEY_KEY_8)
					key8=false;
				if(event.KeyInput.Key==KEY_KEY_9)
					key9=false;
				if(event.KeyInput.Key==KEY_KEY_0)
					key0=false;
				if(event.KeyInput.Key==KEY_F1)
					keyF1=false;
				if(event.KeyInput.Key==KEY_F2)
					keyF2=false;
				if(event.KeyInput.Key==KEY_F3)
					keyF3=false;
				if(event.KeyInput.Key==KEY_KEY_M)
					keyM=false;
				if(win)
				{
					if(event.KeyInput.Key==KEY_SHIFT || event.KeyInput.Key==KEY_LSHIFT || event.KeyInput.Key==KEY_RSHIFT)
					{
						key_shift=false;
						//printf("win shift released\n");
					}
					if(event.KeyInput.Key==KEY_CONTROL)
					{
						key_ctrl=false;
						//printf("win control released\n");
					}
				}
				else
				{
					if(event.KeyInput.Key==KEY_LSHIFT)
					{
						key_shift=false;
						//printf("shift released\n");
					}
					if(event.KeyInput.Key==KEY_LCONTROL)
					{
						key_ctrl=false;
						//printf("control released\n");
					}
				}
			}
		}if (event.EventType == EET_MOUSE_INPUT_EVENT)
		{
			// leftBool mouse button state check
			if(event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
			{
				leftB = true;
			}
			if(event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
			{
				leftB = false;
			}
			if(event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
			{
				rightB = true;
			}
			if(event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP)
			{
				rightB = false;
			}
			if(event.MouseInput.Event == EMIE_MOUSE_WHEEL)
			{
				lastWheelMovement = event.MouseInput.Wheel;
				mouseW = true;
			}
			if (event.MouseInput.Event == EMIE_MOUSE_MOVED)
			{
				CursorPos = cursor->getRelativePosition();
			}
		}
		return false;
	}

}my_event_receiver;
//This function is to copy an array into a vector
void array2vector(double source[], vector<double>& dest, int N){
    dest.clear();
    for (int i=0; i<N; ++i){
        dest.push_back(source[i]);
    }
 }

//===============================================================================================================================
//INIT
//===============================================================================================================================

SIrrlichtCreationParameters driverQuery(SIrrlichtCreationParameters param)
{
	char i;
	/*
	printf("Please select the driver you want:\n"\
	" (a) Direct3D 9.0c\n (b) OpenGL 1.5\n");
	std::cin >> i;
	switch(i)
	{
	case 'a': param.DriverType = video::EDT_DIRECT3D9;break;
	case 'b': param.DriverType =video::EDT_OPENGL;;break;

	}
	printf("Anti-Aliasing ?\n (a) yes\n (b) no\n\n");
	std::cin >> i;
	switch(i)
	{
	case 'a': param.AntiAlias = true;break;
	case 'b': param.AntiAlias = false;break;
	}
	printf("High-Precision FPU ?\n (a) yes\n (b) no\n\n");
	std::cin >> i;
	switch(i)
	{
	case 'a': param.HighPrecisionFPU = true;break;
	case 'b': param.HighPrecisionFPU = false;break;
	}
	printf("VSync ?\n (a) yes\n (b) no\n\n");
	std::cin >> i;
	switch(i)
	{
	case 'a': param.Vsync = true;break;
	case 'b': param.Vsync = false;break;
	}
	*/
	printf("Please select the resolution:\n"\
		" (a) 1920x1200\n (b) 1600x1200\n (c) 1280x1024\n"\
		" (d) 1024x768\n (e) 800x600\n"\
		" (f) 640x480\n (otherKey) exit\n\n");
	std::cin >> i;
	switch(i)
	{
	case 'f': param.WindowSize = core::dimension2di(640,480);break;
	case 'e': param.WindowSize = core::dimension2di(800,600);break;
	case 'd': param.WindowSize = core::dimension2di(1024,768);break;
	case 'c': param.WindowSize = core::dimension2di(1280,1024);break;
	case 'b': param.WindowSize = core::dimension2di(1600,1200);break;
	case 'a': param.WindowSize = core::dimension2di(1920,1200);break;
	}
	printf("Fullscreen ? In Windows alt-F4 quits !!!\n (a) yes\n (b) no\n\n");
	std::cin >> i;
	switch(i)
	{
	case 'a': param.Fullscreen = true;break;
	case 'b': param.Fullscreen = false;break;
	}
	return param;
}
void makeGUI()
{
	//create text elements-----------------------------------------------------------------------------
	fnt = env->getFont("../data/orator.xml");
	fnt->setKerningHeight(9);


	IGUISkin* skin = env->getSkin();
	skin->setFont(fnt);
	skin->setColor(EGDC_BUTTON_TEXT,video::SColor(255,21,188,255));
	skin->setColor(EGDC_SCROLLBAR,video::SColor(120,21,188,255));
	skin->setColor(EGDC_3D_FACE,video::SColor(0,120,120,120));
	skin->setColor(EGDC_3D_LIGHT,video::SColor(120,21,188,255));
	skin->setColor(EGDC_GRAY_TEXT,video::SColor(255,255,0,0));
	skin->setColor(EGDC_HIGH_LIGHT_TEXT,video::SColor(255,255,0,0));

	info = env->addStaticText(L"",core::rect<s32>(Width*0.75,Height*0.25,Width,Height), true, true, 0, -1, true);
	info->setDrawBackground(true);
	info->setDrawBorder(false);
	//info->setTextAlignment(EGUIA_CENTER,EGUIA_SCALE);

	info2 = env->addStaticText(
		L"",core::rect<s32>(Width*0.5,Height*0,Width,Height*0.05), true, true, 0, -1, true);
	info2->setDrawBackground(false);
	info2->setDrawBorder(false);
	info2->setTextAlignment(EGUIA_CENTER,EGUIA_CENTER);

	//create flightmarker aid--------------------------------------------------------------------------
	video::IImage *flightmarkerAidIm = driver->createImageFromFile("../data/flightmarker2.png");
	video::IImage *flightmarkerAidIm_scaled = driver->createImage(video::ECF_A8R8G8B8,core::dimension2d<u32>(Width,Height));
	flightmarkerAidIm->copyToScaling(flightmarkerAidIm_scaled);
	video::ITexture *flightmarkerAid =  driver->addTexture("scaled",flightmarkerAidIm_scaled);
	flightmarkerAidIm->drop();
	flightmarkerAidIm_scaled->drop();

	flightaid = env->addImage(flightmarkerAid,core::vector2di(0,0));
	flightaid->setUseAlphaChannel(true);
	flightaid->setColor(video::SColor(50,255,255,255));

	//create Planning GUI
	planningGUI = env->addStaticText(L"",core::rect<s32>(0,0,Width,Height),false,false);
	planningGUI->setDrawBackground(false);
	planningGUI->setDrawBorder(false);

	//create timestepper
	timestepper = env->addScrollBar(true,core::rect<s32>(Width*0.1,Height*0.04,Width*0.9,Height*0.06),planningGUI,104);
	timestepper->setMax(4999);
	timestepper->setMin(0);
	timestepper->setPos(4999);
	timestepper->setSmallStep(1);
	//timestepper->setText();

	//create edit Button
	editbutton = env->addButton(core::rect<s32>(Width*0.1,Height*0.08,Width*0.15,Height*0.1),planningGUI,107,L"Edit");

	//create burn Button
	burnbutton = env->addButton(core::rect<s32>(Width*0.1,Height*0.12,Width*0.15,Height*0.14),planningGUI,108,L"Burn");
	
	planningGUI->setVisible(false);

	/*
	gui::IGUIStaticText* burnForT = env->addStaticText(
	L"for",core::rect<s32>(Width*0.17,Height*0.125,Width*0.19,Height*0.145), false, false, planningGUI, -1,false);

	//create burn for text input
	burnFor = env->addEditBox(L"",core::rect<s32>(Width*0.2,Height*0.12,Width*0.25,Height*0.14),true,planningGUI,109);
	burnFor->setTextAlignment(gui::EGUI_ALIGNMENT::EGUIA_CENTER,gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT);

	gui::IGUIStaticText* burnForT2 = env->addStaticText(
	L"seconds, with",core::rect<s32>(Width*0.26,Height*0.125,Width*0.35,Height*0.145), false, false, planningGUI, -1,false);

	//create burn with text input
	burnWith = env->addEditBox(L"",core::rect<s32>(Width*0.37,Height*0.12,Width*0.42,Height*0.14),true,planningGUI,110);
	burnWith->setTextAlignment(gui::EGUI_ALIGNMENT::EGUIA_CENTER,gui::EGUI_ALIGNMENT::EGUIA_LOWERRIGHT);

	gui::IGUIStaticText* newton = env->addStaticText(
	L"Kilo Newton",core::rect<s32>(Width*0.44,Height*0.125,Width*0.56,Height*0.145), false, false, planningGUI, -1,false);

	burnbutton->setEnabled(false);
	burnFor->setEnabled(false);
	burnWith->setEnabled(false);
	*/
	gui::IGUIStaticText* burnForT = env->addStaticText(
		L"until",core::rect<s32>(Width*0.17,Height*0.125,Width*0.21,Height*0.145), false, false, planningGUI, -1,false);

	//create burn with text input
	burnUntil = env->addEditBox(L"",core::rect<s32>(Width*0.23,Height*0.12,Width*0.28,Height*0.14),true,planningGUI,110);
	burnUntil->setTextAlignment(EGUIA_CENTER,EGUIA_LOWERRIGHT);

	gui::IGUIStaticText* burnForT2 = env->addStaticText(
		L"KM/s",core::rect<s32>(Width*0.3,Height*0.125,Width*0.4,Height*0.145), false, false, planningGUI, -1,false);

	burnbutton->setEnabled(false);
	burnUntil->setEnabled(false);

	//create Interplanetary Listbox Departure
	gui::IGUIListBox* departureBox = env->addListBox(core::rect<s32>(Width*0.10,Height*0.8,Width*0.2,Height*0.96),planningGUI,105);
	departureBox->addItem(L"Mercury");
	departureBox->addItem(L"Venus");
	departureBox->addItem(L"Earth");
	departureBox->addItem(L"Mars");
	departureBox->addItem(L"Jupiter");
	departureBox->addItem(L"Saturn");
	departureBox->addItem(L"Uranus");
	departureBox->addItem(L"Neptune");
	departureBox->addItem(L"Pluto");

	//create Interplanetary Listbox Arrival
	gui::IGUIListBox* arrivalBox   = env->addListBox(core::rect<s32>(Width*0.22,Height*0.8,Width*0.32,Height*0.96),planningGUI,106);
	arrivalBox->addItem(L"Mercury");
	arrivalBox->addItem(L"Venus");
	arrivalBox->addItem(L"Earth");
	arrivalBox->addItem(L"Mars");
	arrivalBox->addItem(L"Jupiter");
	arrivalBox->addItem(L"Saturn");
	arrivalBox->addItem(L"Uranus");
	arrivalBox->addItem(L"Neptune");
	arrivalBox->addItem(L"Pluto");

	//create flightmarker------------------------------------------------------------------------------
	flightmarker = smgr->addBillboardSceneNode(0,core::dimension2df(0.07,0.07));
	flightmarker->setMaterialFlag(video::EMF_LIGHTING, false);
	flightmarker->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	video::ITexture *tex21 =  driver->getTexture("../data/flightmarker.jpg");
	flightmarker->setMaterialTexture(0,tex21);


}
void updateVars()
{
	if(mFOrb.size() > 0)
	{
		pRot.erase(0,pRot.size());
		mFOrb.erase(0,mFOrb.size());
	}
	//Planets sideral rotation period

	pRot.push_back(  -7.1047755459309529493344246268572e-5 * timescale);	     //M
	pRot.push_back(   1.7145471092392828803842780144996e-5 * timescale);		 //V
	pRot.push_back(	 -0.0041780741460691622216637195534378 * timescale);		 //E
	pRot.push_back(	 -0.0040612488307664616223356989295523 * timescale);		 //M
	pRot.push_back(	 -0.01007556675062972292191435768262 * timescale);			 //J
	pRot.push_back(	 -0.0094607379375591296121097445600757 * timescale);		 //S
	pRot.push_back(	  0.0058004909535543088366999382827763 * timescale);		 //U
	pRot.push_back(	 -0.006206862306966582253339291921148 * timescale);			 //N
	pRot.push_back(   6.5234329539826601933336777705933e-4 * timescale);		 //P

	//Earth
	//moon 0;
	mFOrb.push_back((360.0/(-27.321582*dayV*minSecV*minSecV))*timescale);

	//Jupiter
	//Io
	mFOrb.push_back((360.0/(-1.769137786*dayV*minSecV*minSecV))*timescale);
	//Europa
	mFOrb.push_back((360.0/(-3.551181*dayV*minSecV*minSecV))*timescale);
	//Ganymede
	mFOrb.push_back((360.0/(-7.15455296*dayV*minSecV*minSecV))*timescale);
	//Callisto
	mFOrb.push_back((360.0/(-16.6890184*dayV*minSecV*minSecV))*timescale);

	//Saturn
	//Mimas Enceladus Tethys Dione Rhea Titan Iapetus
	//Mimas
	mFOrb.push_back((360.0/(-0.9424218*dayV*minSecV*minSecV))*timescale);
	//Enceladus
	mFOrb.push_back((360.0/(-1.370218*dayV*minSecV*minSecV))*timescale);
	//Tethys
	mFOrb.push_back((360.0/(-1.887802*dayV*minSecV*minSecV))*timescale);
	//Dione
	mFOrb.push_back((360.0/(-2.736915*dayV*minSecV*minSecV))*timescale);
	//Rhea
	mFOrb.push_back((360.0/(-4.518212*dayV*minSecV*minSecV))*timescale);
	//Titan
	mFOrb.push_back((360.0/(-15.945*dayV*minSecV*minSecV))*timescale);
	//Iapetus
	mFOrb.push_back((360.0/(-79.321*dayV*minSecV*minSecV))*timescale);

	//Uranus
	//Puck Miranda Ariel Umbriel Titania Oberon
	//Puck
	mFOrb.push_back((360.0/(-0.761833*dayV*minSecV*minSecV))*timescale);
	//Miranda incl. 4.232
	mFOrb.push_back((360.0/(-1.413479*dayV*minSecV*minSecV))*timescale);
	//Ariel
	mFOrb.push_back((360.0/(-2.520379*dayV*minSecV*minSecV))*timescale);
	//Umbriel
	mFOrb.push_back((360.0/(-4.144177*dayV*minSecV*minSecV))*timescale);
	//Titania
	mFOrb.push_back((360.0/(-8.705872*dayV*minSecV*minSecV))*timescale);
	//Oberon
	mFOrb.push_back((360.0/(-13.463239*dayV*minSecV*minSecV))*timescale);
}
void initVars()
{
	randomFacts.push_back(core::stringw("Mercury has a radius of around 2440 km"));
	randomFacts.push_back(core::stringw("Venus has a radius of around 6052 km"));
	randomFacts.push_back(core::stringw("Earth has a radius of around 6378 km"));
	randomFacts.push_back(core::stringw("Mars has a radius of around 3397 km"));
	randomFacts.push_back(core::stringw("Jupiter has a radius of around 71492 km"));
	randomFacts.push_back(core::stringw("Saturn has a radius of around 60268 km"));
	randomFacts.push_back(core::stringw("Uranus has a radius of around 25559 km"));
	randomFacts.push_back(core::stringw("Neptune has a radius of around 2440 km"));
	randomFacts.push_back(core::stringw("Pluto has a radius of around 1150 km"));
	randomFacts.push_back(core::stringw("The sun has a radius of around 695000 km"));

	randomFacts.push_back(core::stringw("Mercury has a mass of around 33000e21 kg"));
	randomFacts.push_back(core::stringw("Venus has a mass of around 4869000e21 kg"));
	randomFacts.push_back(core::stringw("Earth has a mass of around 5972000e21 kg"));
	randomFacts.push_back(core::stringw("Mars has a mass of around 6421900e21 kg"));
	randomFacts.push_back(core::stringw("Jupiter has a mass of around 1900000000e21 kg"));
	randomFacts.push_back(core::stringw("Saturn has a mass of around 56800000e21 kg"));
	randomFacts.push_back(core::stringw("Uranus has a mass of around 86830000e21 kg"));
	randomFacts.push_back(core::stringw("Neptune has a mass of around 1024700000e21 kg"));
	randomFacts.push_back(core::stringw("Pluto has a mass of around 1270e21 kg"));
	randomFacts.push_back(core::stringw("The sun has a mass of around 1989000000000e21 kg"));

	randomFacts.push_back(core::stringw("One Mercury year is 87.97 earth days long"));
	randomFacts.push_back(core::stringw("One Venus year is 224.70 earth days long "));
	randomFacts.push_back(core::stringw("One Earth year is 365.261 earth days long "));
	randomFacts.push_back(core::stringw("One Mars year is 686.98 earth days long "));
	randomFacts.push_back(core::stringw("One Jupiter year is 4332.71 earth days long "));
	randomFacts.push_back(core::stringw("One Saturn year is 10759.50 earth days long "));
	randomFacts.push_back(core::stringw("One Uranus year is 30685 earth days long "));
	randomFacts.push_back(core::stringw("One Neptune year is 60190 earth days long "));
	randomFacts.push_back(core::stringw("One Pluto year is 90550 earth days long "));


	intro.push_back(core::stringw("Move your mouse on earth..."));
	intro.push_back(core::stringw("with your mouse you control the rotation of your satellite"));
	intro.push_back(core::stringw("KEYBOARD:"));
	intro.push_back(core::stringw("Ctrl-Q quits the application"));
	intro.push_back(core::stringw("W-A-S-D-Q-E controls the satellite"));
	intro.push_back(core::stringw("press Q to tilt the satellite to the left"));
	intro.push_back(core::stringw("press E to tilt the satellite to the right"));
	intro.push_back(core::stringw("press W to thrust the satellite upwards from view direction"));
	intro.push_back(core::stringw("press S to thrust the satellite downwards from view direction"));
	intro.push_back(core::stringw("press A to thrust the satellite to the left from view direction"));
	intro.push_back(core::stringw("press D to thrust the satellite to the right from view direction"));
	intro.push_back(core::stringw("Shift-Left Mouse Button thrusts forward tangential to orbit"));
	intro.push_back(core::stringw("increasing orbital velocity and thus increasing the altitude"));
	intro.push_back(core::stringw("Shift-Right Mouse Button thrusts backwards tangential to orbit"));
	intro.push_back(core::stringw("decreasing orbital velocity and thus decreasing the altitude"));
	intro.push_back(core::stringw("Left Mouse Button thrusts forward in view direction"));
	intro.push_back(core::stringw("Right Mouse Button thrusts backwards in view direction"));

	intro.push_back(core::stringw("press F2 to change to the mini-map,"));
	intro.push_back(core::stringw("F1 to change back to satellite view"));
	intro.push_back(core::stringw("The arrow (cursor) keys rotate the mini-map"));
	intro.push_back(core::stringw("Shift cursor Left/Right tilts the mini-map"));
	intro.push_back(core::stringw("Shift cursor Up/Down zooms the mini-map"));

	intro.push_back(core::stringw("The blue circle is your velocity vector or flightmarker."));

	intro.push_back(core::stringw("With the keys 1-9 you can jump to different Planets,"));
	intro.push_back(core::stringw("1 being Mercury and 9 Pluto."));

	intro.push_back(core::stringw("Our Solarsystem is a vast place..."));
	intro.push_back(core::stringw("This model has proportional planets and orbits."));
	intro.push_back(core::stringw("The planets position is calculated by painfully hard maths"));
	intro.push_back(core::stringw("and is supposedly accurate for many centuries ahead."));
	intro.push_back(core::stringw("The satellites gravitation simulation is based on the"));
	intro.push_back(core::stringw("planets mass, the gravitational constant and the distance"));
	intro.push_back(core::stringw("to the body so it subscribes to Keplers and Newtons laws."));

	intro.push_back(core::stringw("The magnetosphere around earth is modeled according to"));
	intro.push_back(core::stringw("fortran subroutines created by Dr. Nikolai Tsyganenko from the"));
	intro.push_back(core::stringw("Department of Earth's Physics, Institute of Physics, University of"));
	intro.push_back(core::stringw("St.-Petersburg. It's based on magnetometer measurements"));
	intro.push_back(core::stringw("acquired by several satellites and is affected by"));
	intro.push_back(core::stringw("simulated solar wind properties. It can be updated by pressing"));
	intro.push_back(core::stringw("the 0 (zero) key. Note that this model is intended for earth only"));

	intro.push_back(core::stringw("Right now we are in geosynchronous orbit above earth."));
	intro.push_back(core::stringw("The easiest way to get into an orbit of a planet or a moon"));
	intro.push_back(core::stringw("is adjusting your distance and speed by checking the marker"));
	intro.push_back(core::stringw("little is more... try adjusting instead of controlling."));
	intro.push_back(core::stringw("Try doing nothing for some time, and ellipses will be round."));
	intro.push_back(core::stringw("enjoy..."));

	planetNames.push_back(core::stringw(L"Mercury"));
	planetNames.push_back(core::stringw(L"Venus"));
	planetNames.push_back(core::stringw(L"Earth"));
	planetNames.push_back(core::stringw(L"Mars"));
	planetNames.push_back(core::stringw(L"Jupiter"));
	planetNames.push_back(core::stringw(L"Saturn"));
	planetNames.push_back(core::stringw(L"Uranus"));
	planetNames.push_back(core::stringw(L"Neptune"));
	planetNames.push_back(core::stringw(L"Pluto"));
	planetNames.push_back(core::stringw(L"Sun"));

	pMeanDist.push_back(57909175.00);
	pMeanDist.push_back(108208930.00);
	pMeanDist.push_back(149598790.00);
	pMeanDist.push_back(227936640.00);
	pMeanDist.push_back(778412010.00);
	pMeanDist.push_back(1426725400.00);
	pMeanDist.push_back(2870972200.00);
	pMeanDist.push_back(4498252900.00);
	pMeanDist.push_back(5913520000.00);

	pMeanDist2.push_back(0.387*AU);
	pMeanDist2.push_back(0.723*AU);
	pMeanDist2.push_back(1.000*AU);
	pMeanDist2.push_back(1.524*AU);
	pMeanDist2.push_back(5.203*AU);
	pMeanDist2.push_back(9.529*AU);
	pMeanDist2.push_back(19.19*AU);
	pMeanDist2.push_back(30.06*AU);
	pMeanDist2.push_back(39.53*AU);

	/*
	SOI radius
	Planet			AU				10e4 km.	as a ratio to the planetary radius

	Mercury			7.518 ´ 10–4	11.24e4			45.18
	Venus			4.097 ´ 10–3	61.29e4			99.06
	Earth			6.180 ´ 10–3	92.95e4			144.96
	Mars			3.859 ´ 10–3	57.73e4			170.80
	Jupiter			0.322			4821.00e4		675.50
	Saturn			0.365			5465.00e4		905.87
	Uranus			0.347			5187.00e4		2042.26
	Neptune			0.581			8694.00e4		3895.25
	Pluto			0.081			3149.00e4		2736.24
	*/
	pSOI.push_back(112400.0);
	pSOI.push_back(612900.0);
	pSOI.push_back(929500.0);
	pSOI.push_back(577300.0); //577300.0
	pSOI.push_back(48210000.0);
	pSOI.push_back(54650000.0);
	pSOI.push_back(51870000.0);
	pSOI.push_back(86940000.0);
	pSOI.push_back(31490000.0);
	/*
	<Mercury mass="330e21" />
	<Venus   mass="4869e21" />
	<Earth   mass="5972e21" />
	<Mars    mass="642.19e21" />
	<Jupiter mass="1900000e21" />
	<Saturn  mass="568000e21" />
	<Uranus  mass="86830e21" />
	<Neptune mass="102470e21" />
	<Pluto   mass="12.7e21" />
	<Sun     mass="1989000000e21" />
	*/

	/*
	rocketNames.push_back(core::stringw("SATURN V"));
	rocketNames.push_back(core::stringw("N-1"));
	rocketNames.push_back(core::stringw("ARES-1"));
	rocketNames.push_back(core::stringw("DELTA IV"));
	rocketNames.push_back(core::stringw("ATLAS V"));
	rocketNames.push_back(core::stringw("ARIANE 5"));
	rocketNames.push_back(core::stringw("SPACE SHUTTLE"));
	rocketNames.push_back(core::stringw("H-IIB ROCKET"));
	rocketNames.push_back(core::stringw("FALCON 9"));
	rocketNames.push_back(core::stringw("PROTON"));
	*/

	//Orbit Time in earth days

	orbitT.push_back(87.97);
	orbitT.push_back(224.70);
	orbitT.push_back(365.261);
	orbitT.push_back(686.98);
	orbitT.push_back(4332.71);
	orbitT.push_back(10759.50);
	orbitT.push_back(30685.00);
	orbitT.push_back(60190.00);
	orbitT.push_back(90550.00);

	OrbitCol.push_back(video::SColor(1,128,122,120));
	OrbitCol.push_back(video::SColor(1,204,131,47));
	OrbitCol.push_back(video::SColor(1,41,123,163));
	OrbitCol.push_back(video::SColor(1,184,89,53));
	OrbitCol.push_back(video::SColor(1,212,163,119));
	OrbitCol.push_back(video::SColor(1,236,190,128));
	OrbitCol.push_back(video::SColor(1,155,184,195));
	OrbitCol.push_back(video::SColor(1,95,129,222));
	OrbitCol.push_back(video::SColor(1,127,136,150));
	OrbitCol.push_back(video::SColor(1,255,255,255));

	//Planets radius in km
	planetSize.push_back(2440.0);
	planetSize.push_back(6051.8);
	planetSize.push_back(6371.0);
	planetSize.push_back(3392.0);
	planetSize.push_back(71492.0);
	planetSize.push_back(60268.0);
	planetSize.push_back(25559.0);
	planetSize.push_back(24764.0);
	planetSize.push_back(1159.0);
	planetSize.push_back(695000.0);

	//Planets sideral rotation period
	/*
	pRot.push_back( (360.0/(58.646*dayV*minSecV*minSecV))*timescale);			// 5067014,4		 7,1047755459309529493344246268572e-5
	pRot.push_back( (360.0/(-243.0185 * dayV*minSecV*minSecV))*timescale);		//-20996798,4		-1,7145471092392828803842780144996e-5
	pRot.push_back(	(360.0/(0.99726968*dayV*minSecV*minSecV))*timescale);		// 86164,100352		 0,0041780741460691622216637195534378
	pRot.push_back(	(360.0/(1.025957*dayV*minSecV*minSecV))*timescale);			// 88642,6848		 0,0040612488307664616223356989295523
	pRot.push_back(	(360.0/(9.925 * minSecV*minSecV))*timescale);				// 35730			 0,01007556675062972292191435768262
	pRot.push_back(	(360.0/(10.57 * minSecV*minSecV))*timescale);				// 38052			 0,0094607379375591296121097445600757
	pRot.push_back(	(360.0/(−0.71833 * dayV*minSecV*minSecV))*timescale);		//-62063,712		-0,0058004909535543088366999382827763
	pRot.push_back(	(360.0/(0.6713*dayV*minSecV*minSecV))*timescale);			// 58000,32			 0,006206862306966582253339291921148
	pRot.push_back(	(360.0/0(−6,387230 *dayV*minSecV*minSecV))*timescale);		//-551856,672,		-6,5234329539826601933336777705933e-4
	*/
	updateVars();

	//Earth
	//moon 0;
	mSize.push_back(1737);
	mDist.push_back(384400/scaleF);
	mMass.push_back(7.3477e22);
	//Jupiter
	//Io
	mSize.push_back(1821.3f);
	mDist.push_back(421700/scaleF);
	mMass.push_back(8.9319e22);
	//Europa
	mSize.push_back(1569);
	mDist.push_back(670900/scaleF);
	mMass.push_back(4.8e22);
	//Ganymede
	mSize.push_back(2634.1);
	mDist.push_back(1070400/scaleF);
	mMass.push_back(1.4819e23);
	//Callisto
	mSize.push_back(2410.3);
	mDist.push_back(1882700/scaleF);
	mMass.push_back(1.075938e23);

	//Saturn
	//Mimas Enceladus Tethys Dione Rhea Titan Iapetus
	//Mimas
	mSize.push_back(400);
	mDist.push_back(185000/scaleF);
	mMass.push_back(0.4e20);
	//Enceladus
	mSize.push_back(500);
	mDist.push_back(238000/scaleF);
	mMass.push_back(1.1e20);
	//Tethys
	mSize.push_back(1060);
	mDist.push_back(295000/scaleF);
	mMass.push_back(6.2e20);
	//Dione
	mSize.push_back(1120);
	mDist.push_back(377000/scaleF);
	mMass.push_back(11e20);
	//Rhea
	mSize.push_back(1530);
	mDist.push_back(527000/scaleF);
	mMass.push_back(23e20);
	//Titan
	mSize.push_back(5150);
	mDist.push_back(1222000/scaleF);
	mMass.push_back(1350e20);
	//Iapetus
	mSize.push_back(1440);
	mDist.push_back(3560000/scaleF);
	mMass.push_back(20e20);

	//Uranus
	//Puck Miranda Ariel Umbriel Titania Oberon
	//Puck
	mSize.push_back(162);
	mDist.push_back(86004/scaleF);
	mMass.push_back(0.49e18);
	//Miranda incl. 4.232
	mSize.push_back(471.6);
	mDist.push_back(129390/scaleF);
	mMass.push_back(66e18);
	//Ariel
	mSize.push_back(1157.8);
	mDist.push_back(191020/scaleF);
	mMass.push_back(1350e18);
	//Umbriel
	mSize.push_back(1169.4);
	mDist.push_back(266300/scaleF);
	mMass.push_back(1170e18);
	//Titania
	mSize.push_back(1577.8);
	mDist.push_back(435910/scaleF);
	mMass.push_back(3530e18);
	//Oberon
	mSize.push_back(1522.8);
	mDist.push_back(583520/scaleF);
	mMass.push_back(3010e18);

	/*
	cassini1 orbit insertion

	Start Date MJD(J2000): -157.722000192

    r0[1] = -66781301.221897528, -85261785.277759820, 2689581.778579089
    v0[1] = 27.325053059, -21.757999784, -1.874572041

    r0[2] = 20463705.409188949, 105798706.757600859, 263606.196716629
    v0[2] = -34.502092332, 6.476518598, 2.080035337

    r0[3] = 45890230.425370567, -145000678.106011182, 0.000000000
    v0[3] = 27.915431535, 8.876579581, 0.000000000

    r0[4] = -709271704.120080948, -400307649.954340875, 17519444.363668837
    v0[4] = 6.265896006, -10.771960875, -0.095402591

    v1[4] = 8.351931241, 3.709450466, -0.397050412

    cassini_r[0][0] = -66781301.221897528;
	cassini_r[0][1] = 2689581.778579089;
	cassini_r[0][2] = -85261785.277759820;

	cassini_r[1][0] = 20463705.409188949;
	cassini_r[1][1] = 263606.196716629;
	cassini_r[1][2] = 105798706.757600859;

	cassini_r[2][0] = 45890230.425370567;
	cassini_r[2][1] = 0.0;
	cassini_r[2][2] = -145000678.106011182;

	cassini_r[3][0] = 709271704.120080948;
	cassini_r[3][1] = 17519444.363668837;
	cassini_r[3][2] = -400307649.954340875;

	//--------------------------------------------

	cassini_v[0][0] = 27.325053059;
	cassini_v[0][1] = -1.874572041;
	cassini_v[0][2] = -21.757999784;

	cassini_v[1][0] = -34.502092332;
	cassini_v[1][1] = 2.080035337;
	cassini_v[1][2] = 6.476518598;

	cassini_v[2][0] = 27.915431535;
	cassini_v[2][1] = 0.0;
	cassini_v[2][2] = 8.876579581;

	cassini_v[3][0] = 6.265896006;
	cassini_v[3][1] = -0.095402591;
	cassini_v[3][2] = -10.771960875;

	cassiniStartDay = -157.722000192;

    Start Date MJD(J2000): -782.171210700
    r0[1] = -26316298.668520022, 104246022.875278398, 2944846.158133036
    v0[1] = -34.074019324, -8.766711501, 1.846891662

    r0[2] = 93008466.585570976, 55372703.909920044, -4611211.468269892
    v0[2] = -29.181923790, 21.122212386, -0.646311822

    r0[3] = 143805316.423756003, -45511313.243546568, 0.000000000
    v0[3] = 26.624587677, 27.496579010, -0.121300613

    r0[4] = -27302764.193483740, 768014971.884933829, -2564188.608518883
    v0[4] = -14.190201904, 5.023407140, 0.734124201

	*/
    cassini_r[0][0] = -26316298.668520022;
	cassini_r[0][1] = 2944846.158133036;
	cassini_r[0][2] = 104246022.875278398;

	cassini_r[1][0] = 93008466.585570976;
	cassini_r[1][1] = -4611211.468269892;
	cassini_r[1][2] = 55372703.909920044;

	cassini_r[2][0] = 143805316.423756003;
	cassini_r[2][1] = 0.0;
	cassini_r[2][2] = -45511313.243546568;

	cassini_r[3][0] = -27302764.193483740;
	cassini_r[3][1] = -2564188.608518883;
	cassini_r[3][2] = 768014971.884933829;

	//--------------------------------------------

	cassini_v[0][0] = -34.074019324;
	cassini_v[0][1] = 1.846891662;
	cassini_v[0][2] = -8.766711501;

	cassini_v[1][0] = -29.181923790;
	cassini_v[1][1] = -0.646311822;
	cassini_v[1][2] = 21.122212386;

	cassini_v[2][0] = 26.624587677;
	cassini_v[2][1] = -0.121300613;
	cassini_v[2][2] = 27.496579010;

	cassini_v[3][0] = -14.190201904;
	cassini_v[3][1] = 0.734124201;
	cassini_v[3][2] = 5.023407140;

	cassiniStartDay = -782.171210700;
}
void readconfig()
{
	io::IXMLReader* xml = device->getFileSystem()->createXMLReader("../config.xml");
	pMass.set_used(10);
	while(xml && xml->read())
	{
		switch(xml->getNodeType())
		{
		case io::EXN_ELEMENT:
			{
				if (core::stringw("Mercury") == xml->getNodeName())
					pMass[0] = xml->getAttributeValueAsFloat(L"mass");
				else
					if (core::stringw("Venus") == xml->getNodeName())
						pMass[1] = xml->getAttributeValueAsFloat(L"mass");
					else
						if (core::stringw("Earth") == xml->getNodeName())
							pMass[2] = xml->getAttributeValueAsFloat(L"mass");
						else
							if (core::stringw("Mars") == xml->getNodeName())
								pMass[3] = xml->getAttributeValueAsFloat(L"mass");
							else
								if (core::stringw("Jupiter") == xml->getNodeName())
									pMass[4] = xml->getAttributeValueAsFloat(L"mass");
								else
									if (core::stringw("Saturn") == xml->getNodeName())
										pMass[5] = xml->getAttributeValueAsFloat(L"mass");
									else
										if (core::stringw("Uranus") == xml->getNodeName())
											pMass[6] = xml->getAttributeValueAsFloat(L"mass");
										else
											if (core::stringw("Neptune") == xml->getNodeName())
												pMass[7] = xml->getAttributeValueAsFloat(L"mass");
											else
												if (core::stringw("Pluto") == xml->getNodeName())
													pMass[8] = xml->getAttributeValueAsFloat(L"mass");
												else
													if (core::stringw("Sun") == xml->getNodeName())
														pMass[9] = xml->getAttributeValueAsFloat(L"mass");
													else
														if (core::stringw("CamSpeedUpDown") == xml->getNodeName())
															CspeedUD = (xml->getAttributeValueAsFloat(L"speed"));
														else
															if (core::stringw("CamSpeedLeftRight") == xml->getNodeName())
																CspeedLR = (xml->getAttributeValueAsFloat(L"speed"));
															else
																if (core::stringw("CamSpeedForwardBackward") == xml->getNodeName())
																	CspeedFB = (xml->getAttributeValueAsFloat(L"speed"));
																else
																	if (core::stringw("Timestep") == xml->getNodeName())
																		timescale = xml->getAttributeValueAsFloat(L"step");
																	else
																		if (core::stringw("year") == xml->getNodeName())
																			Syear = xml->getAttributeValueAsInt(L"is");
																		else
																			if (core::stringw("month") == xml->getNodeName())
																				Smonth = xml->getAttributeValueAsInt(L"is");
																			else
																				if (core::stringw("day") == xml->getNodeName())
																					Sday = xml->getAttributeValueAsInt(L"is");
																				else
																					if (core::stringw("hour") == xml->getNodeName())
																						Shour = xml->getAttributeValueAsInt(L"is");
																					else
																						if (core::stringw("minute") == xml->getNodeName())
																							Sminute = xml->getAttributeValueAsInt(L"is");
																						else
																							if (core::stringw("second") == xml->getNodeName())
																								Ssecond = xml->getAttributeValueAsInt(L"is");
			}
			break;
		default:
			break;
		}
	}
	CspeedLR = (CspeedLR*timescale/scaleF)/1500;
	CspeedUD = (CspeedUD*timescale/scaleF)/1500;
	CspeedFB = (CspeedFB*timescale/scaleF)/1500;
	if(xml)
		xml->drop(); // don't forget to delete the xml reader
}

//===============================================================================================================================
//OBJECT CREATION
//===============================================================================================================================

void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
{
	int i;
	float f, p, q, t;
	if( s == 0 ) {
		// achromatic (grey)
		*r = *g = *b = v;
		return;
	}
	h /= 60;			// sector 0 to 5
	i = floor( h );
	f = h - i;			// factorial part of h
	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );
	switch( i ) {
		case 0:
			*r = v;
			*g = t;
			*b = p;
			break;
		case 1:
			*r = q;
			*g = v;
			*b = p;
			break;
		case 2:
			*r = p;
			*g = v;
			*b = t;
			break;
		case 3:
			*r = p;
			*g = q;
			*b = v;
			break;
		case 4:
			*r = t;
			*g = p;
			*b = v;
			break;
		default:		// case 5:
			*r = v;
			*g = p;
			*b = q;
			break;
	}
}
scene::ISceneNode* createDebugAxis(core::vector3df size)
{
	//Create a huge world axis for debug purposes
	scene::ISceneNode* axis = smgr->addEmptySceneNode();
	scene::IMesh* axisM = smgr->getMesh("../data/axis.B3D");
	axisM->setHardwareMappingHint(scene::EHM_STATIC);
	axis = smgr->addMeshSceneNode(axisM);
	axis->setMaterialFlag(video::EMF_LIGHTING, false);
	axis->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
	axis->setMaterialFlag(video::EMF_WIREFRAME, true);
	axis->setScale(size);
	return axis;

}
scene::ISceneNode* createDebugPlane(core::vector3df size, core::vector3df rot)
{
	//Create a huge Plane for debug purposes
	scene::ISceneNode* plane = smgr->addEmptySceneNode();
	scene::IMesh* planeM = smgr->getMesh("../data/debugPlane.B3D");
	planeM->setHardwareMappingHint(scene::EHM_STATIC);
	plane = smgr->addMeshSceneNode(planeM);
	plane->setMaterialFlag(video::EMF_LIGHTING, false);
	plane->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
	plane->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	plane->setScale(size);
	plane->setRotation(rot);
	return plane;

}
scene::ISceneNode* makeLine(core::array<core::vector3df> positions, video::SColor color, f32 thickness)
{
	scene::ISceneNode* returnNode = smgr->addEmptySceneNode();


	u32 originalSize = 2000;
	u32 SegmentsNeeded = core::ceil32( ((f32)positions.size()/(f32)originalSize) );

	u32 SegmentSize = 2000;
	u32 finishedSegments = 2000;

	if(SegmentsNeeded == 1)
	{
		SegmentSize = positions.size();
		finishedSegments = positions.size();
	}

	for(u32 i=0; i<SegmentsNeeded; i++)
	{
		scene::SMeshBuffer *segmentBuffer = new scene::SMeshBuffer();

		for(u32 j=0; j<SegmentSize; j++)
		{
			segmentBuffer->Indices.push_back(j);

			if(j !=0 && j != positions.size() - 1)
				segmentBuffer->Indices.push_back(j);

			segmentBuffer->Vertices.push_back(video::S3DVertex(
				positions[i*originalSize+j].X,
				positions[i*originalSize+j].Y,
				positions[i*originalSize+j].Z,
				1,1,1,
				color,
				1,1));
		}

		segmentBuffer->getMaterial().Thickness = thickness;

		CIndexedPrimitiveNode* segNode = new CIndexedPrimitiveNode(returnNode,smgr,-1,segmentBuffer,
			segmentBuffer->Indices.size()/2,
			scene::EPT_LINES);

		segNode->setMaterialFlag(video::EMF_LIGHTING, false);
		segNode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);

		if(finishedSegments + 2000 < positions.size())
		{
			finishedSegments += 2000;
		}
		else
		{
			u32 remainder = positions.size() - finishedSegments;
			SegmentSize = remainder;
		}
	}

	return returnNode;
}
scene::ISceneNode* makeLineDouble(core::array< core::array<f64> > positions, core::array< core::array<f64> > velocity, video::SColor color, f32 thickness)
{
	scene::ISceneNode* returnNode = smgr->addEmptySceneNode();


	u32 originalSize = 2000;
	u32 SegmentsNeeded = core::ceil32( ((f32)positions.size()/(f32)originalSize) );

	u32 SegmentSize = 2000;
	u32 finishedSegments = 2000;

	if(SegmentsNeeded == 1)
	{
		SegmentSize = positions.size();
		finishedSegments = positions.size();
	}
	
	f64 max = 0;

	for(u32 v=0; v<velocity.size(); v++)
	{
		f64 current = mag(velocity[v]);
		if(current > max)
			max = current;
	}

	for(u32 i=0; i<SegmentsNeeded; i++)
	{
		scene::SMeshBuffer *segmentBuffer = new scene::SMeshBuffer();

		for(u32 j=0; j<SegmentSize; j++)
		{
			segmentBuffer->Indices.push_back(j);

			if(j !=0 && j != positions.size() - 1)
				segmentBuffer->Indices.push_back(j);
			
			f64 currentVel = mag(velocity[i*originalSize+j]);
			f64 percentVel = currentVel/max;
			f32 red,green,blue;

			HSVtoRGB(&red,&green,&blue,percentVel*360,1,1);
			video::SColor color3 = video::SColor(255,red*255,green*255,blue*255);
			
			/*
			
			Rainbow colors
			///////////////////////
			f64 sunDist;
			f64 satToSun[3];
			satToSun[0] = positions[i*originalSize+j][0] - SS->doublePositions[9][0];
			satToSun[1] = positions[i*originalSize+j][1] - SS->doublePositions[9][1];
			satToSun[2] = positions[i*originalSize+j][2] - SS->doublePositions[9][2];
			sunDist = mag(satToSun);

			f64 plutoDist = pMeanDist[3];
			f64 percentToPluto = sunDist/plutoDist;
			f32 degreeToPluto = percentToPluto*360;
			if(degreeToPluto > 360)
				degreeToPluto - 360.;
			f32 red,green,blue;
			///////////////////////

			u32 closestPlanet = 0;
			f64 closeDist = 99999999999999999;
			f64 sunDist;
			f64 satToSun[3];

			for(u32 c=0; c<8; c++)
			{
				f64 dist = core::abs_(pMeanDist[c]-sunDist);
				
				if(dist<closeDist)
				{
					closestPlanet = c;
					closeDist = dist;
				}
			}
			
			satToSun[0] = positions[i*originalSize+j][0] - SS->doublePositions[9][0];
			satToSun[1] = positions[i*originalSize+j][1] - SS->doublePositions[9][1];
			satToSun[2] = positions[i*originalSize+j][2] - SS->doublePositions[9][2];
			sunDist = mag(satToSun);
				
			video::SColor color3 = OrbitCol[closestPlanet];
			
			f64 totalDistanceFromCurrentToNext = pMeanDist[closestPlanet+1] - pMeanDist[closestPlanet];
			f64 currentDistanceFromCurrentToNext = pMeanDist[closestPlanet+1] - sunDist;
			f64 percentToNex = currentDistanceFromCurrentToNext/totalDistanceFromCurrentToNext;
			
			printf("%f\n",percentToNex);

			f32 RStart = OrbitCol[closestPlanet].getRed();
			f32 GStart = OrbitCol[closestPlanet].getGreen();
			f32 BStart = OrbitCol[closestPlanet].getBlue();

			f32 RTarget = OrbitCol[closestPlanet+1].getRed();
			f32 GTarget = OrbitCol[closestPlanet+1].getGreen();
			f32 BTarget = OrbitCol[closestPlanet+1].getBlue();

			f32 ROffset = RTarget-RStart;
			f32 GOffset = GTarget-GStart;
			f32 BOffset = BTarget-BStart;

			color3 = video::SColor(255,
									 RStart+ROffset*percentToNex,
									 BStart+GOffset*percentToNex,
									 GStart+BOffset*percentToNex);
			
			*/
			
			segmentBuffer->Vertices.push_back(video::S3DVertex(
				positions[i*originalSize+j][0]/scaleF,
				positions[i*originalSize+j][1]/scaleF,
				positions[i*originalSize+j][2]/scaleF,
				1,1,1,
				color3,
				1,1));
		}

		segmentBuffer->getMaterial().Thickness = thickness;

		CIndexedPrimitiveNode* segNode = new CIndexedPrimitiveNode(returnNode,smgr,-1,segmentBuffer,
			segmentBuffer->Indices.size()/2,
			scene::EPT_LINES);

		segNode->setMaterialFlag(video::EMF_LIGHTING, false);
		segNode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);

		if(finishedSegments + 2000 < positions.size())
		{
			finishedSegments += 2000;
		}
		else
		{
			u32 remainder = positions.size() - finishedSegments;
			SegmentSize = remainder;
		}
	}

	return returnNode;
}
CIndexedPrimitiveNode* makeTrailLine(core::array<scene::ISceneNode*> nodes, f32 thickness)
{
	//scene::ISceneNode* returnNode = smgr->addEmptySceneNode();
	scene::SMeshBuffer *segmentBuffer = new scene::SMeshBuffer();

	f32 colorFaktor = 255.0/(f32)nodes.size();

	u32 nextInOrder = 0;

	for(u32 j=0; j<nodes.size(); j++)
	{
		segmentBuffer->Indices.push_back(j);

		if(j !=0 && j != trailcount - 1)
			segmentBuffer->Indices.push_back(j);

		video::SColor color = video::SColor(255,255,j*colorFaktor,0);

		segmentBuffer->Vertices.push_back(video::S3DVertex(
			nodes[j]->getPosition().X,
			nodes[j]->getPosition().Y,
			nodes[j]->getPosition().Z,
			1,1,1,
			color,
			1,1));

	}


	segmentBuffer->getMaterial().Thickness = thickness;

	CIndexedPrimitiveNode* segNode = new CIndexedPrimitiveNode(smgr->getRootSceneNode(),smgr,-1,segmentBuffer,
		segmentBuffer->Indices.size()/2,
		scene::EPT_LINES);

	segNode->setMaterialFlag(video::EMF_LIGHTING, false);
	segNode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);

	return segNode;

}
CIndexedPrimitiveNode* makeTrailLine2(core::array<core::vector3df> nodes, f32 thickness)
{
	//scene::ISceneNode* returnNode = smgr->addEmptySceneNode();
	scene::SMeshBuffer *segmentBuffer = new scene::SMeshBuffer();

	f32 colorFaktor = 255.0/(f32)nodes.size();

	u32 nextInOrder = 0;

	for(u32 j=0; j<nodes.size(); j++)
	{
		segmentBuffer->Indices.push_back(j);

		if(j !=0 && j != trailcount - 1)
			segmentBuffer->Indices.push_back(j);

		video::SColor color = video::SColor(255,255,j*colorFaktor,0);

		segmentBuffer->Vertices.push_back(video::S3DVertex(
			nodes[j].X,
			nodes[j].Y,
			nodes[j].Z,
			1,1,1,
			color,
			1,1));

	}


	segmentBuffer->getMaterial().Thickness = thickness;

	CIndexedPrimitiveNode* segNode = new CIndexedPrimitiveNode(smgr->getRootSceneNode(),smgr,-1,segmentBuffer,
		segmentBuffer->Indices.size()/2,
		scene::EPT_LINES);

	segNode->setMaterialFlag(video::EMF_LIGHTING, false);
	segNode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);

	return segNode;

}
core::array<scene::ISceneNode*> makeEllipse(f64 semiMajor, f64 ecc, f64 incl, f64 longiA, f64 longiP, video::SColor color, f32 thicknes, u32 steps)
{
	/*
	optimized orbit generation based on new and good working method:
	-makeEllipse()
	which returns an array of scene::ISceneNode's containing
	[0] orbialPlane turned about -1 * longitude of Ascending Node and later by -1 * inclination
	|_[1] argPerig  turned about -1 * (longitude of Perihelion - longitude of Ascending Node)
	   |_[2] mOLine no Rotation but points on X are offset by -foci
		  ｜[3] debugPlane2 is positioned on on X by -offset


	inclination
	Inclination is the angle between the orbital plane and the equatorial plane.
	By convention, inclination is a number between 0 and 180 degrees.

	longitude of ascending node

	Two numbers orient the orbital plane in space. The first number was Inclination.
	This is the second. The longitude of ascending node is an angle, measured at
	the focus, from the vernal equinox eastwards to the ascending node.

	longitude of perihelion

	The angle measured from the vernal equinox eastward along the ecliptic to the
	ascending node of a planet's orbit, and then continued eastward along the orbital
	plane to the perihelion. It is one of the orbital elements.

	wiki:
	In astrodynamics, the longitude of the periapsis of an orbiting body
	is the longitude (measured from the point of the vernal equinox) at which the periapsis
	(closest approach to the central body) would occur if the body's inclination were zero.
	For motion of a planet around the sun, this position could be called longitude of perihelion.
	The longitude of periapsis is a compound angle, with part of it being measured in the plane
	of reference and the rest being measured in the plane of the orbit. Likewise, any angle derived
	from the longitude of periapsis (e.g. mean longitude and true longitude) will also be compound.
	Sometimes, the term longitude of periapsis is used to refer to ω, the angle between the ascending
	node and the periapsis. That usage of the term is especially common in discussions of binary
	stars and exoplanets.[1] However, the angle ω is less ambiguously known as the argument of periapsis.

	The

	Argument of peiapsis would be longitude of perihelion minus longitude of ascending node

	*/

	video::ITexture *peri = driver->getTexture("../data/perihelion.jpg");
	video::ITexture *asce = driver->getTexture("../data/ascending.jpg");

	f64 semiMinor = semiMajor*sqrt(1.0-pow(ecc,2));
	f64 foci = sqrt(pow(semiMajor,2)-pow(semiMinor,2));
	//printf("%f\n",foci);
	f64 angle = 0;
	f64 angleDelta = 2.0 * core::PI64 / steps;

	scene::ISceneNode* mOLine;
	scene::ISceneNode* orbPlane = smgr->addEmptySceneNode(orbitsM);
	orbPlane->setRotation(core::vector3df(0,-1.0*longiA,0));

	scene::ISceneNode* argPerig = smgr->addEmptySceneNode(orbPlane);
	argPerig->setRotation(core::vector3df(0,longiP*-1.0,0));

	f64 ax, ay;
	f32 ascending = longiA;
	f32 perihelion = longiP;
	bool periSet = false;
	bool asceSet = false;


	
	u32 segments = steps / 14;
	u32 segsize = steps / segments;
	u32 s = 0;
	while(s < segments)
	{
		core::array<core::vector3df> mOPoints;
		for(u32 i = 0; i < segsize + 2; i++)
		{
			angle += angleDelta;
			ax = -foci - cos(angle)*semiMajor;
			ay = sin(angle)*semiMinor;

			core::vector3df result = core::vector3df(ax,0,ay);
			mOPoints.push_back(result);

			f32 currentDeg = result.getHorizontalAngle().Y;

			if(core::abs_(currentDeg - (ascending + 90.0)) <= 1)
			{

				if(!asceSet)
				{
					printf("%.9f\n",currentDeg);
					scene::IBillboardSceneNode* ascendingB = smgr->addBillboardSceneNode(argPerig,core::dimension2df(50,50));
					ascendingB->setPosition(result);
					ascendingB->updateAbsolutePosition();
					core::vector3df absPos = ascendingB->getAbsolutePosition();
					printf("POS.%.9f,%.9f,%.9f\n",absPos.X,absPos.Y,absPos.Z);
					ascendingB->setMaterialTexture(0,asce);
					ascendingB->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
					ascendingB->setMaterialFlag(video::EMF_LIGHTING, false);
					asceSet = true;
					ascendingNodes.push_back(ascendingB);

				}
			}
			if(core::abs_(currentDeg - (perihelion-ascending)) <= 1)
			{
				if(!periSet)
				{
					scene::IBillboardSceneNode* perihelionB = smgr->addBillboardSceneNode(argPerig,core::dimension2df(50,50));
					perihelionB->setPosition(result);
					perihelionB->setMaterialTexture(0,peri);
					perihelionB->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
					perihelionB->setMaterialFlag(video::EMF_LIGHTING, false);
					periSet = true;
					perihelions.push_back(perihelionB);
				}
			}
		}
		mOLine = makeLine(mOPoints,color,thicknes);
		mOLine->setParent(argPerig);
		s++;
	}

	

	orbPlane->setRotation(core::vector3df(-1.0*incl,orbPlane->getRotation().Y,orbPlane->getRotation().Z));

	scene::ISceneNode* debugPlane2 = createDebugPlane(core::vector3df(semiMajor/200,semiMajor/200,semiMinor/200),core::vector3df(0,0,0));
	debugPlane2->setParent(mOLine);
	debugPlane2->setPosition(core::vector3df(-foci,0,0));
	debugPlane2->setVisible(false);

	core::array<scene::ISceneNode*> planes;
	planes.push_back(orbPlane);
	planes.push_back(argPerig);
	planes.push_back(mOLine);
	planes.push_back(debugPlane2);

	return planes;

}
core::array<scene::ISceneNode*> makeOrbitalPlane()
{
	/*
	optimized orbit generation based on new and good working method:
	-makeEllipse()
	which returns an array of scene::ISceneNode's containing
	[0] orbialPlane turned about -1 * longitude of Ascending Node and later by -1 * inclination
	|_[1] argPerig  turned about -1 * (longitude of Perihelion - longitude of Ascending Node)
	   |_[2] debugPlane is positioned on on X by -offset
	*/

	

	scene::ISceneNode* orbPlane = smgr->addEmptySceneNode();
	orbPlane->setScale(core::vector3df(0.005,0.005,0.005));
	scene::ISceneNode* argPerig = smgr->addEmptySceneNode(orbPlane);
	
	scene::ISceneNode* debugPlane = createDebugPlane(core::vector3df(1,1,1),core::vector3df(0,0,0));
	debugPlane->setParent(argPerig);
	
	core::array<scene::ISceneNode*> planes;
	planes.push_back(orbPlane);
	planes.push_back(argPerig);
	planes.push_back(debugPlane);

	return planes;

}
void stars2()
{

	//create Billboards for the star Sphere
	video::ITexture *star1 =  driver->getTexture("../data/starbill0.jpg");
	video::ITexture *star2 =  driver->getTexture("../data/starbill1.jpg");
	video::ITexture *star3 =  driver->getTexture("../data/starbill2.jpg");
	video::ITexture *star4 =  driver->getTexture("../data/starbill3.jpg");
	video::ITexture *star5 =  driver->getTexture("../data/starbill4.jpg");
	video::ITexture *star6 =  driver->getTexture("../data/starbill5.jpg");
	//63672 / 7 = 9096
	//create a sceneNode as parent for all billboards and create all stars
	//based on the Bright Star Catalouge 5 included in the header bsc.h
	allstars = smgr->addEmptySceneNode();
	for(s32 s = 0; s < 63672 ; s += 7){

		f64 hourDeg = 360.0f/24.0f;
		f64 minuteDeg = 360.0f/1440.0f;
		f64 secDeg = 360.0f/86400.0f;
		f64 arcMin = 1.0f/60.0f;
		f64 arcSec = incrementF;

		f64 ra = (bsc2[s]*hourDeg) + (bsc2[s+1]*minuteDeg) + (bsc2[s+2]*secDeg);
		f64 dec;
		if(bsc2[s+3]>0)
			dec = (bsc2[s+3]) + (bsc2[s+4]/60.0f) + ((bsc2[s+5]/360.0f)*arcMin);
		else
			dec = (bsc2[s+3]) - (bsc2[s+4]/60.0f) - ((bsc2[s+5]/360.0f)*arcMin);
		f32 Yrot = (ra);
		f32 Xrot = (dec)*-1.0f;


		core::vector3df direction = core::vector3df(Xrot,Yrot,-23.5f);
		core::vector3df directionV = direction.rotationToDirection();
		core::vector3df directionVx = directionV * 40000.0f;

		//printf("%f,%f,%f\n",directionVx.X,directionVx.Y,directionVx.Z);
		scene::IBillboardSceneNode* star = smgr->addBillboardSceneNode(allstars,
			core::dimension2df((7.2f-bsc2[s+6])*302.0f,(7.2f-bsc2[s+6])*150.0f),
			//core::dimension2df(350.0f,350.0f),
			directionVx,
			-1);
		star->setMaterialFlag(video::EMF_LIGHTING, false);
		star->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
		star->setMaterialTexture(0,star2);
	}

}
void stars1()
{

	//create Billboards for the star Sphere
	video::ITexture *star1 =  driver->getTexture("../data/starbill0.jpg");
	video::ITexture *star2 =  driver->getTexture("../data/starbill1.jpg");
	video::ITexture *star3 =  driver->getTexture("../data/starbill2.jpg");
	video::ITexture *star4 =  driver->getTexture("../data/starbill3.jpg");
	video::ITexture *star5 =  driver->getTexture("../data/starbill4.jpg");
	video::ITexture *star6 =  driver->getTexture("../data/starbill5.jpg");
	//36384 / 4 = 9096
	//create a sceneNode as parent for all billboards and create all stars
	//based on the Bright Star Catalouge 5 included in the header bsc.h
	allstars = smgr->addEmptySceneNode();
	for(s32 s = 0; s < 36384 ; s += 4){
		scene::IBillboardSceneNode* star = smgr->addBillboardSceneNode(allstars,
			core::dimension2df(bsc[s+3]/1024,bsc[s+3]/1024),
			core::vector3df(bsc[s]/160.0, bsc[s+1]/160.0, bsc[s+2]/160.0),-1);
		star->setMaterialFlag(video::EMF_LIGHTING, false);
		star->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
		star->setMaterialTexture(0,star6);

		u16 spectT = spect[s];

		switch(spectT)
		{
		case 0:
			star->setMaterialTexture(0,star6);
			break;
		case 1:
			star->setMaterialTexture(0,star5);
			break;
		case 2:
			star->setMaterialTexture(0,star4);
			break;
		case 3:
			star->setMaterialTexture(0,star3);
			break;
		case 4:
			star->setMaterialTexture(0,star2);
			break;
		case 5:
			star->setMaterialTexture(0,star1);
			break;
		}

	}
	allstars->setRotation(core::vector3df(23.5f,0,0));
	allstars->setRotation(allstars->getRotation() + core::vector3df(-90,90,0));
	allstars->setScale(core::vector3df(1,-1,1));

}
void plotCassini()
{
    //Day = cassiniStartDay;

    scene::ISceneNode* pos1 = createDebugAxis(core::vector3df(1.1,1.1,1.1));
    scene::ISceneNode* pos2 = createDebugAxis(core::vector3df(1.1,2.1,1.1));
    scene::ISceneNode* pos3 = createDebugAxis(core::vector3df(1.1,3.1,1.1));
    scene::ISceneNode* pos4 = createDebugAxis(core::vector3df(1.1,4.1,1.1));

    pos1->setPosition(core::vector3df(cassini_r[0][0]/scaleF,
									  cassini_r[0][1]/scaleF,
									  cassini_r[0][2]/scaleF));
    pos2->setPosition(core::vector3df(cassini_r[1][0]/scaleF,
									  cassini_r[1][1]/scaleF,
									  cassini_r[1][2]/scaleF));
    pos3->setPosition(core::vector3df(cassini_r[2][0]/scaleF,
									  cassini_r[2][1]/scaleF,
									  cassini_r[2][2]/scaleF));
    pos4->setPosition(core::vector3df(cassini_r[3][0]/scaleF,
									  cassini_r[3][1]/scaleF,
									  cassini_r[3][2]/scaleF));

    pos1->setParent(orbitsM);
    pos2->setParent(orbitsM);
    pos3->setParent(orbitsM);
    pos4->setParent(orbitsM);

    cassiniPoints.push_back(pos1);
    cassiniPoints.push_back(pos2);
    cassiniPoints.push_back(pos3);
    cassiniPoints.push_back(pos4);

    core::array<core::vector3df> wholeTrajectory;

    f64 currentPos[3];
    f64 currentVel[3];
    f64 fromCurrentPosToNextLeg[3];
    f64 distance = 9e99;

    currentPos[0] = cassini_r[0][0];
    currentPos[1] = cassini_r[0][1];
    currentPos[2] = cassini_r[0][2];

    currentVel[0] = cassini_v[0][0];
    currentVel[1] = cassini_v[0][1];
    currentVel[2] = cassini_v[0][2];


    for(u32 i=0; i<4; i++)
    {

        if(i<=2)
        {
            core::vector3df current = double2V(currentPos);
            core::vector3df next = core::vector3df(cassini_r[i+1][0], 
												   cassini_r[i+1][1], 
												   cassini_r[i+1][2]);
            distance = current.getDistanceFrom(next);
        }

        f64 olddist = 9e99;

        while(distance > 1500000.0)
        //for(u32 j=0; j<25000; j++)
        {
            core::vector3df current = double2V(currentPos);
            
			core::vector3df next = core::vector3df(cassini_r[i+1][0], 
												   cassini_r[i+1][1], 
												   cassini_r[i+1][2]);
            
			distance = current.getDistanceFrom(next);

            if(distance < olddist)
            {
                olddist = distance;
                //printf("Distance to %d = %.9f\n",i+1,distance);
            }

            //printf("CurrentPos = %.9f,%.9f,%.9f\n",currentPos[0],currentPos[1],currentPos[2]);
            propagateKEPInOut(currentPos,currentVel,2400,GconstKM*pMass[9]);
            wholeTrajectory.push_back(core::vector3df(currentPos[0]/scaleF,currentPos[1]/scaleF,currentPos[2]/scaleF));
        }


        if(i<=2)
        {
            currentPos[0] = cassini_r[i+1][0];
            currentPos[1] = cassini_r[i+1][1];
            currentPos[2] = cassini_r[i+1][2];

            currentVel[0] = cassini_v[i+1][0];
            currentVel[1] = cassini_v[i+1][1];
            currentVel[2] = cassini_v[i+1][2];
        }

    }

    scene::ISceneNode* traj = makeLine(wholeTrajectory,video::SColor(255,255,255,255),3);
    traj->setParent(orbitsM);

}

//===============================================================================================================================
//COORDINATE TRANSFORMATIONS
//===============================================================================================================================

void makeRelativePosTo(u32 p, f64 pos[3])
{
	f64 planetPos[3];
	planetPos[0] = SS->doublePositions[p][0];
	planetPos[1] = SS->doublePositions[p][1];
	planetPos[2] = SS->doublePositions[p][2];

	pos[0] = pos[0] - planetPos[0];
	pos[1] = pos[1] - planetPos[1];
	pos[2] = pos[2] - planetPos[2];
}
void makeRelativeVelTo(u32 p, f64 vel[3])
{
	f64 planetVel[3];
	planetVel[0] = SS->doubleVelocities[p][0];
	planetVel[1] = SS->doubleVelocities[p][1];
	planetVel[2] = SS->doubleVelocities[p][2];

	vel[0] = vel[0] - planetVel[0];
	vel[1] = vel[1] - planetVel[1];
	vel[2] = vel[2] - planetVel[2];
}
void makeAbsolutePosTo(u32 p, f64 pos[3])
{
	f64 planetPos[3];
	planetPos[0] = SS->doublePositions[p][0];
	planetPos[1] = SS->doublePositions[p][1];
	planetPos[2] = SS->doublePositions[p][2];

	pos[0] = pos[0] + planetPos[0];
	pos[1] = pos[1] + planetPos[1];
	pos[2] = pos[2] + planetPos[2];
}
void makeAbsoluteVelTo(u32 p, f64 vel[3])
{
	f64 planetVel[3];
	planetVel[0] = SS->doubleVelocities[p][0];
	planetVel[1] = SS->doubleVelocities[p][1];
	planetVel[2] = SS->doubleVelocities[p][2];

	vel[0] = vel[0] + planetVel[0];
	vel[1] = vel[1] + planetVel[1];
	vel[2] = vel[2] + planetVel[2];
}
void setSolarOffset(f64 offsetDouble[])
{
	SS->offsetDouble[0] = offsetDouble[0]*scaleF;
	SS->offsetDouble[1] = offsetDouble[1]*scaleF;
	SS->offsetDouble[2] = offsetDouble[2]*scaleF;
}
void translateWorld(f64 offsetDouble[])
{
	//update Planets--------------------------------------------------------------------------
	SS->updateAtDay(Day,-1);
	for(u32 i=0; i < 9; i++)
	{
		planets[i]->setPosition(double2V(SS->doublePositions[i])/scaleF);
		planets[i]->updateAbsolutePosition();
	}
	//update Sun------------------------------------------------------------------------------
	planets[9]->setPosition(double2V(offsetDouble));

	//update Light----------------------------------------------------------------------------
	light->setPosition(double2V(offsetDouble));

	//update orbits---------------------------------------------------------------------------
	orbitsM->setPosition(double2V(offsetDouble));
	
	if(axis)
		axis->setPosition(double2V(offsetDouble));

}
void resetWorld()
{
	offsetDouble[0] = ((doublePosition[0]/scaleF)-offsetDouble[0])*-1.0;
	offsetDouble[1] = ((doublePosition[1]/scaleF)-offsetDouble[1])*-1.0;
	offsetDouble[2] = ((doublePosition[2]/scaleF)-offsetDouble[2])*-1.0;

	setSolarOffset(offsetDouble);
	translateWorld(offsetDouble);
}
void irrlichtToHeliocentric(f64 in[3])
{	
	f64 transform[3];
	
	transform[0] = in[0];
	transform[1] = in[1];
	transform[2] = in[2];

	in[0] = transform[0];
	in[1] = transform[2];
	in[2] = transform[1];
}

//===============================================================================================================================
//SOLAR SYSTEM CREATION
//===============================================================================================================================

void createPlanets()
{
	//Create earth Moon Orbit
	f64 moonIn360Steps = 27.321582/360.0;
	f64 orbitsDay = Day;
	core::array<core::vector3df> mPosA;

	for(u32 i=0; i<361; i++)
	{
		f64 lambda;
		core::vector3df mPos;
		moonpos(orbitsDay,&mPos,&lambda);
		mPosA.push_back(mPos/scaleF);
		orbitsDay += moonIn360Steps;
	}

	scene::ISceneNode * mOLine = makeLine(mPosA,video::SColor(255,180,180,180),3);
	
	//load all the textures
	video::ITexture *tex1 =  driver->getTexture("../data/mercurymap.jpg");
	video::ITexture *tex2 =  driver->getTexture("../data/mercurybump.jpg");
	video::ITexture *tex3 =  driver->getTexture("../data/venusmap.jpg");
	video::ITexture *tex4 =  driver->getTexture("../data/venusbump.jpg");
	video::ITexture *tex5 =  driver->getTexture("../data/earthmap1k.jpg");
	video::ITexture *tex6 =  driver->getTexture("../data/earthbump1k.jpg");
	video::ITexture *tex7 =  driver->getTexture("../data/mars_1k_color.jpg");
	video::ITexture *tex8 =  driver->getTexture("../data/mars_1k_topo.jpg");
	video::ITexture *tex9 =  driver->getTexture("../data/jupitermap.jpg");
	video::ITexture *tex10 =  driver->getTexture("../data/nobump.jpg");
	video::ITexture *tex11 =  driver->getTexture("../data/saturnmap.jpg");
	video::ITexture *tex13 =  driver->getTexture("../data/uranusmap.jpg");
	video::ITexture *tex15 =  driver->getTexture("../data/neptunemap.jpg");
	video::ITexture *tex17 =  driver->getTexture("../data/plutomap1k.jpg");
	video::ITexture *tex18 =  driver->getTexture("../data/plutobump1k.jpg");
	video::ITexture *tex19 =  driver->getTexture("../data/earthcloudmap.tga");
	video::ITexture *tex20 =  driver->getTexture("../data/ebill.jpg");
	video::ITexture *tex21 =  driver->getTexture("../data/saturnringcolor.tga");
	video::ITexture *tex22 =  driver->getTexture("../data/moon512.jpg");
	video::ITexture *tex23 =  driver->getTexture("../data/moon_height.jpg");
	video::ITexture *tex24 =  driver->getTexture("../data/io.jpg");
	video::ITexture *tex25 =  driver->getTexture("../data/io_height.jpg");
	video::ITexture *tex26 =  driver->getTexture("../data/europa.jpg");
	video::ITexture *tex27 =  driver->getTexture("../data/europa_height.jpg");
	video::ITexture *tex28 =  driver->getTexture("../data/ganymede.jpg");
	video::ITexture *tex29 =  driver->getTexture("../data/ganymede_height.jpg");
	video::ITexture *tex30 =  driver->getTexture("../data/callisto.jpg");
	video::ITexture *tex31 =  driver->getTexture("../data/europa_height.jpg");
	video::ITexture *sunbill =  driver->getTexture("../data/bill.jpg");
	video::ITexture *sun =  driver->getTexture("../data/diffuse.jpg");
	video::ITexture *checktex =  driver->getTexture("../data/gridFarbe2.jpg");

	//make space to hold the meshes
	scene::IAnimatedMesh* ringMesh;
	scene::IAnimatedMesh* planetMesh;
	scene::IAnimatedMesh* moonMesh;
	scene::IAnimatedMesh* sunMesh;
	scene::IAnimatedMesh* atmoMesh;
	scene::IAnimatedMesh* SOIMesh;

	scene::IMesh* tangentMesh;
	scene::IBillboardSceneNode* earthBill;
	scene::ISceneNodeAnimator* rota;

	scene::IMesh* PlanetNodeAxisM;
	scene::IMesh* PlanetAxisM;
	scene::IMesh* PlanetAxisM2;

	core::matrix4 transformMesh;
	//create the SceneNodes for hierarchy as follows
	/*

	planetNode		= is driven by solar.cpp and pushed into array<scene::ISceneNode*> planets
	|
	|--->planet		= is rotated according to sideral rotation period and pushed into array<scene::ISceneNode*> planetsR
	|	 |--->atmo
	|	 |--->ring
	|
	|---->moonNode	= is positioned according to moons orbital period and pushed into array<scene::ISceneNode*> moons
	|--->moon  = is rotated according to moons sideral rotation period and pushed into array<scene::ISceneNode*> moonsR
	*/

	for(u32 i=0; i<9; i++)
	{

		scene::ISceneNode* planetNode;
		scene::ISceneNode* SOInode;
		scene::ISceneNode* planet;
		scene::ISceneNode* atmo;
		scene::ISceneNode* ring;
		scene::ISceneNode* moonNode;
		scene::ISceneNode* moon;

		scene::ISceneNode* PlanetNodeAxis;
		scene::ISceneNode* PlanetAxis;
		scene::ISceneNode* PlanetAxis2;

		core::matrix4 mTmatr;
		core::vector3df scale;
		core::vector3df SOIscale;

		core::vector3df pos;
		f64 lambda;
		core::stringw date;

		switch(i)
		{
	case 0:
		planetNode = smgr->addEmptySceneNode();
		planetMesh = smgr->addSphereMesh("Mercury",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(planetMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((planetSize[i]/scaleF),(planetSize[i]/scaleF),(planetSize[i]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		planet = smgr->addMeshSceneNode(tangentMesh);
		planet->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
		planet->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		planet->setMaterialTexture(0,tex1);
		planet->setMaterialTexture(1,tex2);
		//planet->getMaterial(0).SpecularColor.set(255,255,255,255);
		//planet->getMaterial(0).Shininess = 3800;
		planet->setRotation(core::vector3df(0,0,-0.1));
		planetNode->setName("mercury");

		SOInode = smgr->addEmptySceneNode();
		SOIMesh = smgr->addSphereMesh("SOI",1.0,48,48);
		SOIMesh->setHardwareMappingHint(scene::EHM_STATIC);
		smgr->getMeshManipulator()->setVertexColors(SOIMesh,OrbitCol[i]);
		SOIscale = core::vector3df((pSOI[i]/scaleF),(pSOI[i]/scaleF),(pSOI[i]/scaleF));
		smgr->getMeshManipulator()->scale(SOIMesh,SOIscale);
		SOInode = smgr->addMeshSceneNode(SOIMesh);
		SOInode->setMaterialFlag(video::EMF_WIREFRAME, true);
		SOInode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		SOInode->setVisible(false);
		SOInode->setParent(planetNode);
		SOINodes.push_back(SOInode);

		planet->setParent(planetNode);
		planets.push_back(planetNode);
		planetsR.push_back(planet);
		tangentMesh->drop();
		break;
	case 1:
		planetNode = smgr->addEmptySceneNode();
		planetMesh = smgr->addSphereMesh("Venus",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(planetMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((planetSize[i]/scaleF),(planetSize[i]/scaleF),(planetSize[i]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		planet = smgr->addMeshSceneNode(tangentMesh);
		planet->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
		planet->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		planet->setMaterialTexture(0,tex3);
		planet->setMaterialTexture(1,tex4);
		planet->setRotation(core::vector3df(0,0,3));
		planetNode->setName("venus");

		SOInode = smgr->addEmptySceneNode();
		SOIMesh = smgr->addSphereMesh("SOI1",1.0,48,48);
		SOIMesh->setHardwareMappingHint(scene::EHM_STATIC);
		smgr->getMeshManipulator()->setVertexColors(SOIMesh,OrbitCol[i]);
		SOIscale = core::vector3df((pSOI[i]/scaleF),(pSOI[i]/scaleF),(pSOI[i]/scaleF));
		smgr->getMeshManipulator()->scale(SOIMesh,SOIscale);
		SOInode = smgr->addMeshSceneNode(SOIMesh);
		SOInode->setMaterialFlag(video::EMF_WIREFRAME, true);
		SOInode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		SOInode->setVisible(false);
		SOInode->setParent(planetNode);
		SOINodes.push_back(SOInode);

		planet->setParent(planetNode);
		planets.push_back(planetNode);
		planetsR.push_back(planet);
		tangentMesh->drop();
		break;
	case 2:
		planetNode = smgr->addEmptySceneNode();
		planetMesh = smgr->addSphereMesh("Earth",1.0,64,64);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(planetMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((planetSize[i]/scaleF),(planetSize[i]/scaleF),(planetSize[i]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		planet = smgr->addMeshSceneNode(tangentMesh);
		planet->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
		planet->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		planet->setMaterialTexture(0,tex5);
		planet->setMaterialTexture(1,tex6);
		planet->setRotation(core::vector3df(0,0,-23.439281));
		planetNode->setName("earth");

		SOInode = smgr->addEmptySceneNode();
		SOIMesh = smgr->addSphereMesh("SOI2",1.0,48,48);
		SOIMesh->setHardwareMappingHint(scene::EHM_STATIC);
		smgr->getMeshManipulator()->setVertexColors(SOIMesh,OrbitCol[i]);
		SOIscale = core::vector3df((pSOI[i]/scaleF),(pSOI[i]/scaleF),(pSOI[i]/scaleF));
		smgr->getMeshManipulator()->scale(SOIMesh,SOIscale);
		SOInode = smgr->addMeshSceneNode(SOIMesh);
		SOInode->setMaterialFlag(video::EMF_WIREFRAME, true);
		SOInode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		SOInode->setVisible(false);
		SOInode->setParent(planetNode);
		SOINodes.push_back(SOInode);

		planet->setParent(planetNode);
		planets.push_back(planetNode);
		planetsR.push_back(planet);
		tangentMesh->drop();

		atmoMesh = smgr->addSphereMesh("EarthAtmo",1.0,64,64);
		smgr->getMeshManipulator()->scale(atmoMesh,scale*1.0001);
		atmo = smgr->addMeshSceneNode(atmoMesh,planet);
		atmo->setScale(core::vector3df(1.005,1.005,1.005));
		atmo->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
		atmo->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
		atmo->setMaterialTexture(0,tex19);
		atmo->setName("earth-atmo");
		atmo->getMaterial(0).SpecularColor.set(255,255,255,255);
		atmo->getMaterial(0).Shininess = 200;
		//rota = smgr->createRotationAnimator(core::vector3df(0.0042,0.015,0.0042));
		//atmo->addAnimator(rota);
		//rota->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon0",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[0]/scaleF),(mSize[0]/scaleF),(mSize[0]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moon->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
		moonNode->setRotation(core::vector3df(0,0,5.14));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		//moon->setMaterialFlag(video::EMF_LIGHTING, false);
		moonpos(Day, &pos, &lambda);
		//date = SS->J2000ToGreg(Day);
		//printf("\n\nL A M B D A : %f , %f\n\n",lambda,Day);
		moon->setRotation(core::vector3df(0,360.0-lambda,-6.58));
		//moon->setScale(core::vector3df((mSize[0]/scaleF),(mSize[0]/scaleF),(mSize[0]/scaleF)));
		moon->setMaterialTexture(0,tex22);
		moon->setMaterialTexture(1,tex23);
		moon->setName("Moon");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		mOLine->setParent(planetNode);
		earthBill = smgr->addBillboardSceneNode(planetNode,core::dimension2df((planetSize[i]/scaleF)*2.5f,(planetSize[i]/scaleF)*2.5f));
		earthBill->setMaterialFlag(video::EMF_LIGHTING, false);
		earthBill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
		earthBill->setMaterialTexture(0,tex20);
		earthBill->setName("earth-bill");

		/*
		PlanetAxis = smgr->addEmptySceneNode();
		PlanetAxisM = smgr->getMesh("../data/axis.b3d");
		PlanetAxisM->setHardwareMappingHint(scene::EHM_STATIC);
		PlanetAxisM->setMaterialFlag(video::EMF_LIGHTING, false);
		PlanetAxisM->setMaterialFlag(video::EMF_WIREFRAME, true);
		PlanetAxisM->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		PlanetAxis = smgr->addMeshSceneNode(PlanetAxisM);
		PlanetAxis->setPosition(planetNode->getPosition());
		PlanetAxis->setParent(planet);
		PlanetAxis->setScale(core::vector3df(0.006f,0.006f,0.006f));
		
		PlanetAxis2 = smgr->addEmptySceneNode();
		PlanetAxisM2 = smgr->getMesh("../data/axis.b3d");
		PlanetAxisM2->setHardwareMappingHint(scene::EHM_STATIC);
		PlanetAxisM2->setMaterialFlag(video::EMF_LIGHTING, false);
		PlanetAxisM2->setMaterialFlag(video::EMF_WIREFRAME, true);
		PlanetAxisM2->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		PlanetAxis2 = smgr->addMeshSceneNode(PlanetAxisM2);
		PlanetAxis2->setPosition(planetNode->getPosition());
		PlanetAxis2->setParent(planetNode);
		PlanetAxis2->setScale(core::vector3df(0.006f,0.006f,0.006f));
		*/
		break;
	case 3:
		planetNode = smgr->addEmptySceneNode();
		planetMesh = smgr->addSphereMesh("Mars",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(planetMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((planetSize[i]/scaleF),(planetSize[i]/scaleF),(planetSize[i]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		planet = smgr->addMeshSceneNode(tangentMesh);
		planet->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
		planet->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		planet->setMaterialTexture(0,tex7);
		planet->setMaterialTexture(1,tex8);
		planet->setRotation(core::vector3df(0,0,-25));
		planetNode->setName("mars");

		SOInode = smgr->addEmptySceneNode();
		SOIMesh = smgr->addSphereMesh("SOI3",1.0,48,48);
		SOIMesh->setHardwareMappingHint(scene::EHM_STATIC);
		smgr->getMeshManipulator()->setVertexColors(SOIMesh,OrbitCol[i]);
		SOIscale = core::vector3df((pSOI[i]/scaleF),(pSOI[i]/scaleF),(pSOI[i]/scaleF));
		smgr->getMeshManipulator()->scale(SOIMesh,SOIscale);
		SOInode = smgr->addMeshSceneNode(SOIMesh);
		SOInode->setMaterialFlag(video::EMF_WIREFRAME, true);
		SOInode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		SOInode->setVisible(false);
		SOInode->setParent(planetNode);
		SOINodes.push_back(SOInode);

		planet->setParent(planetNode);
		planets.push_back(planetNode);
		planetsR.push_back(planet);
		tangentMesh->drop();
		break;
	case 4:
		planetNode = smgr->addEmptySceneNode();
		planetMesh = smgr->addSphereMesh("Jupiter",20.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(planetMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((planetSize[i]/scaleF/20.0),(planetSize[i]/scaleF/20.0),(planetSize[i]/scaleF/20.0));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		planet = smgr->addMeshSceneNode(tangentMesh);
		planet->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
		planet->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		planet->setMaterialTexture(0,tex9);
		planet->setMaterialTexture(1,tex10);
		planet->setRotation(core::vector3df(0,0,-3));
		planetNode->setName("jupiter");

		SOInode = smgr->addEmptySceneNode();
		SOIMesh = smgr->addSphereMesh("SOI4",1.0,48,48);
		SOIMesh->setHardwareMappingHint(scene::EHM_STATIC);
		smgr->getMeshManipulator()->setVertexColors(SOIMesh,OrbitCol[i]);
		SOIscale = core::vector3df((pSOI[i]/scaleF),(pSOI[i]/scaleF),(pSOI[i]/scaleF));
		smgr->getMeshManipulator()->scale(SOIMesh,SOIscale);
		SOInode = smgr->addMeshSceneNode(SOIMesh);
		SOInode->setMaterialFlag(video::EMF_WIREFRAME, true);
		SOInode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		SOInode->setVisible(false);
		SOInode->setParent(planetNode);
		SOINodes.push_back(SOInode);

		planet->setParent(planetNode);
		planets.push_back(planetNode);
		planetsR.push_back(planet);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon1",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[1]/scaleF),(mSize[1]/scaleF),(mSize[1]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[1]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex24);
		moon->setMaterialTexture(1,tex25);
		moon->setName("Io");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon2",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[2]/scaleF),(mSize[2]/scaleF),(mSize[2]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[2]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex26);
		moon->setMaterialTexture(1,tex27);
		moon->setName("Europa");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon3",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[3]/scaleF),(mSize[3]/scaleF),(mSize[3]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[3]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Ganymede");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon4",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[4]/scaleF),(mSize[4]/scaleF),(mSize[4]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[4]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex30);
		//moon->setMaterialTexture(1,tex27);
		moon->setName("Callisto");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();
		break;
	case 5:
		planetNode = smgr->addEmptySceneNode();
		planetMesh = smgr->addSphereMesh("Saturn",20.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(planetMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((planetSize[i]/scaleF/20.0),(planetSize[i]/scaleF/20.0),(planetSize[i]/scaleF/20.0));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		planet = smgr->addMeshSceneNode(tangentMesh);
		planet->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
		planet->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		planet->setMaterialTexture(0,tex11);
		planet->setMaterialTexture(1,tex10);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon5",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[5]/scaleF),(mSize[5]/scaleF),(mSize[5]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[5]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Mimas");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon6",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[6]/scaleF),(mSize[6]/scaleF),(mSize[6]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[6]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Enceladus");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon7",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[7]/scaleF),(mSize[7]/scaleF),(mSize[7]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[7]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Tethys");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon8",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[8]/scaleF),(mSize[8]/scaleF),(mSize[8]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[8]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Dione");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon9",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[9]/scaleF),(mSize[9]/scaleF),(mSize[9]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[9]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Rhea");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon10",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[10]/scaleF),(mSize[10]/scaleF),(mSize[10]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[10]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Titan");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon11",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[11]/scaleF),(mSize[11]/scaleF),(mSize[11]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[11]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Iapetus");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		ringMesh = smgr->getMesh("../data/ring.obj");
		ring = smgr->addMeshSceneNode(ringMesh,planetNode);
		ring->setParent(planet);
		ring->setScale(core::vector3df(1,0.1,1)*0.016);
		ring->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
		ring->setMaterialTexture(0,tex21);
		ring->setMaterialFlag(video::EMF_LIGHTING,false);
		ring->setName("saturn-ring");

		planet->setRotation(core::vector3df(0,0,-26.44));
		planetNode->setName("saturn");

		SOInode = smgr->addEmptySceneNode();
		SOIMesh = smgr->addSphereMesh("SOI5",1.0,48,48);
		SOIMesh->setHardwareMappingHint(scene::EHM_STATIC);
		smgr->getMeshManipulator()->setVertexColors(SOIMesh,OrbitCol[i]);
		SOIscale = core::vector3df((pSOI[i]/scaleF),(pSOI[i]/scaleF),(pSOI[i]/scaleF));
		smgr->getMeshManipulator()->scale(SOIMesh,SOIscale);
		SOInode = smgr->addMeshSceneNode(SOIMesh);
		SOInode->setMaterialFlag(video::EMF_WIREFRAME, true);
		SOInode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		SOInode->setVisible(false);
		SOInode->setParent(planetNode);
		SOINodes.push_back(SOInode);

		planet->setParent(planetNode);
		planets.push_back(planetNode);
		planetsR.push_back(planet);
		break;
	case 6:
		planetNode = smgr->addEmptySceneNode();
		planetMesh = smgr->addSphereMesh("Uranus",20.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(planetMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((planetSize[i]/scaleF/20.0),(planetSize[i]/scaleF/20.0),(planetSize[i]/scaleF/20.0));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		planet = smgr->addMeshSceneNode(tangentMesh);
		planet->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
		planet->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		planet->setMaterialTexture(0,tex13);
		planet->setMaterialTexture(1,tex10);
		planet->setRotation(core::vector3df(0,0,-98));
		planetNode->setName("uranus");

		SOInode = smgr->addEmptySceneNode();
		SOIMesh = smgr->addSphereMesh("SOI6",1.0,48,48);
		SOIMesh->setHardwareMappingHint(scene::EHM_STATIC);
		smgr->getMeshManipulator()->setVertexColors(SOIMesh,OrbitCol[i]);
		SOIscale = core::vector3df((pSOI[i]/scaleF),(pSOI[i]/scaleF),(pSOI[i]/scaleF));
		smgr->getMeshManipulator()->scale(SOIMesh,SOIscale);
		SOInode = smgr->addMeshSceneNode(SOIMesh);
		SOInode->setMaterialFlag(video::EMF_WIREFRAME, true);
		SOInode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		SOInode->setVisible(false);
		SOInode->setParent(planetNode);
		SOINodes.push_back(SOInode);

		planet->setParent(planetNode);
		planets.push_back(planetNode);
		planetsR.push_back(planet);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon12",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[12]/scaleF),(mSize[12]/scaleF),(mSize[12]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[12]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Puck");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon13",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[13]/scaleF),(mSize[13]/scaleF),(mSize[13]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[13]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Miranda");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon14",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[14]/scaleF),(mSize[14]/scaleF),(mSize[14]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[14]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Ariel");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon15",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[15]/scaleF),(mSize[15]/scaleF),(mSize[15]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[15]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Umbriel");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon16",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[16]/scaleF),(mSize[16]/scaleF),(mSize[16]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[16]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Titania");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();

		moonNode = smgr->addEmptySceneNode(planetNode);
		moonMesh = smgr->addSphereMesh("Moon17",1.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(moonMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((mSize[17]/scaleF),(mSize[17]/scaleF),(mSize[17]/scaleF));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		moon = smgr->addMeshSceneNode(tangentMesh,moonNode);
		moonNode->setPosition(core::vector3df(0,0,mDist[17]));
		moon->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		moon->setMaterialTexture(0,tex28);
		moon->setMaterialTexture(1,tex29);
		moon->setName("Oberon");
		moons.push_back(moonNode);
		moonsR.push_back(moon);
		tangentMesh->drop();
		break;
	case 7:
		planetNode = smgr->addEmptySceneNode();
		planetMesh = smgr->addSphereMesh("Neptune",20.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(planetMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((planetSize[i]/scaleF/20.0),(planetSize[i]/scaleF/20.0),(planetSize[i]/scaleF/20.0));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		planet = smgr->addMeshSceneNode(tangentMesh);
		planet->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
		planet->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		planet->setMaterialTexture(0,tex15);
		planet->setMaterialTexture(1,tex10);
		planet->setRotation(core::vector3df(0,0,-30));
		planetNode->setName("neptune");

		SOInode = smgr->addEmptySceneNode();
		SOIMesh = smgr->addSphereMesh("SOI7",1.0,48,48);
		SOIMesh->setHardwareMappingHint(scene::EHM_STATIC);
		smgr->getMeshManipulator()->setVertexColors(SOIMesh,OrbitCol[i]);
		SOIscale = core::vector3df((pSOI[i]/scaleF),(pSOI[i]/scaleF),(pSOI[i]/scaleF));
		smgr->getMeshManipulator()->scale(SOIMesh,SOIscale);
		SOInode = smgr->addMeshSceneNode(SOIMesh);
		SOInode->setMaterialFlag(video::EMF_WIREFRAME, true);
		SOInode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		SOInode->setVisible(false);
		SOInode->setParent(planetNode);
		SOINodes.push_back(SOInode);

		planet->setParent(planetNode);
		planets.push_back(planetNode);
		planetsR.push_back(planet);
		tangentMesh->drop();
		break;
	case 8:
		planetNode = smgr->addEmptySceneNode();
		planetMesh = smgr->addSphereMesh("Pluto",20.0,48,48);

		tangentMesh = smgr->getMeshManipulator()->createMeshWithTangents(planetMesh,false,true);
		tangentMesh->setHardwareMappingHint(scene::EHM_STATIC);
		scale = core::vector3df((planetSize[i]/scaleF/20.0),(planetSize[i]/scaleF/20.0),(planetSize[i]/scaleF/20.0));
		smgr->getMeshManipulator()->scale(tangentMesh,scale);

		planet = smgr->addMeshSceneNode(tangentMesh);
		planet->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
		planet->setMaterialType(video::EMT_NORMAL_MAP_SOLID);
		planet->setMaterialTexture(0,tex17);
		planet->setMaterialTexture(1,tex18);
		planet->setRotation(core::vector3df(0,0,-120));
		planetNode->setName("pluto");

		SOInode = smgr->addEmptySceneNode();
		SOIMesh = smgr->addSphereMesh("SOI8",1.0,48,48);
		SOIMesh->setHardwareMappingHint(scene::EHM_STATIC);
		smgr->getMeshManipulator()->setVertexColors(SOIMesh,OrbitCol[i]);
		SOIscale = core::vector3df((pSOI[i]/scaleF),(pSOI[i]/scaleF),(pSOI[i]/scaleF));
		smgr->getMeshManipulator()->scale(SOIMesh,SOIscale);
		SOInode = smgr->addMeshSceneNode(SOIMesh);
		SOInode->setMaterialFlag(video::EMF_WIREFRAME, true);
		SOInode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		SOInode->setVisible(false);
		SOInode->setParent(planetNode);
		SOINodes.push_back(SOInode);

		planet->setParent(planetNode);
		planets.push_back(planetNode);
		planetsR.push_back(planet);
		tangentMesh->drop();
		break;
		}
	}
	sunMesh = smgr->addSphereMesh("Sun",20.0,48,48);
	scene::ISceneNode* Sun = smgr->addMeshSceneNode(sunMesh);
	Sun->setScale(core::vector3df(planetSize[9]/scaleF/20.0,planetSize[9]/scaleF/20.0,planetSize[9])/scaleF/20.0);
	Sun->setMaterialFlag(video::EMF_LIGHTING, false);
	Sun->setMaterialType(video::EMT_LIGHTMAP);
	Sun->setMaterialTexture(0,sun);

	scene::IBillboardSceneNode* sunBill = smgr->addBillboardSceneNode(Sun,core::dimension2df(planetSize[9]/scaleF,planetSize[9]/scaleF)*15.5);
	sunBill->setMaterialFlag(video::EMF_LIGHTING, false);
	sunBill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	sunBill->setMaterialTexture(0,sunbill);

	planets.push_back(Sun);
}
void createSolarSystem()
{
	/*After quite a long excursion from this code I came back and decided it's time to comment
	a little more on what's going on.
	*/

	//stars1();
	stars2();

	//create stars and nebula with a skybox
	scene::ISceneNode* SkyBox = smgr->addSkyBoxSceneNode (
		driver->getTexture("../data/star_top.jpg"),
		driver->getTexture("../data/star_bot.jpg"),
		driver->getTexture("../data/star4.jpg"),
		driver->getTexture("../data/star2.jpg"),
		driver->getTexture("../data/star3.jpg"),
		driver->getTexture("../data/star1.jpg"));

	/*
	//create stars and nebula with a skybox
	scene::ISceneNode* SkyBox = smgr->addSkyBoxSceneNode (
	driver->getTexture("../data/gridFarbe.jpg"),
	driver->getTexture("../data/gridFarbe.jpg"),
	driver->getTexture("../data/gridFarbe.jpg"),
	driver->getTexture("../data/gridFarbe.jpg"),
	driver->getTexture("../data/gridFarbe.jpg"),
	driver->getTexture("../data/gridFarbe.jpg"));
	*/

	// make a 1024x1024 texture RTT skybox
	sky = new scene::CRTTSkyBoxSceneNode(core::dimension2d<u32>(1024,1024), smgr->getRootSceneNode(),smgr,0);

	// render stars and nebula to new skybox with default blue bg
	sky->renderToSkyBox(core::vector3df(0,0,0));

	// hide billboard stars and old Skybox
	allstars->setVisible(false);
	SkyBox->setVisible(false);

	/*Create a huge world axis for debug purposes
	axis = smgr->addEmptySceneNode();
	scene::IMesh* axisM = smgr->getMesh("../data/axis.b3d");
	axisM->setHardwareMappingHint(scene::EHM_STATIC);
	axis = smgr->addMeshSceneNode(axisM);
	axis->setMaterialFlag(video::EMF_LIGHTING, false);
	axis->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
	axis->setMaterialFlag(video::EMF_WIREFRAME, true);
	axis->setScale(core::vector3df(2,2,2));
	*/

	//Initialize orbitalElements
	offsetDouble[0] = 0.0;
	offsetDouble[1] = 0.0;
	offsetDouble[2] = 0.0;

	setSolarOffset(offsetDouble);

	SS->updateAtDay((f64)Day,-1);

	orbitsM = smgr->addEmptySceneNode();
	
	u32 x;
	u32 y;
	u32 j;

	for(u32 p=0; p<9; p++)
	{
		core::array<scene::ISceneNode*> pOLine = makeEllipse( (SS->OrbitalElements[p][0])/scaleF,
											  SS->OrbitalElements[p][1],
											  SS->OrbitalElements[p][2]*core::RADTODEG64,
											  SS->OrbitalElements[p][3]*core::RADTODEG64,
											  SS->OrbitalElements[p][4]*core::RADTODEG64,
											  OrbitCol[p],
											  3.0,
											  720);
		
		OrbitPlanes.push_back(pOLine);

	}

	departureMarker = createDebugAxis(core::vector3df(0.0015,0.0015,0.0015));
	departureMarker->setParent(orbitsM);
	arrivalMarker = createDebugAxis(core::vector3df(0.0015,0.0015,0.0015));
	arrivalMarker->setParent(orbitsM);
	
	createPlanets();

}

//===============================================================================================================================
//RUNTIME
//===============================================================================================================================

void calcEllipticalOrbVelocity(int type, int i, f64 Perigee, f64 Apogee)
{
	doubleRelativeVelocity[0] = 0;
	doubleRelativeVelocity[1] = 0;
	doubleRelativeVelocity[2] = 0;

	doubleRelativePosition[0] = 0;
	doubleRelativePosition[1] = 0;
	doubleRelativePosition[2] = 0;

	doubleVelocity[0] = 0;
	doubleVelocity[1] = 0;
	doubleVelocity[2] = 0;
	
	doublePosition[0] = 0;
	doublePosition[1] = 0;
	doublePosition[2] = 0;
	
	core::vector3df velocity;
	f64 distPerigee;
	f64 distApogee;
	f64 pMassS;

	if(type == 1){
		distPerigee = (mSize[i]+Perigee)*1000.0;
		distApogee = (mSize[i]+Apogee)*1000.0;
		pMassS = mMass[i];
	}

	if(type == 0){
		distPerigee = (planetSize[i]+Perigee)*1000.0;
		distApogee = (planetSize[i]+Apogee)*1000.0;
		pMassS = pMass[i];
	}

	f64 semiMajorAxisM = ((distPerigee+distApogee)/2);
	f64 pConst = Gconst*pMassS;

	//Calculate elliptical OrbitVelocity
	//Velocity at Perigee  	=      sqrt( GM * ( ( 2 * distApogee) / ( distPerigee * ( distPerigee+distApogee ) ) ) )
	f64 ellipticOrbVelocity = sqrt( pConst * ( ( 2 * distApogee) / ( distPerigee * ( distPerigee + distApogee ) ) ) );
	ellipticOrbVelocity /= 1000.0;

	//calculate orbital period in seconds, the formula for an ellipse is
	//p = 2PI*sqrt(semi-major axis^3/GM) where p is in seconds and semi-major axis in meters
	f64 oPeriod = (2*core::PI64*sqrt(pow(semiMajorAxisM,3)/pConst))/60.0;
	printf("Period in minutes: %f\n",oPeriod);
	f64 temp = pConst*pow(1440.0*60.0/(2.0*core::PI64),2);
	f64 altitude = (pow(temp,f64(1.0/3.0))/1000.0)-planetSize[i];

	//Equatorial Velocity Vector ----------------------------------------------------------------------
	/*
	core::vector3df planRot = core::vector3df(planetsR[i]->getRotation().X,0,planetsR[i]->getRotation().Z);
	velocity = planRot.rotationToDirection(core::vector3df(1,0,0));
	velocity.setLength(ellipticOrbVelocity);
	velocity *= -1;
	*/
	f64 planRot[3];
	planRot[0] = planetsR[i]->getRotation().X;
	planRot[1] = planetsR[i]->getRotation().Y; //+90.0;
	planRot[2] = planetsR[i]->getRotation().Z;

	f64 forward[3];
	forward[0] = 1.0;
	forward[1] = 0.0;
	forward[2] = 0.0;

	rotationToDirectionDouble(forward, planRot, doubleRelativeVelocity);
	setLengthDouble(ellipticOrbVelocity, doubleRelativeVelocity);

	doubleRelativeVelocity[0] *= -1.0;
	doubleRelativeVelocity[1] *= -1.0;
	doubleRelativeVelocity[2] *= -1.0;
	
	
	//Equatorial Position Vector ----------------------------------------------------------------------
	/*
	core::vector3df aboveGreenwich = planetsR[i]->getRotation().rotationToDirection();
	aboveGreenwich.setLength((planetSize[i]+startAlt)/scaleF);
	aboveGreenwich = core::vector3df(0,0,(planetSize[i]+startAlt)/scaleF);
	//determine startPos------------------------------------------------------------------------------
	startPos =    core::vector3df(SS->doublePositions[i][0]/scaleF,
								  SS->doublePositions[i][1]/scaleF,
								  SS->doublePositions[i][2]/scaleF)
				+ aboveGreenwich;
	*/


	forward[0] = 0.0;
	forward[1] = 0.0;
	forward[2] = 1.0;

	rotationToDirectionDouble(forward, planRot, doubleRelativePosition);
	setLengthDouble((planetSize[i]+startAlt), doubleRelativePosition);

	doublePosition[0] = doubleRelativePosition[0];
	doublePosition[1] = doubleRelativePosition[1];
	doublePosition[2] = doubleRelativePosition[2];

	makeAbsolutePosTo(i,doublePosition);

	startPos.X = doublePosition[0]/scaleF;
	startPos.Y = doublePosition[1]/scaleF;
	startPos.Z = doublePosition[2]/scaleF;

}
void calcLaunchSolution(u32 start, u32 destination)// f64 *launchday, core::vector3df *velocity
{
	// 1st.
	//calculate mean transfer orbit period
	f64 distPerigee = pMeanDist2[start]*1000.0f;
	f64 distApogee = pMeanDist2[destination]*1000.0f;

	f64 semiMajorAxisM = ((distPerigee+distApogee)/2);
	printf("\n\nSemi-major-axis: %.9f\n", semiMajorAxisM/1000.0);
	f64 pConst = Gconst*pMass[9];

	//calculate orbital period in days, for an ellipse that is tangential to start and destination planet
	//the formula for an ellipse is
	//p = 2PI*sqrt(semi-major axis^3/GM) where p is in seconds and semi-major axis in meters
	//then divide it by 2 to give us a transfer orbit period
	f64 TOPeriod = ((((2*core::PI64*sqrt(pow(semiMajorAxisM,3)/pConst))/60.0f)/60.0f)/24.0f);
	//printf("Period in days: %f\n",oPeriod);
	//Half of the elipse is the Transfer Orbit, therefore we divide by 2;
	TOPeriod /= 2.0f;
	printf("Transfer Period = %.9f\n",TOPeriod);

	//2nd
	//Find the 180 - angle that destination planet moves during the transfer period.
	//calculate 180 minus (360 * (transfer period / total orbit Period))
	//if the destination planets orbital period is greater than the start planets orbital period,
	//ie. it moves more slow, then the angle should be the headstart, otherwise it should be behind
	//by that angle
	f64 angleOffset = 180.0 - (360.0*(TOPeriod / orbitT[destination]));

	if(angleOffset > 360)
	{
		angleOffset -= 360;
	}

	printf("Angle Offset = %.9f\n",angleOffset);

	//3rd
	//find a day with this angular offset between start and destination planet
	bool windowNotFound = true;

	f64 testday = Day;
	f64 approxWindowDay;
	core::stringc date;

	u32 dates = 0;
	f64 startSMA0;
	f64 destiSMA0;

	while(dates < 1)
	{
		testday += 0.1f;
		date = SS->J2000ToGreg(testday).c_str();
		//core::array<core::vector3df> startPos =
		SS->updateAtDay(testday, start);
		startSMA0 = mag(SS->doublePositions[start]);
		//core::array<core::vector3df> DestPos =
		SS->updateAtDay(testday, destination);
		core::vector3df startAngle = double2V(SS->doublePositions[start]).getHorizontalAngle();
		core::vector3df DestAngle = double2V(SS->doublePositions[destination]).getHorizontalAngle();

		f32 delta = 0;

		if(orbitT[start] < orbitT[destination])
		{
			delta = startAngle.Y - DestAngle.Y;
		}
		else
		{
			delta = DestAngle.Y - startAngle.Y;
		}


		//printf("Delta:%f,- %s\n",delta,date);

		if( core::abs_(delta - angleOffset) <= 0.1)
		{
			approxWindowDay = testday;
			windowNotFound = false;
			dates++;
		}

	};
	//approxWindowDay = SS->gregToJ2000(2014,2,2,12,0,0);
	startDay = approxWindowDay;

	date = SS->J2000ToGreg(approxWindowDay).c_str();
	printf("Approximate Launch Date = %s\n",date);
	//4th
	//calculate a more precise transfer period based on determined launch date
	//core::array<core::vector3df> DestPos =
	SS->updateAtDay(approxWindowDay+TOPeriod, destination);
	destiSMA0 = mag(SS->doublePositions[destination]);
	f64 lDPerigee = startSMA0*1000.0;
	f64 lDApogee = destiSMA0*1000.0;
	f64 semiMajorAxisLD = (lDPerigee+lDApogee)/2.0;
	f64 lDTOPeriodEst = ((((2*core::PI64*sqrt(pow(semiMajorAxisLD,3)/pConst))/60.0)/60.0)/24.0)/2.0;

	//do that again to get a even better estimate
	SS->updateAtDay(approxWindowDay+lDTOPeriodEst, destination);
	destiSMA0 = mag(SS->doublePositions[destination]);
	lDPerigee = startSMA0*1000.0;
	lDApogee = destiSMA0*1000.0;
	semiMajorAxisLD = (lDPerigee+lDApogee)/2.0;
	lDTOPeriod = ((((2*core::PI64*sqrt(pow(semiMajorAxisLD,3)/pConst))/60.0)/60.0)/24.0)/2.0;
	
	printf("Approximate transfer period: %.9f\nold Estimate: %.9f\nSemi-major-Axis: %.9f\n",
			lDTOPeriod,
			lDTOPeriodEst,
			semiMajorAxisLD);
	
	//iterate -/+ 20 days to find best transfer

	f64 sma;
	f64 slr;
	f64 theta;
	
	f64 testV0[3];
	f64 testV1[3];
	
	f64 testDv0[3];
	f64 testDv1[3];

	f64 Dv0[3];
	f64 Dv1[3];

	f64 finalTheta;

	f64 doubleStartPos[3];
	f64 doubleStartVel[3];

	f64 doubleStartTestPos[3];
	f64 doubleStartTestVel[3];

	f64 doubleDestiPos[3];
	f64 doubleDestiVel[3];

	f64 doubleDestiTestPos[3];
	f64 doubleDestiTestVel[3];
	
	int iter;
	
	f64 totalDV = 9999999999.0;
	
	for(f64 startTestD = approxWindowDay - 2.; startTestD <= approxWindowDay + 2; startTestD += 1)
	{
		SS->updateAtDay(startTestD, start);
		
		doubleStartTestPos[0] = SS->doublePositions[start][0];
		doubleStartTestPos[1] = SS->doublePositions[start][1];
		doubleStartTestPos[2] = SS->doublePositions[start][2];

		doubleStartTestVel[0] = SS->doubleVelocities[start][0];
		doubleStartTestVel[1] = SS->doubleVelocities[start][1];
		doubleStartTestVel[2] = SS->doubleVelocities[start][2];
	
		for(f64 testD = lDTOPeriod - 60; testD <= lDTOPeriod + 30; testD += 0.1)
		{
		
			SS->updateAtDay(startTestD + testD - 0.005, destination);
			
			doubleDestiTestPos[0] = SS->doublePositions[destination][0];
			doubleDestiTestPos[1] = SS->doublePositions[destination][1];
			doubleDestiTestPos[2] = SS->doublePositions[destination][2];

			doubleDestiTestVel[0] = SS->doubleVelocities[destination][0];
			doubleDestiTestVel[1] = SS->doubleVelocities[destination][1];
			doubleDestiTestVel[2] = SS->doubleVelocities[destination][2];

			LambertI(doubleStartTestPos,
					 doubleDestiTestPos,
					 testD*86400.0,
					 mu,
					 0,
					 testV0,
					 testV1,
					 sma,
					 slr,
					 theta,
					 iter);
			
			
			testDv0[0] = testV0[0] - doubleStartTestVel[0];
			testDv0[1] = testV0[1] - doubleStartTestVel[1];
			testDv0[2] = testV0[2] - doubleStartTestVel[2];

			testDv1[0] = doubleDestiTestVel[0] - testV1[0];
			testDv1[1] = doubleDestiTestVel[1] - testV1[1];
			testDv1[2] = doubleDestiTestVel[2] - testV1[2];

			f64 currentTotDv = mag(testDv0) + mag(testDv1);
			
			printf("Test Total Dv = %f, Transfer Period = %f, Transfer Angle = %f\n",currentTotDv,testD,theta*core::RADTODEG64);

			if(currentTotDv < totalDV)
			{
				totalV0[0] = testV0[0];
				totalV0[1] = testV0[1];
				totalV0[2] = testV0[2];

				totalV1[0] = testV1[0];
				totalV1[1] = testV1[1];
				totalV1[2] = testV1[2];

				Dv0[0] = testDv0[0];
				Dv0[1] = testDv0[1];
				Dv0[2] = testDv0[2];

				Dv1[0] = testDv1[0];
				Dv1[1] = testDv1[1];
				Dv1[2] = testDv1[2];
				
				totalDV = currentTotDv;
				finalTheta = theta;
				
				doubleDestiPos[0] = doubleDestiTestPos[0];
				doubleDestiPos[1] = doubleDestiTestPos[1];
				doubleDestiPos[2] = doubleDestiTestPos[2];

				doubleDestiVel[0] = doubleDestiTestVel[0];
				doubleDestiVel[1] = doubleDestiTestVel[1];
				doubleDestiVel[2] = doubleDestiTestVel[2];

				doubleStartPos[0] = doubleStartTestPos[0];
				doubleStartPos[1] = doubleStartTestPos[1];
				doubleStartPos[2] = doubleStartTestPos[2];

				doubleStartVel[0] = doubleStartTestVel[0];
				doubleStartVel[1] = doubleStartTestVel[1];
				doubleStartVel[2] = doubleStartTestVel[2];
				
				approxWindowDay = startTestD;
				lDTOPeriod = testD;
			}

		}
	}
	
	departureMarker->setPosition(double2V(doubleStartPos)/scaleF);
	departureMarker->updateAbsolutePosition();

	arrivalMarker->setPosition(double2V(doubleDestiPos)/scaleF);
	arrivalMarker->updateAbsolutePosition();
	
	Day = approxWindowDay - 0.2;
	arrivalDay = approxWindowDay+lDTOPeriod;;

	date = SS->J2000ToGreg(approxWindowDay).c_str();
	printf("\nLaunch Date = %s\n",date);
	
	date = SS->J2000ToGreg(arrivalDay).c_str();
	printf("Arrival Date = %s\n",date);
	
	printf("Transfer Period: %f\n",lDTOPeriod);
	printf("Transfer Angle: %f\n",finalTheta*core::RADTODEG64);
	printf("delta V0 = %f\ndelta V1 = %f\n",mag(Dv0),mag(Dv1));
	printf("Total Dv = %f\n",totalDV);

	printf("TotalV0 X = %f Y = %f Z = %f\n",totalV0[0],totalV0[1],totalV0[2]);
	printf("EarthV0 X = %f Y = %f Z = %f\n\n",doubleStartVel[0],doubleStartVel[1],doubleStartVel[2]);

	f64 angle[3];
	getAngle(totalV0,angle);

	departureMarker->setRotation(double2V(angle));

	Day = approxWindowDay - 0.2;
	SS->updateAtDay(Day, -1);
}
void handlePlanets(f64 day)
{
    //update Planets--------------------------------------------------------------------------
	SS->updateAtDay(day,-1);
	//set their Positions
	u32 i;
	//printf("offset:%f\n",offset);
	for(i=0; i < 9; i++)
	{
		planets[i]->setPosition(core::vector3df(SS->doublePositions[i][0]/scaleF,
												SS->doublePositions[i][1]/scaleF,
												SS->doublePositions[i][2]/scaleF));
		planets[i]->updateAbsolutePosition();
	}

	//spin planets, spin moons around planets & spin moons itself
	//start with spinning planets as to say one planetary day...
	for(u32 i=0; i<9; i++)
	{
		//printf("%f\n",planetsR[i]->getRotation().Y);
		if(i!=2)
		{

			planetsR[i]->setRotation(planetsR[i]->getRotation() + core::vector3df(0,pRot[i],0));
		}
		else
		{
			f64 degSince12 = SS->deltaMidDay()*360.0;
			//printf("%f\n",degSince12);
			planetsR[i]->setRotation(core::vector3df(planetsR[i]->getRotation().X,degSince12-90.0,planetsR[i]->getRotation().Z));
		}
	}

	//spin moons around planets & spin moons (moonsR) itself.
	for(u32 i=0; i<moons.size(); i++)
	{
		if(i==0)
		{
			core::vector3df pos;
			f64 lambda;
			moonpos(day, &pos, &lambda);
			moons[i]->setPosition(pos/scaleF);
			//moonsR[i]->setRotation(core::vector3df(0,360.0-lambda,-6.58));
			moonsR[i]->setRotation(core::vector3df(0,moonsR[0]->getRotation().Y + mFOrb[i],-6.58));

		}
		else
		{
			core::vector3df pos = moons[i]->getPosition();
			rotateAroundCentre(pos,core::vector3df(0,0,0),core::vector3df(0,mFOrb[i],0));
			moons[i]->setPosition(pos);
			moonsR[i]->setRotation(moonsR[i]->getRotation() + core::vector3df(0,mFOrb[i],0));
		}
	}

}
void handleOrbitalLines()
{    
	core::array<scene::ISceneNode*> planes;

	for(u32 i=0; i<OrbitPlanes.size(); i++)
	{
		planes = OrbitPlanes[i];
		
		if(closestIndex == i && planesVisible)
		{
			planes[3]->setVisible(true);
		}
		if(closestIndex != i || !planesVisible)
		{
			planes[3]->setVisible(false);
		}
		
		planes[0]->setRotation(core::vector3df(SS->OrbitalElements[i][2]*core::RADTODEG64*-1.0,
											   SS->OrbitalElements[i][3]*core::RADTODEG64*-1.0,
											   0));
		planes[1]->setRotation(core::vector3df(0,
											  (SS->OrbitalElements[i][4])*core::RADTODEG64*-1.0,
											   0));
	}
	
}
void handleOrbitalPlanes()
{
	
	currentOrbitalPlane[0]->setRotation(core::vector3df(currentKeps[2]*core::RADTODEG64*-1,
														 currentKeps[3]*core::RADTODEG64*-1,
														 0));
	
	currentOrbitalPlane[1]->setRotation(core::vector3df(0,
														 currentKeps[4]*core::RADTODEG64*-1,
														 0));

	if(closestIndex != 9 && planesVisible)
	{
		currentOrbitalPlane[0]->setVisible(true);
		currentOrbitalPlane[0]->setParent(planets[closestIndex]);
		//currentOrbitalPlane[0]->setPosition(core::vector3df(0,0,0));

		if(currentKeps[0] > 0)
		{
			f64 semiMajor = currentKeps[0];
			f64 semiMinor = semiMajor*sqrt(1.0-pow(currentKeps[1],2));
			f64 foci = sqrt(pow(semiMajor,2)-pow(semiMinor,2));
			
			foci /= scaleF;
			//plane is not scale 1,1,1, but 200,200,200 therefore created with scale 0.005,0.005,0.005
			foci *= 200.;
			

			currentOrbitalPlane[2]->setScale(core::vector3df(semiMajor/scaleF,1,semiMinor/scaleF));
			currentOrbitalPlane[2]->setPosition(core::vector3df(-foci,0,0));
		}
		else
		{
			currentOrbitalPlane[2]->setScale(core::vector3df(pSOI[closestIndex]/scaleF,1,pSOI[closestIndex]/scaleF));
			currentOrbitalPlane[2]->setPosition(core::vector3df(0,0,0));
		}
	}
	else
	{
		currentOrbitalPlane[0]->setVisible(false);
	}
	
	
}
void handleZeroGCam()
{

	f64 camSpeed = 0;
	f64 camSpeed2 = 0;
	f64 camSpeedh = 0;
	f64 camSpeedv = 0;
	core::vector2df deltaC = CursorPos - core::vector2df(0.5,0.5);
	//cursor->setPosition(core::vector2df(0.5,0.5));
	deltaC /= 20;
	f64 deadzone = 0.002;
	f64 cX = 0;
	f64 cY = 0;
	if(deltaC.X > deadzone){
		cX = deadzone - deltaC.X;
	}if(deltaC.X < -deadzone){
		cX = -deadzone - deltaC.X;
	}if(deltaC.Y > deadzone){
		cY = deadzone - deltaC.Y;
	}if(deltaC.Y < -deadzone){
		cY = -deadzone - deltaC.Y;
	}if(deltaC.X < deadzone && deltaC.X > -deadzone){
		cX = 0.0;
	}if(deltaC.Y < deadzone && deltaC.Y > -deadzone){
		cY = 0.0;
	}
	deltaC = core::vector2df(cX*-1,cY*-1);
	//printf("%f , %f\n",cX,cY);

	if(keyQ)
		camSpeedr += 0.00035;
	if(keyE)
		camSpeedr -= 0.00035;

	if(!keyQ && !keyE)
	{
		if(camSpeedr > 0)
			camSpeedr -= 0.00005;
		if(camSpeedr < 0)
			camSpeedr += 0.00005;
		if(camSpeedr < 0.0001 && camSpeedr > -0.0001)
			camSpeedr = 0;
	}

	//Rotation--------------------------------------------------------------------------------------
	//Work out the 3 Axis of the Camera
	core::vector3df forwardD = (flyCam->getTarget() - flyCam->getAbsolutePosition()).normalize();
	core::vector3df upD = flyCam->getUpVector();
	core::vector3df rightD = forwardD.crossProduct(upD);

	//yaw Around the up axis
	rotateVectorAroundAxis(forwardD, upD, deltaC.X);

	// pitch around the right axis (we need to change both forward AND up)
	//Non Windows fullscreen cursor bug ?
	if(!win && param.Fullscreen == true)
	{
		rotateVectorAroundAxis(forwardD, rightD, deltaC.Y);
		rotateVectorAroundAxis(upD, rightD, deltaC.Y);
	}
	else
	{
		rotateVectorAroundAxis(forwardD, rightD, deltaC.Y*-1);
		rotateVectorAroundAxis(upD, rightD, deltaC.Y*-1);
	}

	// roll around the forward axis
	rotateVectorAroundAxis(upD, forwardD, camSpeedr);

	// And re-orient the flyCam to face along the foward and up axes.
	core::vector3df targetD = flyCam->getAbsolutePosition() + forwardD;
	flyCam->setTarget(targetD);
	flyCam->setUpVector(upD);

	//Thrusters---------------------------------------------------------------------------------------
	core::vector3df tangentVel = core::vector3df(orbVelocity.X,orbVelocity.Y,orbVelocity.Z).normalize();
	if(leftB && !key_shift){
		camSpeed += CspeedFB;
		thrusters += forwardD*camSpeed;
		//printf("%f\n",camSpeed);
	}if(rightB && !key_shift){
		camSpeed -= CspeedFB;
		thrusters += forwardD*camSpeed;
		//printf("%f\n",camSpeed);
	}if(leftB && key_shift){
		camSpeed2 += CspeedFB;
		thrusters += tangentVel*camSpeed2;
		//printf("%f\n",camSpeed);
	}if(rightB && key_shift){
		camSpeed2 -= CspeedFB;
		thrusters += tangentVel*camSpeed2;
		//printf("%f\n",camSpeed);
	}if(leftBool){
		camSpeedh += CspeedLR;
		thrusters += rightD*camSpeedh;
		//printf("%f\n",camSpeedh);
	}if(rightBool){
		camSpeedh -= CspeedLR;
		thrusters += rightD*camSpeedh;
		//printf("%f\n",camSpeedh);
	}if(up){
		camSpeedv += CspeedUD;
		thrusters += upD*camSpeedv;
		//printf("%f\n",camSpeedh);
	}if(down){
		camSpeedv -= CspeedUD;
		thrusters += upD*camSpeedv;
		//printf("%f\n",camSpeedh);
	}
	if(!leftB && !rightB && !leftBool && !rightBool && !down && !up)
	{
		thrusters = core::vector3df(0,0,0);
	}

}
void handleArrowNodes(core::vector3df camPosF, core::vector3df flyAtF, core::vector3df velocityF)
{
	//set force arrows direction & length-----------------------------------------------------

	core::vector3df toVel = velocityF.getHorizontalAngle();
	core::vector3df flyAtInverse = core::vector3df(flyAtF.X,flyAtF.Y,flyAtF.Z);
	core::vector3df toTan = flyAtInverse.getHorizontalAngle();

	velA->setPosition(camPosF);
	tanA->setPosition(camPosF);

	velA->setRotation(toVel);
	tanA->setRotation(toTan);

	f64 VelScale = core::min_(0.01, (velocityF.getLength()/100.0)*1000.0);
	velA->setScale(core::vector3df(1.0,1.0,1.0)*VelScale);
	tanA->setScale(core::vector3df(1.0,1.0,1.0)*0.001);

	flightmarker->setPosition( camPosF+
							 core::vector3df(flyAtF.X,
											 flyAtF.Y,
											 flyAtF.Z)/10.0);
	/*
	checkmesh->setPosition( camPosF+
							 core::vector3df(flyAtF.X,
											 flyAtF.Z,
											 flyAtF.Y)/20.0);
	*/
}
void handleStarCam(f32 RAh,f32 RAm,f32 RAs, f32 DEC, f32 DECm, f32 DECs) {

	f64 hourDeg = 360.0f/24.0f;
	f64 minuteDeg = 360.0f/1440.0f;
	f64 secDeg = 360.0f/86400.0f;
	f64 arcMin = 1.0f/60.0f;
	f64 arcSec = incrementF;

	f64 ra = (RAh*hourDeg) + (RAm*minuteDeg) + (RAs*secDeg);
	f64 dec;
	if(DEC>0)
		dec = (DEC) + (DECm/60.0f) + ((DECs/360.0f)*arcMin);
	else
		dec = (DEC) - (DECm/60.0f) - ((DECs/360.0f)*arcMin);
	f32 Yrot = (ra);
	f32 Xrot = (dec)*-1.0f;

	core::vector3df rota = core::vector3df(Xrot,Yrot,-23.5f);
	starCam->setRotation(rota);
	core::vector3df rotToDir = rota.rotationToDirection();
	flightmarker->setPosition(starCam->getAbsolutePosition()+ rotToDir*2.0f);
}
void handleMoonCam(int i)
{
	core::vector3df moonPos = moons[i]->getAbsolutePosition();
	core::vector3df planPos = planets[2]->getAbsolutePosition();
	core::vector3df planToMoon = moonPos-planPos;
	core::vector3df angle = planToMoon.getHorizontalAngle();
	//printf("%f,%f,%f\n",angle.X,angle.Y,angle.Z);
	moonCam->setPosition(double2V(SS->doublePositions[2])/scaleF);
	moonCam->setRotation(angle);
	//moonCam->setTarget(moonPos);
	
}
void handleMap()
{
	f32 moveSpeed = delta/2;
	//printf("MS%f\n",moveSpeed);
	// Work out the 3 axes for the flyCam.
	core::vector3df forwardDm = (splitSCam->getTarget() - splitSCam->getAbsolutePosition()).normalize();
	core::vector3df upDm = splitSCam->getUpVector();
	core::vector3df rightDm = forwardDm.crossProduct(upDm);

	// yaw around the up axis
	if(leftA && !key_shift)
		rotateVectorAroundAxis(forwardDm, upDm, -moveSpeed);
	else if(rightA && !key_shift)
		rotateVectorAroundAxis(forwardDm, upDm, +moveSpeed);

	// roll around the forward axis
	if(leftA && key_shift)
		rotateVectorAroundAxis(upDm, forwardDm, +moveSpeed);
	else if(rightA && key_shift)
		rotateVectorAroundAxis(upDm, forwardDm, -moveSpeed);

	// pitch around the rightBool axis (we need to change both forward AND up)
	if(upA && !key_shift)
	{
		rotateVectorAroundAxis(forwardDm, rightDm, -moveSpeed);
		rotateVectorAroundAxis(upDm, rightDm, -moveSpeed);
	}
	else if(downA && !key_shift)
	{
		rotateVectorAroundAxis(forwardDm, rightDm, +moveSpeed);
		rotateVectorAroundAxis(upDm, rightDm, +moveSpeed);
	}
	//Zoom in and out
	if(upA && key_shift)
		camOrbitDistance = core::max_(0.1f, camOrbitDistance - (camOrbitDistance * moveSpeed * 8.0f));
	else if(downA && key_shift)
		camOrbitDistance = core::min_(140000.f, camOrbitDistance + (camOrbitDistance * moveSpeed * 8.0f));


	for(u32 p=0; p<perihelions.size(); p++)
	{
		perihelions[p]->setSize(core::dimension2df(0.02f,0.02f)*camOrbitDistance);
	}
	for(u32 a=0; a<ascendingNodes.size(); a++)
	{
		ascendingNodes[a]->setSize(core::dimension2df(0.02f,0.02f)*camOrbitDistance);
	}


	if(!planningM)
	{
		// Move BACK up the forward axis of the flyCam to place it in its orbit.
		splitSCam->setPosition(camPos - (forwardDm * camOrbitDistance));

		// Point the flyCam at the target node, and align its up axis correctly.
		splitSCam->setTarget(camPos);
		splitSCam->setUpVector(upDm);

		if(planetsR[0]->getScale().X > 1.0f)
		{
			for(u32 p=0; p<planetsR.size()-1; p++)
			{
				planetsR[p]->setScale(core::vector3df(1,1,1));
			}
			for(u32 p=0; p<moonsR.size()-1; p++)
			{
				moonsR[p]->setScale(core::vector3df(1,1,1));;
			}
		}

		//initialCamOrbitDistance = camOrbitDistance + 0.5f;
	}
	if(planningM)
	{
		projectedPosition[0] = projectionPosition[timestepperPos][0];
		projectedPosition[1] = projectionPosition[timestepperPos][1];
		projectedPosition[2] = projectionPosition[timestepperPos][2];
		// Move BACK up the forward axis of the flyCam to place it in its orbit.
		splitSCam->setPosition(double2V(projectedPosition)/scaleF - (forwardDm * camOrbitDistance));
		// Point the flyCam at the target node, and align its up axis correctly.
		splitSCam->setTarget(double2V(projectedPosition)/scaleF);
		splitSCam->setUpVector(upDm);

		if(camOrbitDistance >= initialCamOrbitDistance)
		{
			for(u32 p=0; p<planetsR.size()-1; p++)
			{
				planetsR[p]->setScale(core::vector3df(1,1,1) * core::clamp(camOrbitDistance/initialCamOrbitDistance,1.0f,2000.0f));
			}
			for(u32 p=0; p<moonsR.size()-1; p++)
			{
				moonsR[p]->setScale(core::vector3df(1,1,1) * core::clamp(camOrbitDistance/initialCamOrbitDistance,1.0f,2000.0f));
			}
		}
	}

	for(u32 i=0; i<SOINodes.size(); i++)
	{
		SOINodes[i]->setVisible(showSOI);
	}

	//view 0 = main
	//view 1 = miniMap
	//view 2 = starCam
	//view 3 = moonCam
	if(keyF1)
	{
		view = 0;
	}
	if(keyF2)
	{
		view = 1;
	}
	if(keyF3)
	{
		view = 2;
	}
	if(keyM)
	{
		view = 3;
	}
	//printf("%d\n",view);

}
void handleSOI(f64 satPos[3])
{
	closestIndex = -1;
	closestDistance = 9999999999999999999999999.9;
	
	for(u32 i=0; i<9; i++)
	{
		//direction from flyCam to planet
		f64 sat2plan[3];
		sat2plan[0] = SS->doublePositions[i][0] - satPos[0];
		sat2plan[1] = SS->doublePositions[i][1] - satPos[1];
		sat2plan[2] = SS->doublePositions[i][2] - satPos[2];

		f64 pdistance = mag(sat2plan);

		//check
		if(pdistance < closestDistance)
		{
			closestDistance = pdistance;
			closestPDistance = pdistance;
			closestIndex = i;
			closestPlanet = i;
			toClosestBody = double2V(sat2plan);
			closestBody = planets[closestIndex];
			
		}
	}
	
	f64 satToSun[3];
	satToSun[0] = satPos[0] - SS->doublePositions[9][0];
	satToSun[1] = satPos[1] - SS->doublePositions[9][1];
	satToSun[2] = satPos[2] - SS->doublePositions[9][2];
	sunDistanceKM = mag(satToSun);

	if(closestDistance > pSOI[closestIndex])
	{
		//printf("must be the sun\n");
		//printf("%f > %f\n",closestDistance,pSOI[closestIndex]);
		toSun = double2V(satToSun);
		closestDistance = sunDistanceKM;
		toClosestBody = double2V(satToSun);
		closestIndex = 9;
		closestBody = planets[9];
	}
	
	//core::stringc planet = planetNames[closestIndex].c_str();
	//printf("closest Planet is %s            \n",planet);

	//Transfer Debug
	
	if(interplanetary)
	{
		if(closestIndex == 2)
		{
			toSun = double2V(satToSun);
			closestDistance = sunDistanceKM;
			toClosestBody = double2V(satToSun);
			closestIndex = 9;
			closestBody = planets[9];
			oldBody = planets[9];
			//printf("Should be Earth\n");
		}
		/*
		if(closestIndex == 3)
		{
			toSun = double2V(satToSun);
			closestDistance = sunDistanceKM;
			toClosestBody = double2V(satToSun);
			closestIndex = 9;
			closestBody = planets[9];
			oldBody = planets[9];
		}
		*/
	}

	if(closestBody != oldBody)
	{
		SOIchanged = true;
		
		if(!planningM)
		{
			core::stringc planet = planetNames[oldIndex].c_str();
			printf("----------------------------SOI CHANGED-------------------------------\n");
			printf("Was relative to %s            \n",planet);
			printf("%.9f,%.9f,%.9f\n",doubleRelativePosition[0],
                                      doubleRelativePosition[1],
                                      doubleRelativePosition[2]
			);
			printf("with velocity:\n");
			printf("%.9f,%.9f,%.9f = %.9f\n",doubleRelativeVelocity[0],
											 doubleRelativeVelocity[1],
											 doubleRelativeVelocity[2],
											 mag(doubleRelativeVelocity)
			);

			doubleRelativeVelocity[0] = doubleVelocity[0];
			doubleRelativeVelocity[1] = doubleVelocity[1];
			doubleRelativeVelocity[2] = doubleVelocity[2];

			doubleRelativePosition[0] = doublePosition[0];
			doubleRelativePosition[1] = doublePosition[1];
			doubleRelativePosition[2] = doublePosition[2];

			makeRelativePosTo(closestIndex,doubleRelativePosition);
			makeRelativeVelTo(closestIndex,doubleRelativeVelocity);
			
			planet = planetNames[closestIndex].c_str();
			printf("Is  relative to %s            \n",planet);
			printf("%.9f,%.9f,%.9f\n",doubleRelativePosition[0],
                                      doubleRelativePosition[1],
                                      doubleRelativePosition[2]
			);
			printf("with velocity:\n");
			printf("%.9f,%.9f,%.9f = %.9f\n\n",doubleRelativeVelocity[0],
											 doubleRelativeVelocity[1],
											 doubleRelativeVelocity[2],
											 mag(doubleRelativeVelocity)
			);

			

		}

		else
		{
			core::stringc planet = planetNames[oldIndex].c_str();
			printf("----------------------------SOI CHANGED-------------------------------\n");
			printf("Was relative to %s            \n",planet);
			printf("%.9f,%.9f,%.9f = %.9f\n",projectedRelativePosition[0],
                                            projectedRelativePosition[1],
                                            projectedRelativePosition[2],
                                            mag(projectedRelativePosition)
            );
			printf("with velocity:\n");
			printf("%.9f,%.9f,%.9f = %.9f\n",projectedRelativeVel[0],
                                            projectedRelativeVel[1],
                                            projectedRelativeVel[2],
                                            mag(projectedRelativeVel)
            );

			projectedRelativeVel[0] = projectedVelocity[0];
			projectedRelativeVel[1] = projectedVelocity[1];
			projectedRelativeVel[2] = projectedVelocity[2];

			projectedRelativePosition[0] = projectedPosition[0];
			projectedRelativePosition[1] = projectedPosition[1];
			projectedRelativePosition[2] = projectedPosition[2];

			makeRelativePosTo(closestIndex,projectedRelativePosition);
			makeRelativeVelTo(closestIndex,projectedRelativeVel);
			
			planet = planetNames[closestIndex].c_str();
			printf("Is  relative to %s        \n",planet);
			printf("%.9f,%.9f,%.9f = %.9f\n",projectedRelativePosition[0],
                                            projectedRelativePosition[1],
                                            projectedRelativePosition[2],
                                            mag(projectedRelativePosition)
            );
			printf("with velocity:\n");
			printf("%.9f,%.9f,%.9f = %.9f\n\n",projectedRelativeVel[0],
											projectedRelativeVel[1],
											projectedRelativeVel[2],
											mag(projectedRelativeVel)
			);

			
		}

	}
	oldBody = closestBody;
	oldIndex = closestIndex;
}
void handleTrail()
{
	
	
	if(SOIchanged)
	{
		trail.clear();
		SOIchanged = false;
		
		if(jumped)
		{
			absTrail.clear();
		}
	}

	core::vector3df relaPos = double2V(doubleRelativePosition)/scaleF;
	core::vector3df absoPos = ((double2V(offsetDouble))*-1.0) - double2V(doublePosition)/scaleF;

	scene::ISceneNode* current = smgr->addEmptySceneNode();
	current->setPosition(relaPos);

	trail.push_back(current);
	absTrail.push_back( absoPos );

	if(trail.size() > totalTrailSize)
	{
		scene::ISceneNode* remove = trail[0];
		trail.erase(0);
		absTrail.erase(0);
		remove->remove();
	}

	if(trailLine)
	{
		trailLine->remove();
		trailLine->drop();
	}
	if(trailLineAbs)
	{
		trailLineAbs->remove();
		trailLineAbs->drop();
	}


	trailLineAbs = makeTrailLine2(absTrail,3.5);
	trailLineAbs->setPosition(double2V(offsetDouble));
	

	if(closestIndex != 9)
	{
		trailLine = makeTrailLine(trail,3.5);
		trailLine->setPosition(closestBody->getAbsolutePosition());
		
	}
	else
	{
		trailLine = makeTrailLine2(absTrail,3.5);
		trailLine->setPosition(double2V(offsetDouble));
	}

	if(trailAbsolute)
	{
		trailLineAbs->setVisible(true);
		trailLine->setVisible(false);
	}
	else
	{
		trailLineAbs->setVisible(false);
		trailLine->setVisible(true);
	}
	if(trailAbsolute && view == 3)
	{
		trailLineAbs->setVisible(false);
		trailLine->setVisible(false);
	}


}
void handleIntro(u32 pintroseconds)
{
	//printf("%d\n",pintroseconds);
	//play Intro------------------------------------------------------------------------------
	if(introcount < intro.size()){
		if(pintroseconds >= 11500){
			introseconds = 0;
			info2->setText(intro[introcount].c_str());
			introcount++;
		}
	}else if(introcount >= intro.size()){
		if(pintroseconds >= 5000){
			introseconds = 0;
			//generates the random number by using the     system clock time
			srand(time(NULL));
			int Number = (rand() % (29));
			u32 o = 0;
			if(o < 10)
			{
				const wchar_t * text = info->getText();
				info2->setText(randomFacts[Number].c_str());
				//printf("%d \n",Number);
			}
		}
	}
}
void handlePlanetJump()
{
	if(jumpWait < 5 && jumped)
	{
		jumpWait += 0.1;
		//printf("jumpWait = %f\n",jumpWait);
	}
	else
	{
		jumped = false;
	}
	
	if(!jumped)
	{
		if(key1)
		{
			orbVelocity = core::vector3df(0,0,0);
			pull = core::vector3df(0,0,0);
			velocity = core::vector3df(0,0,0);
			closestIndex = 0;
			closestBody = planets[closestIndex];
			calcEllipticalOrbVelocity(0,closestIndex,startAlt,getAway);
			//Transform State Vectors from Relative to Absolute (Planet to Sun)-------------------
			doubleVelocity[0] = doubleRelativeVelocity[0];
			doubleVelocity[1] = doubleRelativeVelocity[1];
			doubleVelocity[2] = doubleRelativeVelocity[2];

			doublePosition[0] = doubleRelativePosition[0];
			doublePosition[1] = doubleRelativePosition[1];
			doublePosition[2] = doubleRelativePosition[2];

			makeAbsolutePosTo(closestIndex,doublePosition);
			makeAbsoluteVelTo(closestIndex,doubleVelocity);
			jumpWait = 0;
		}
		if(key2)
		{
			orbVelocity = core::vector3df(0,0,0);
			pull = core::vector3df(0,0,0);
			velocity = core::vector3df(0,0,0);
			closestIndex = 1;
			closestBody = planets[closestIndex];
			calcEllipticalOrbVelocity(0,closestIndex,startAlt,getAway);
			//Transform State Vectors from Relative to Absolute (Planet to Sun)-------------------
			doubleVelocity[0] = doubleRelativeVelocity[0];
			doubleVelocity[1] = doubleRelativeVelocity[1];
			doubleVelocity[2] = doubleRelativeVelocity[2];

			doublePosition[0] = doubleRelativePosition[0];
			doublePosition[1] = doubleRelativePosition[1];
			doublePosition[2] = doubleRelativePosition[2];

			makeAbsolutePosTo(closestIndex,doublePosition);
			makeAbsoluteVelTo(closestIndex,doubleVelocity);
			jumpWait = 0;
		}
		if(key3)
		{
			orbVelocity = core::vector3df(0,0,0);
			pull = core::vector3df(0,0,0);
			velocity = core::vector3df(0,0,0);
			closestIndex = 2;
			closestBody = planets[closestIndex];
			calcEllipticalOrbVelocity(0,closestIndex,startAlt,getAway);
			//Transform State Vectors from Relative to Absolute (Planet to Sun)-------------------
			doubleVelocity[0] = doubleRelativeVelocity[0];
			doubleVelocity[1] = doubleRelativeVelocity[1];
			doubleVelocity[2] = doubleRelativeVelocity[2];

			doublePosition[0] = doubleRelativePosition[0];
			doublePosition[1] = doubleRelativePosition[1];
			doublePosition[2] = doubleRelativePosition[2];

			makeAbsolutePosTo(closestIndex,doublePosition);
			makeAbsoluteVelTo(closestIndex,doubleVelocity);
			jumpWait = 0;
		}
		if(key4)
		{
			orbVelocity = core::vector3df(0,0,0);
			pull = core::vector3df(0,0,0);
			velocity = core::vector3df(0,0,0);
			closestIndex = 3;
			closestBody = planets[closestIndex];
			calcEllipticalOrbVelocity(0,closestIndex,startAlt,getAway);
			//Transform State Vectors from Relative to Absolute (Planet to Sun)-------------------
			doubleVelocity[0] = doubleRelativeVelocity[0];
			doubleVelocity[1] = doubleRelativeVelocity[1];
			doubleVelocity[2] = doubleRelativeVelocity[2];

			doublePosition[0] = doubleRelativePosition[0];
			doublePosition[1] = doubleRelativePosition[1];
			doublePosition[2] = doubleRelativePosition[2];

			makeAbsolutePosTo(closestIndex,doublePosition);
			makeAbsoluteVelTo(closestIndex,doubleVelocity);
			jumpWait = 0;
		}
		if(key5)
		{
			orbVelocity = core::vector3df(0,0,0);
			pull = core::vector3df(0,0,0);
			velocity = core::vector3df(0,0,0);
			closestIndex = 4;
			closestBody = planets[closestIndex];
			calcEllipticalOrbVelocity(0,closestIndex,startAlt,getAway);
			//Transform State Vectors from Relative to Absolute (Planet to Sun)-------------------
			doubleVelocity[0] = doubleRelativeVelocity[0];
			doubleVelocity[1] = doubleRelativeVelocity[1];
			doubleVelocity[2] = doubleRelativeVelocity[2];

			doublePosition[0] = doubleRelativePosition[0];
			doublePosition[1] = doubleRelativePosition[1];
			doublePosition[2] = doubleRelativePosition[2];

			makeAbsolutePosTo(closestIndex,doublePosition);
			makeAbsoluteVelTo(closestIndex,doubleVelocity);
			jumpWait = 0;
		}
		if(key6)
		{
			orbVelocity = core::vector3df(0,0,0);
			pull = core::vector3df(0,0,0);
			velocity = core::vector3df(0,0,0);
			closestIndex = 5;
			closestBody = planets[closestIndex];
			calcEllipticalOrbVelocity(0,closestIndex,startAlt,getAway);
			//Transform State Vectors from Relative to Absolute (Planet to Sun)-------------------
			doubleVelocity[0] = doubleRelativeVelocity[0];
			doubleVelocity[1] = doubleRelativeVelocity[1];
			doubleVelocity[2] = doubleRelativeVelocity[2];

			doublePosition[0] = doubleRelativePosition[0];
			doublePosition[1] = doubleRelativePosition[1];
			doublePosition[2] = doubleRelativePosition[2];

			makeAbsolutePosTo(closestIndex,doublePosition);
			makeAbsoluteVelTo(closestIndex,doubleVelocity);
			jumpWait = 0;
		}
		if(key7)
		{
			orbVelocity = core::vector3df(0,0,0);
			pull = core::vector3df(0,0,0);
			velocity = core::vector3df(0,0,0);
			closestIndex = 6;
			closestBody = planets[closestIndex];
			calcEllipticalOrbVelocity(0,closestIndex,startAlt,getAway);
			//Transform State Vectors from Relative to Absolute (Planet to Sun)-------------------
			doubleVelocity[0] = doubleRelativeVelocity[0];
			doubleVelocity[1] = doubleRelativeVelocity[1];
			doubleVelocity[2] = doubleRelativeVelocity[2];

			doublePosition[0] = doubleRelativePosition[0];
			doublePosition[1] = doubleRelativePosition[1];
			doublePosition[2] = doubleRelativePosition[2];

			makeAbsolutePosTo(closestIndex,doublePosition);
			makeAbsoluteVelTo(closestIndex,doubleVelocity);
			jumpWait = 0;
		}
		if(key8)
		{
			orbVelocity = core::vector3df(0,0,0);
			pull = core::vector3df(0,0,0);
			velocity = core::vector3df(0,0,0);
			closestIndex = 7;
			closestBody = planets[closestIndex];
			calcEllipticalOrbVelocity(0,closestIndex,startAlt,getAway);
			//Transform State Vectors from Relative to Absolute (Planet to Sun)-------------------
			doubleVelocity[0] = doubleRelativeVelocity[0];
			doubleVelocity[1] = doubleRelativeVelocity[1];
			doubleVelocity[2] = doubleRelativeVelocity[2];

			doublePosition[0] = doubleRelativePosition[0];
			doublePosition[1] = doubleRelativePosition[1];
			doublePosition[2] = doubleRelativePosition[2];

			makeAbsolutePosTo(closestIndex,doublePosition);
			makeAbsoluteVelTo(closestIndex,doubleVelocity);
			jumpWait = 0;
		}
		if(key9)
		{
			orbVelocity = core::vector3df(0,0,0);
			pull = core::vector3df(0,0,0);
			velocity = core::vector3df(0,0,0);
			closestIndex = 8;
			closestBody = planets[closestIndex];
			calcEllipticalOrbVelocity(0,closestIndex,startAlt,getAway);
			//Transform State Vectors from Relative to Absolute (Planet to Sun)-------------------
			doubleVelocity[0] = doubleRelativeVelocity[0];
			doubleVelocity[1] = doubleRelativeVelocity[1];
			doubleVelocity[2] = doubleRelativeVelocity[2];

			doublePosition[0] = doubleRelativePosition[0];
			doublePosition[1] = doubleRelativePosition[1];
			doublePosition[2] = doubleRelativePosition[2];

			makeAbsolutePosTo(closestIndex,doublePosition);
			makeAbsoluteVelTo(closestIndex,doubleVelocity);
			jumpWait = 0;
		}
		
		jumped = true;
		
	}
	
	if(key0)
	{
		updateFieldLines = true;
	}

}
void handleFieldLines()
{
	if(displayFieldLines)
	{
		if(FieldLineNode)
			FieldLineNode->setVisible(true);
	}
	else
	{
		if(FieldLineNode)
			FieldLineNode->setVisible(false);
	}

	if(updateFieldLines)
	{
		if(FieldLineNode)
		{
			FieldLineNode->removeAll();
		}

		updateFieldLines = false;
		core::array<scene::SMeshBuffer*> fieldLineMeshBufferA = SS->calculateFieldLines(SS->year,
																						SS->dayM,
																						SS->hour,
																						SS->minute,
																						SS->second);


		FieldLineNode = smgr->addEmptySceneNode();
		FieldLineNodeGSW = smgr->addEmptySceneNode(FieldLineNode);
		FieldLineNodeGSW->setRotation(core::vector3df(90,-90,0));
		f32 Re = planetSize[2];


		for(u32 l=0; l<fieldLineMeshBufferA.size(); l++)
		{
			scene::SMeshBuffer* currentline = fieldLineMeshBufferA[l];
			currentline->setHardwareMappingHint(scene::EHM_STATIC);
			CIndexedPrimitiveNode* lineNode = new CIndexedPrimitiveNode(FieldLineNodeGSW,
				smgr,-1,currentline,
				currentline->Indices.size()/2,
				scene::EPT_LINES);
			lineNode->setMaterialFlag(video::EMF_LIGHTING, false);
			lineNode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		}


	}
	if(FieldLineNode)
	{
		FieldLineNode->setPosition(planets[2]->getPosition());
		core::vector3df toSunV = planets[9]->getPosition() - planets[2]->getPosition();
		core::vector3df toSunA = toSunV.getHorizontalAngle();
		FieldLineNode->setRotation(toSunA);
	}
}
void project(f64 interval, u32 amount, core::array<f64> fdirection)
{
	printf("project\n");
	projected = true;

	printf("input Vel X:%f Y: %fZ:%f\n",fdirection[0],fdirection[1],fdirection[2]);
	printf("input Speed:%f\n\n",mag(fdirection));

	if(projectionPosition.size() == 0)
	{
		projectedPosition[0] = doublePosition[0];
		projectedPosition[1] = doublePosition[1];
		projectedPosition[2] = doublePosition[2];

		projectedVelocity[0] = doubleVelocity[0];
		projectedVelocity[1] = doubleVelocity[1];
		projectedVelocity[2] = doubleVelocity[2];

		projectedRelativeVel[0] = doubleRelativeVelocity[0];
		projectedRelativeVel[1] = doubleRelativeVelocity[1];
		projectedRelativeVel[2] = doubleRelativeVelocity[2];

		projectedRelativePosition[0] = doubleRelativePosition[0];
		projectedRelativePosition[1] = doubleRelativePosition[1];
		projectedRelativePosition[2] = doubleRelativePosition[2];

		projectionPosition.push_back(doubleToIrrArray(projectedPosition));
		projectionVelocity.push_back(doubleToIrrArray(projectedVelocity));
		projectionRelativeVel.push_back(doubleToIrrArray(projectedRelativeVel));
		projectionRelativePosition.push_back(doubleToIrrArray(projectedRelativePosition));
		
		projectedDay = Day;
		projectedDistance = closestDistance;
		
		projectionDay.push_back(Day);
		projectionDistance.push_back(projectedDistance);

		core::array<f64> kepsPush;
		kepsPush.push_back(currentKeps[0]);
		kepsPush.push_back(currentKeps[1]);
		kepsPush.push_back(currentKeps[2]);
		kepsPush.push_back(currentKeps[3]);
		kepsPush.push_back(currentKeps[4]);
		kepsPush.push_back(currentKeps[5]);

		projectionKeps.push_back(kepsPush);
		projectionClosestIndex.push_back(closestIndex);

	}
	else
	{
		projectedDay = projectionDay[projectionDay.size()-1];
		projectedDistance = projectionDistance[projectionDistance.size()-1];
		
		projectedKeps[0] = projectionKeps[projectionKeps.size()-1][0];
		projectedKeps[1] = projectionKeps[projectionKeps.size()-1][1];
		projectedKeps[2] = projectionKeps[projectionKeps.size()-1][2];
		projectedKeps[3] = projectionKeps[projectionKeps.size()-1][3];
		projectedKeps[4] = projectionKeps[projectionKeps.size()-1][4];
		projectedKeps[5] = projectionKeps[projectionKeps.size()-1][5];

		projectedPosition[0] = projectionPosition[projectionPosition.size()-1][0];
		projectedPosition[1] = projectionPosition[projectionPosition.size()-1][1];
		projectedPosition[2] = projectionPosition[projectionPosition.size()-1][2];

		projectedVelocity[0] = projectionVelocity[projectionVelocity.size()-1][0];
		projectedVelocity[1] = projectionVelocity[projectionVelocity.size()-1][1];
		projectedVelocity[2] = projectionVelocity[projectionVelocity.size()-1][2];

		projectedRelativeVel[0] = projectionRelativeVel[projectionRelativeVel.size()-1][0];
		projectedRelativeVel[1] = projectionRelativeVel[projectionRelativeVel.size()-1][1];
		projectedRelativeVel[2] = projectionRelativeVel[projectionRelativeVel.size()-1][2];

		projectedRelativePosition[0] = projectionRelativePosition[projectionRelativeVel.size()-1][0];
		projectedRelativePosition[1] = projectionRelativePosition[projectionRelativeVel.size()-1][1];
		projectedRelativePosition[2] = projectionRelativePosition[projectionRelativeVel.size()-1][2];

	}
	
	projectedRelativeVel[0] = fdirection[0];
	projectedRelativeVel[1] = fdirection[1];
	projectedRelativeVel[2] = fdirection[2];

	makeRelativeVelTo(closestIndex,projectedRelativeVel);
	
	printf("converted input Vel X:%f Y:%f Z:%f\n",fdirection[0],fdirection[1],fdirection[2]);
	printf("converted input Speed: %f\n\n",mag(fdirection));

	printf("Start Rel Pos X:%f Y:%f Z:%f\n",
		projectionRelativePosition[projectionRelativePosition.size()-1][0],
		projectionRelativePosition[projectionRelativePosition.size()-1][1],
		projectionRelativePosition[projectionRelativePosition.size()-1][2]);

	printf("Start Abs Pos X:%f Y:%f Z:%f\n\n",
		projectionPosition[projectionPosition.size()-1][0],
		projectionPosition[projectionPosition.size()-1][1],
		projectionPosition[projectionPosition.size()-1][2]);
	
	printf("Planet Pos X:%f Y:%f Z:%f\n\n",
		SS->doublePositions[closestIndex][0],
		SS->doublePositions[closestIndex][1],
		SS->doublePositions[closestIndex][2]);
	
	for(projectionCounter = 0; projectionCounter < amount-1; projectionCounter++)
	{
		//set time--------------------------------------------------------------------------------
		projectedDay += interval;
		projectionDay.push_back(projectedDay);
		
		//handlePlanets
		handlePlanets(projectedDay);
		
		irrlichtToHeliocentric(projectedRelativePosition);
		irrlichtToHeliocentric(projectedRelativeVel);

		propagateKEPInOut(projectedRelativePosition,projectedRelativeVel,interval*86400,GconstKM*pMass[closestIndex]);
		IC2par(projectedRelativePosition,projectedRelativeVel,GconstKM*pMass[closestIndex],projectedKeps);

		irrlichtToHeliocentric(projectedRelativePosition);
		irrlichtToHeliocentric(projectedRelativeVel);
		
		projectionRelativePosition.push_back(doubleToIrrArray(projectedRelativePosition));
		projectionRelativeVel.push_back(doubleToIrrArray(projectedRelativeVel));
		
		core::array<f64> kepsPush;
		kepsPush.push_back(projectedKeps[0]);
		kepsPush.push_back(projectedKeps[1]);
		kepsPush.push_back(projectedKeps[2]);
		kepsPush.push_back(projectedKeps[3]);
		kepsPush.push_back(projectedKeps[4]);
		kepsPush.push_back(projectedKeps[5]);
		
		projectionKeps.push_back(kepsPush);
		
		projectedPosition[0] = projectedRelativePosition[0];
		projectedPosition[1] = projectedRelativePosition[1];
		projectedPosition[2] = projectedRelativePosition[2];

		projectedVelocity[0] = projectedRelativeVel[0];
		projectedVelocity[1] = projectedRelativeVel[1];
		projectedVelocity[2] = projectedRelativeVel[2];

		makeAbsolutePosTo(closestIndex,projectedPosition);
		makeAbsoluteVelTo(closestIndex,projectedVelocity);

		projectionPosition.push_back(doubleToIrrArray(projectedPosition));
		projectionVelocity.push_back(doubleToIrrArray(projectedVelocity));
		
		projectionDistance.push_back(closestDistance);
		projectionClosestIndex.push_back(closestIndex);

		handleSOI(projectedPosition);
	
	}
	
	flightpathLine = makeLineDouble(projectionPosition,projectionVelocity,video::SColor(255,0,255,0),2.5);
	//flightpathLine->setParent(orbitsM);

	printf("End Rel Pos X:%f Y:%f Z:%f\n",
		projectionRelativePosition[projectionRelativePosition.size()-1][0],
		projectionRelativePosition[projectionRelativePosition.size()-1][1],
		projectionRelativePosition[projectionRelativePosition.size()-1][2]);

	printf("End Abs Pos X:%f Y:%f Z:%f\n\n",
		projectionPosition[projectionPosition.size()-1][0],
		projectionPosition[projectionPosition.size()-1][1],
		projectionPosition[projectionPosition.size()-1][2]);

}
void planning()
{
	//show planning GUI
	planningGUI->setVisible(true);

	//project
	if(!projected)
	{
		if(projectionPosition.size() > 0)
		{
			projectionPosition.clear();
			projectionVelocity.clear();

			projectionRelativePosition.clear();
			projectionRelativeVel.clear();

			projectionDay.clear();
			projectionKeps.clear();
			projectionDistance.clear();
			projectionClosestIndex.clear();
			
		}
		project(fpsincrement,1000,doubleToIrrArray(doubleVelocity));
		timestepper->setMax(1000-1);
		timestepper->setPos(0);
		timestepperPos = 0;
		
	}

	if(editB)
	{
		editB = false;
		timestepper->setEnabled(false);
		editbutton->setEnabled(false);

		burnbutton->setEnabled(true);
		burnUntil->setEnabled(true);

		if(timestepperPos+1 < projectionPosition.size())
		{
			projectionPosition.erase(timestepperPos+1,projectionPosition.size() - timestepperPos);
			projectionVelocity.erase(timestepperPos+1,projectionVelocity.size() - timestepperPos);

			projectionRelativePosition.erase(timestepperPos+1,projectionRelativePosition.size() - timestepperPos);
			projectionRelativeVel.erase(timestepperPos+1,projectionRelativeVel.size() - timestepperPos);
			
			projectionDay.erase(timestepperPos+1,projectionDay.size() - timestepperPos);
			projectionKeps.erase(timestepperPos+1,projectionKeps.size() - timestepperPos);
			projectionDistance.erase(timestepperPos+1,projectionDistance.size() - timestepperPos);
			projectionClosestIndex.erase(timestepperPos+1,projectionClosestIndex.size() - timestepperPos);

			if(flightpathLine)
			{
				flightpathLine->remove();
				flightpathLine = makeLineDouble(projectionPosition,projectionVelocity,video::SColor(255,0,255,0),2.5);
				//flightpathLine->setParent(orbitsM);
			}
		}
	}

	if(burnB)
	{
		burnB = false;

		if(!interplanetary)
		{
			const wchar_t *burnSpeed = burnUntil->getText();
			f64 burnS = _wtof(burnSpeed);
			f64 inputSpeed[3];
			inputSpeed[0] = projectionVelocity[projectionVelocity.size()-1][0];
			inputSpeed[1] = projectionVelocity[projectionVelocity.size()-1][1];
			inputSpeed[2] = projectionVelocity[projectionVelocity.size()-1][2];
			if(burnS != 0)
			{
				setLengthDouble(burnS,inputSpeed);
			}
			project(0.1,1000,doubleToIrrArray(inputSpeed));
		}
		else
		{
			projectedVelocity[0] = totalV0[0];
			projectedVelocity[1] = totalV0[1];
			projectedVelocity[2] = totalV0[2];
			
			f64 interval = lDTOPeriod/5500;
			project(interval,5500,doubleToIrrArray(projectedVelocity));
			
			projectedVelocity[0] = projectionVelocity[projectionVelocity.size()-1][0];
			projectedVelocity[1] = projectionVelocity[projectionVelocity.size()-1][1];
			projectedVelocity[2] = projectionVelocity[projectionVelocity.size()-1][2];

			interplanetary = false;
		}

		timestepper->setEnabled(true);
		editbutton->setEnabled(true);

		burnbutton->setEnabled(false);
		burnUntil->setEnabled(false);

		timestepper->setMax(projectionPosition.size()-1);
		timestepper->setPos(projectionPosition.size()-1);
		timestepperPos = projectionPosition.size()-1;
	}

	//set Time
	Day = projectionDay[timestepperPos];
	date = SS->J2000ToGreg(Day);

	speed = mag(projectionVelocity[timestepperPos]);
	oSpeed = mag(projectionRelativeVel[timestepperPos]);
	sunDistanceKM = mag(projectionPosition[timestepperPos]);
	closestDistance = projectionDistance[timestepperPos];
	closestIndex = projectionClosestIndex[timestepperPos];
	
	currentKeps[0] = projectionKeps[timestepperPos][0];
	currentKeps[1] = projectionKeps[timestepperPos][1];
	currentKeps[2] = projectionKeps[timestepperPos][2];
	currentKeps[3] = projectionKeps[timestepperPos][3];
	currentKeps[4] = projectionKeps[timestepperPos][4];
	currentKeps[5] = projectionKeps[timestepperPos][5];
	
	//handle planets
	handlePlanets(Day);
	
	//set flycam pos
	flyCam->setPosition(double2V(projectionPosition[timestepperPos])/scaleF);
	
	//set Arrow Nodes
	handleArrowNodes(
		double2V(projectionPosition[timestepperPos])/scaleF,
		double2V(projectionRelativeVel[timestepperPos]),
		double2V(projectionVelocity[timestepperPos])/scaleF
	);

	handleOrbitalPlanes();

	doublePosition[0] = projectionPosition[timestepperPos][0];
	doublePosition[1] = projectionPosition[timestepperPos][1];
	doublePosition[2] = projectionPosition[timestepperPos][2];

	doubleVelocity[0] = projectionVelocity[timestepperPos][0];
	doubleVelocity[1] = projectionVelocity[timestepperPos][1];
	doubleVelocity[2] = projectionVelocity[timestepperPos][2];

	doubleRelativeVelocity[0] = projectionRelativeVel[timestepperPos][0];
	doubleRelativeVelocity[1] = projectionRelativeVel[timestepperPos][1];
	doubleRelativeVelocity[2] = projectionRelativeVel[timestepperPos][2];

	doubleRelativePosition[0] = projectionRelativePosition[timestepperPos][0];
	doubleRelativePosition[1] = projectionRelativePosition[timestepperPos][1];
	doubleRelativePosition[2] = projectionRelativePosition[timestepperPos][2];

}
void showBackground(bool yes)
{
	sky->setVisible(yes);
	orbitsM->setVisible(yes);
	for(u32 i=0; i<planets.size(); i++)
	{
		planets[i]->setVisible(yes);
	}
}
void showArrows(bool yes)
{
	//forceA->setVisible(yes);
	velA->setVisible(yes);
	tanA->setVisible(yes);
}
void renderloop()
{
	flyCam->setNearValue(5);
	flyCam->setFarValue(100000);
	planets[closestIndex]->setVisible(false);
	smgr->drawAll();

	showBackground(false);

	driver->clearZBuffer();
	flyCam->setNearValue(0.001);
	flyCam->setFarValue((closestDistance/scaleF)*2.2);

	planets[closestIndex]->setVisible(true);
	smgr->drawAll();

	showBackground(true);

}
void setMainView()
{
	//view 0 = main
	//view 1 = miniMap
	//view 2 = starCam
	//view 3 = moonCam
	if(view == 0)
	{
		smgr->setActiveCamera(flyCam);
		showArrows(false);
		
	}
	if(view == 1)
	{
		smgr->setActiveCamera(splitSCam);
		showArrows(true);
		
	}
	if(view == 2)
	{
		smgr->setActiveCamera(starCam);
		//Sun in March 20
		//handleStarCam(0,0,0,0,0,0);
		//Sun in June 22
		//handleStarCam(6,1,54.9,23,26,20.1);
		//Sun in September 22
		//handleStarCam(12,0,0,0,0,0);
		//Sun in December 22
		//handleStarCam(18,0,0.5,-23,26,29.9);

		//orion Top

		//Kaus Australis
		//handleStarCam(18,24,10.4,-34,23,3.2);
		handleStarCam(5,32,0.4,0,-17,-56.1);
		showArrows(false);
	}
	if(view == 3)
	{
		smgr->setActiveCamera(moonCam);
		moons[0]->setScale(core::vector3df(1,1,1)*45);
		handleMoonCam(0);
		showArrows(false);
		orbitsM->setVisible(false);
		
		flightmarker->setVisible(false);
		flightaid->setVisible(false);
		planets[9]->setVisible(false);
	}
	else
	{
		moons[0]->setScale(core::vector3df(1,1,1));
		orbitsM->setVisible(true);
		
		flightmarker->setVisible(true);
		flightaid->setVisible(true);
		planets[9]->setVisible(true);
	}
}
void set2ndView()
{
	//view 0 = main
	//view 1 = miniMap
	//view 2 = starCam
	//view 3 = moonCam
	if(view == 0)
	{
		smgr->setActiveCamera(splitSCam);
		showArrows(true);
	}
	if(view == 1)
	{
		smgr->setActiveCamera(flyCam);
		showArrows(false);
	}
	if(view == 2)
	{
		smgr->setActiveCamera(splitSCam);
		handleStarCam(5,32,0.4,0,-17,-56.1);
		showArrows(false);
	}
	
}
core::stringw makeInfoString()
{
	//draw GUI--------------------------------------------------------------------------------
	core::stringw speedS;
	core::stringw oSpeedS;
	core::stringw currentOrbitDistanceS;
	core::stringw gforceS;
	core::stringw timescaleS;
	
	currentOrbitDistanceS = (core::stringw)(closestDistance);
	gforceS = (core::stringw)gforce;
	oSpeedS = (core::stringw)oSpeed;
	speedS = (core::stringw)speed;
	timescaleS = (core::stringw)timescale;
	
	core::stringw planet = planetNames[closestIndex].c_str();
			
	core::stringw elementsS = L"\n\n";
	elementsS += "Relative to ";
	elementsS += planet;
	elementsS += "\n";
	elementsS += "a:";
	elementsS += (core::stringw)currentKeps[0];
	elementsS += "\n";
	elementsS += "e:";
	elementsS += (core::stringw)currentKeps[1];
	elementsS += "\n";
	elementsS += "i:";
	elementsS += (core::stringw)(currentKeps[2]*core::RADTODEG64);
	elementsS += "\n";
	elementsS += "O:";
	elementsS +=(core::stringw)(currentKeps[3]*core::RADTODEG64);
	elementsS += "\n";
	elementsS += "w:";
	elementsS +=(core::stringw)(currentKeps[4]*core::RADTODEG64);
	elementsS += "\n";
	elementsS += "v:";
	elementsS +=(core::stringw)(currentKeps[5]*core::RADTODEG64);
	elementsS += "\n";
	
	core::stringw str = L"Distance to Sun: ";
	str += sunDistanceKM/AU;

	str += " AU\nOrbit Distance: ";
	str += currentOrbitDistanceS.subString(0,currentOrbitDistanceS.findFirst('.')+3);

	str += " KM\nCurrent G-force: ";
	str += gforceS.subString(0,gforceS.findFirst('.')+5);

	str += " m/s^2\nOrbital Velocity: ";
	str += oSpeedS.subString(0,oSpeedS.findFirst('.')+4);

	str += " KM/s\nTotal Velocity:  ";
	str += speedS.subString(0,speedS.findFirst('.')+4);
	str += " KM/s\n";

	str += date;

	str += "\nTimefactor:  ";
	str += (int)timefactor;
	str += "\nTimescale:  ";
	str += timescaleS;
	str += elementsS;
	return str;
}
int main()
{

	IrrlichtDevice *nulldevice = createDevice(video::EDT_NULL);

	IOSOperator *os = nulldevice->getOSOperator();
	core::stringc osS = (core::stringw)os->getOperationSystemVersion();

	core::dimension2d<u32> area;

	//windows
	if(osS[0] == 'M' && osS[1] == 'i' && osS[2] == 'c') //rosoft
	{
		win = true;
		//Doesn't work with multiples Screens on macs
		area = nulldevice->getVideoModeList()->getDesktopResolution();
	}
	//Macintosh
	else
	{
		video::IVideoModeList* deskres = nulldevice->getVideoModeList();
		s32 i = deskres->getVideoModeCount()-1;
		//Getting the last videoMode from the list works on my macbook pro
		//and my wife's macbook
		area = core::dimension2di(deskres->getVideoModeResolution(i).Width,
			deskres->getVideoModeResolution(i).Height);
	}

	//drop the nulldevice
	nulldevice -> drop();

	param.DriverType = video::EDT_OPENGL;
	param.Vsync = false;
	param.AntiAlias = true;
	param.WindowSize = area;
	param.Bits = 16;
	param.WindowSize = core::dimension2d<u32>(1600,1200);
	param.Fullscreen = true;

	//create pointers to device, scene manager, GUI env., sound engine, cursor--------------------------------
	device=createDeviceEx(param);

	device->setEventReceiver(&my_event_receiver);
	driver=device->getVideoDriver();
	smgr=device->getSceneManager();
	device->getTimer()->setTime( 0 );
	env = device->getGUIEnvironment();
	cursor = device->getCursorControl();

	driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);

	Width = driver->getScreenSize().Width;
	Height = driver->getScreenSize().Height;
	cursor->setPosition(s32(Width/2),s32(Height/2));

	//make GUI
	makeGUI();
	
	//createSegUnitCubeMeshNew(10);

	//Initialize variables-----------------------------------------------------------------------------
	readconfig();
	initVars();
	Day = SS->gregToJ2000(Syear,Smonth,Sday,Shour,Sminute,Ssecond);

	//we need days (in the julian calender) as a unit for astronomical calculations... so...
	//how many fractional days pass in one frame (can be a very small number)--------------------------
	//24*60*60 seconds = 86400 seconds = 1 day;
	//Day += (1/86400)*timescale; <-- is to small for f64
	fpsincrement = incrementF*timescale;

	//add light----------------------------------------------------------------------------------------
	light = smgr->addLightSceneNode(0,
		core::vector3df(0,0,0),
		video::SColorf(1.0f,1.0f,1.0f),1000.0f);

	video::SLight ldata = light->getLightData();
	//ldata.Attenuation.set(0.0f,1.0f,0.0f);
	ldata.AmbientColor.set(0.156f,0.156f,0.156f);
	light->setLightData(ldata);

	//create solar system------------------------------------------------------------------------------
	createSolarSystem();
	//plotCassini();
	
	currentOrbitalPlane = makeOrbitalPlane();

	if(!interplanetary)
	{
		//set inital planet position------------------------------------------------------------------------
		SS->updateAtDay(Day,-1);
		//set Position
		u32 j;
		for(j=0; j < 9; j++)
		{
			planets[j]->setPosition(double2V(SS->doublePositions[j])/scaleF);
			planets[j]->updateAbsolutePosition();
		}

		closestIndex = startplanet;
		oldIndex = startplanet;
		closestBody = planets[startplanet];
		oldBody = closestBody;

		resetWorld();

		//calculate orbital State Vector------------------------------------------------------------
		calcEllipticalOrbVelocity(0,startplanet,startAlt,getAway);
		
		//Transform State Vectors from Relative to Absolute (Planet to Sun)-------------------
		doubleVelocity[0] = doubleRelativeVelocity[0];
		doubleVelocity[1] = doubleRelativeVelocity[1];
		doubleVelocity[2] = doubleRelativeVelocity[2];

		doublePosition[0] = doubleRelativePosition[0];
		doublePosition[1] = doubleRelativePosition[1];
		doublePosition[2] = doubleRelativePosition[2];

		makeAbsolutePosTo(closestIndex,doublePosition);
		makeAbsoluteVelTo(closestIndex,doubleVelocity);
	
	}
	else
	{
		//Transfer debug
		closestIndex = 9;
		oldIndex = 9;
		closestBody = planets[9];
		oldBody = closestBody;

		calcLaunchSolution(2,3);
		doublePosition[0] = SS->doublePositions[2][0];
		doublePosition[1] = SS->doublePositions[2][1];
		doublePosition[2] = SS->doublePositions[2][2];

		doubleRelativePosition[0] = SS->doublePositions[2][0];
		doubleRelativePosition[1] = SS->doublePositions[2][1];
		doubleRelativePosition[2] = SS->doublePositions[2][2];

		doubleVelocity[0] = SS->doubleVelocities[2][0];
		doubleVelocity[1] = SS->doubleVelocities[2][1];
		doubleVelocity[2] = SS->doubleVelocities[2][2];

		doubleRelativeVelocity[0] = SS->doubleVelocities[2][0];
		doubleRelativeVelocity[1] = SS->doubleVelocities[2][1];
		doubleRelativeVelocity[2] = SS->doubleVelocities[2][2];

		//Set the irrlicht Vectors
		velocity.X = doubleVelocity[0]/scaleF;
		velocity.Y = doubleVelocity[1]/scaleF;
		velocity.Z = doubleVelocity[2]/scaleF;

		startPos.X = doublePosition[0]/scaleF;
		startPos.Y = doublePosition[1]/scaleF;
		startPos.Z = doublePosition[2]/scaleF;
	}
	
	//test Date
	printf("___\nJulian %f\n___",SS->gregToJ(2011,5,3,0,0,0));
	printf("___\nMJD200 %f\n___",SS->gregToJ2000(2011,5,3,0,0,0));
	
	//make force arrows--------------------------------------------------------------------------------
	//arrow mesh points along Y axis originaly, we have to change that
	core::matrix4 matr;
	matr.setRotationDegrees(core::vector3df(90,0,0));
	/*
	scene::IAnimatedMesh* forceAm = smgr->addArrowMesh("forceAm",
		video::SColor(0,255,0,0),
		video::SColor(0,255,0,0),4,8,
		20,15, 0.5f,1);
	smgr->getMeshManipulator()->transformMesh(forceAm, matr);
	forceA = smgr->addMeshSceneNode(forceAm);
	forceA->setMaterialFlag(video::EMF_LIGHTING,false);
	forceA->setVisible(false);
	*/
	scene::IAnimatedMesh* velAm = smgr->addArrowMesh("velAm",
		video::SColor(0,0,255,0),
		video::SColor(0,0,255,0),4,8,
		20,15, 0.5f,1);
	smgr->getMeshManipulator()->transformMesh(velAm, matr);
	velA = smgr->addMeshSceneNode(velAm);
	velA->setMaterialFlag(video::EMF_LIGHTING,false);
	velA->setVisible(false);

	scene::IAnimatedMesh* tanAm = smgr->addArrowMesh("tanAm",
		video::SColor(0,0,0,255),
		video::SColor(0,0,0,255),4,8,
		20,15, 0.5f,1);
	smgr->getMeshManipulator()->transformMesh(tanAm, matr);
	tanA = smgr->addMeshSceneNode(tanAm);
	tanA->setMaterialFlag(video::EMF_LIGHTING,false);
	tanA->setVisible(false);

	//texture for the trail
	bluedot = driver->getTexture("../data/bluedot.jpg");
	greendot = driver->getTexture("../data/greendot.jpg");

	//make and position cameras------------------------------------------------------------------------
	//flyCam=smgr->addCameraSceneNodeFPS(0,100,0);
	flyCam=smgr->addCameraSceneNode();
	flyCam->setNearValue(0.01);
	flyCam->setFarValue(10000);
	flyCam->setPosition(startPos);
	flyCam->setFOV(1.1);
	flyCam->setTarget(planets[2]->getPosition());

	mayaCam=smgr->addCameraSceneNodeMaya(0,500,100,500);
	mayaCam->setNearValue(0.001);
	mayaCam->setFarValue(10000);
	mayaCam->setFOV(1.1);
	mayaCam->setPosition(planetsR[startplanet]->getAbsolutePosition()+core::vector3df(0,0,0.5));
	mayaCam->setTarget(planets[2]->getAbsolutePosition());

	starCam=smgr->addCameraSceneNode();
	starCam->bindTargetAndRotation(true);
	starCam->setNearValue(0.01);
	starCam->setFarValue(10000);
	starCam->setFOV(1.3);
	starCam->setParent(planets[2]);

	splitSCam=smgr->addCameraSceneNode();
	splitSCam->setNearValue(0.1);
	splitSCam->setFarValue(1000000);
	splitSCam->setTarget(startPos);
	splitSCam->setPosition(startPos + core::vector3df(0,10,0));

	moonCam=smgr->addCameraSceneNode();
	moonCam->setNearValue(0.1);
	moonCam->setFarValue(1000000);
	moonCam->setFOV(1.0);
	moonCam->bindTargetAndRotation(true);

	camOrbitDistance = startPos.getDistanceFrom(splitSCam->getPosition());
	initialCamOrbitDistance = camOrbitDistance + 0.5;

	//initialize some more variables for loop------------------------------------------------------
	u32 oldseconds = 0;
	u32 trailseconds = 0;
	smgr->setActiveCamera(flyCam);
	
	/*READ HORIZON QUERY FILE
	AstroFileReader *reader = new AstroFileReader;
	HorizonFile smallBodies;
	
	reader->ReadHorizonQuery(&smallBodies);
	*/
	
	vector<double> CASSINI1sol;
	vector<double> rp;
    double dummy7[] = {-789.8117,158.302027105278,449.385873819743,54.7489684339665,1024.36205846918,4552.30796805542};
    array2vector(dummy7,CASSINI1sol,6);
    double CASSINI1obj = cassini1(CASSINI1sol,rp);
    cout << "CASSINI1 problem: " << endl;
    cout << "ObjFun: " << CASSINI1obj << endl << endl;
    //cout << "ObjFun: " << 2000000 - GTOC1obj << endl;
    for (int i=0;i<4;++i){
            cout << "rp[" << i << "]: " << rp[i] << endl;
    }
    cout << endl;
	
    //--------------------------------------------------------------------------------------------
	//loop_loop_loop_loop_loop_loop_loop_loop_loop_loop_loop_loop_loop_loop_loop_loop_loop_loop___
	//--------------------------------------------------------------------------------------------
	
	while(device->run())
	{
		while(currentCount < timefactor)
		{
			//begin-----------------------------------------------------------------------------------
			driver->setViewPort(core::rect<s32>(0,0,Width,Height));
			driver->beginScene(true,true,video::SColor(0,0,0,0));
			setMainView();

			if(planningM)
			{

				planning();

			}

			if(!planningM)
			{
				if(updateVar)
				{
					fpsincrement = incrementF * timescale;
					updateVar = false;
					updateVars();
				}

				//set time--------------------------------------------------------------------------------
				Day += fpsincrement;
				
				//handlePlanets---------------------------------------------------------------------------
				handlePlanets(Day);
				handleOrbitalLines();

				handleFieldLines();
				
				//resetTheCoordinateSystem;
				resetWorld();

				//handle Thruster Input-------------------------------------------------------------------
				handleZeroGCam();

				//Add Thruster Input----------------------------------------------------------------------
				doubleRelativeVelocity[0] += thrusters.X * timescale * scaleF;
				doubleRelativeVelocity[1] += thrusters.Y * timescale * scaleF;
				doubleRelativeVelocity[2] += thrusters.Z * timescale * scaleF;
				
				irrlichtToHeliocentric(doubleRelativeVelocity);
				irrlichtToHeliocentric(doubleRelativePosition);

				//Determine New State Vectors relative to Planet (Sphere of Influence)--------------------
				propagateKEPInOut(doubleRelativePosition,doubleRelativeVelocity,fpsincrement*86400,GconstKM*pMass[closestIndex]);
				//kepler(doubleRelativePosition,doubleRelativeVelocity,fpsincrement*86400,GconstKM*pMass[closestIndex]);
				
				IC2par(doubleRelativePosition,doubleRelativeVelocity,GconstKM*pMass[closestIndex],currentKeps);
				
				irrlichtToHeliocentric(doubleRelativeVelocity);
				irrlichtToHeliocentric(doubleRelativePosition);

				//printf("MAG = %f X = %f Y = %f Z = %f\n",mag(doubleRelativeVelocity),doubleRelativeVelocity[0],doubleRelativeVelocity[1],doubleRelativeVelocity[2]);

				/*
				printf("\n\na = %f\ne = %f\ni = %f\nO = %f\nw = %f\nv = %f",
						currentKeps[0],
						currentKeps[1],
						currentKeps[2],
						currentKeps[3],
						currentKeps[4],
						currentKeps[5]);
				*/
				//Determine Orbital velocity--------------------------------------------------------------
				orbVelocity = core::vector3df(doubleRelativeVelocity[0],
											  doubleRelativeVelocity[1],
											  doubleRelativeVelocity[2]);
				oSpeed = mag(doubleRelativeVelocity);

				//Transform State Vectors from Relative to Absolute (Planet to Sun)-------------------
				doubleVelocity[0] = doubleRelativeVelocity[0];
				doubleVelocity[1] = doubleRelativeVelocity[1];
				doubleVelocity[2] = doubleRelativeVelocity[2];

				doublePosition[0] = doubleRelativePosition[0];
				doublePosition[1] = doubleRelativePosition[1];
				doublePosition[2] = doubleRelativePosition[2];

				makeAbsolutePosTo(closestIndex,doublePosition);
				makeAbsoluteVelTo(closestIndex,doubleVelocity);

				camPos = core::vector3df(doublePosition[0]/scaleF,doublePosition[1]/scaleF,doublePosition[2]/scaleF);
				velocity = core::vector3df(doubleVelocity[0]/scaleF,doubleVelocity[1]/scaleF,doubleVelocity[2]/scaleF);

				//set cameras position--------------------------------------------------------------------
				flyCam->setPosition(camPos);

				//set Arrow Nodes-------------------------------------------------------------------------
				handleArrowNodes(camPos,orbVelocity,velocity);

				//setLightRadius--------------------------------------------------------------------------
				light->setRadius((sunDistanceKM/scaleF)*1.1);
				//flyCam->setFarValue(sunDistanceKM/1000.0f);
				
				//play Intro------------------------------------------------------------------------------
				handleIntro(introseconds);

				//handle trail----------------------------------------------------------------------------
				handleTrail();

				//determine speed-------------------------------------------------------------------------
				speed = mag(doubleVelocity);

				//determine date--------------------------------------------------------------------------
				date = SS->J2000ToGreg(Day);

				//determine Sphere of Influence------------------------------------------------------------
				handleSOI(doublePosition);

				handlePlanetJump();

				handleOrbitalPlanes();

			}

			currentCount++;
		}

		currentCount = 0;

		//determine frame delta in seconds----------------------------------------------------------------
		u32 mseconds = (device->getTimer()->getTime());
		delta = mseconds-oldseconds;
		oldseconds = mseconds;
		introseconds += delta;
		trailseconds += delta;
		delta /= 1000;
		//printf("%f\n",1/delta);

		//handle map------------------------------------------------------------------------------
		handleMap();

		//draw------------------------------------------------------------------------------------
		renderloop();

		//draw GUI--------------------------------------------------------------------------------
		core::stringw str = makeInfoString();
		env->getSkin()->setFont(fnt);
		info->setText(str.c_str());
		env->drawAll();

		//handle splitscreen---------------------------------------------------------------------
		driver->setViewPort(core::rect<s32>(Width*0.76,Height*0.76,Width,Height));
		set2ndView();
		renderloop();

		//end-------------------------------------------------------------------------------------
		driver->endScene();

		if(key_ctrl && keyQ)
		{
			device->closeDevice();
		}

	}
	device->drop();
	return 0;
}
