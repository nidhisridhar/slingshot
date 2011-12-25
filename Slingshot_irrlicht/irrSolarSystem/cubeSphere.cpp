#include <irrlicht.h>

using namespace irr;

/*
transform a cube patch to sphere patch 

void Cube2Sphere(mesh%) 
{ 
   Local s%,surf%,v% 
   Local vx#,vy#,vz# 
    
   For s=1 To CountSurfaces(mesh) 
       
      surf=GetSurface(mesh,s) 
       
      For v=0 To CountVertices(surf)-1 
          
         vx=VertexX(surf,v) 
         vy=VertexY(surf,v) 
         vz=VertexZ(surf,v) 
          
         VertexCoords surf,v,SphericalX(vx,vy,vz)*1.333333,SphericalY(vx,vy,vz)*1.333333,SphericalZ(vx,vy,vz)*1.333333 
          
      Next 
       
   Next 
    
 
}
*/


/*
------------------------------------------------------------------------------------------
calculate spherical X 
*/
f32 SphericalX(f32 x, f32 y, f32 z)
{    
   return x*sqrt(1.0-y*y*0.5-z*z*0.5+y*y*z*z*1.0/3);
}

/*
------------------------------------------------------------------------------------------
calculate spherical Y 
*/
f32 SphericalY(f32 x, f32 y, f32 z)
{
   return y*sqrt(1.0-z*z*0.5-x*x*0.5+z*z*x*x*1.0/3); 
}

/*
------------------------------------------------------------------------------------------
calculate spherical Z 
*/
f32 SphericalZ(f32 x, f32 y, f32 z) 
{ 
   return z*sqrt(1.0-x*x*0.5-y*y*0.5+x*x*y*y*1.0/3); 
}

/*
; creates a segmented cube 
Function CreateSegCube(segs=1,parent=0) 
    
   Local side%,surf% 
   Local stx#,sty#,stp#,y# 
   Local a%,x#,v#,b%,u# 
   Local v0%,v1%,v2%,v3% 
   Local mesh%=CreateMesh(parent) 
    
   ; sides 
   For side=0 To 3 
       
      surf=CreateSurface( mesh ) 
      stx=-.5 
      sty=stx 
      stp=Float(1)/Float(segs) 
      y=sty 
       
      For a=0 To segs 
          
         x=stx 
         v=a/Float(segs) 
          
         For b=0 To segs 
             
            u=b/Float(segs) 
            AddVertex(surf,x,y,0.5,u,v) 
            x=x+stp 
             
         Next 
          
         y=y+stp 
          
      Next 
       
      For a=0 To segs-1 
          
         For b=0 To segs-1 
             
            v0=a*(segs+1)+b:v1=v0+1 
            v2=(a+1)*(segs+1)+b+1:v3=v2-1 
            AddTriangle( surf,v0,v1,v2 ) 
            AddTriangle( surf,v0,v2,v3 ) 
             
         Next 
          
      Next 
       
      RotateMesh mesh,0,90,0 
       
   Next 
    
   ;top and bottom 
   RotateMesh mesh,0,90,90 
    
   For side=0 To 1 
       
      surf=CreateSurface( mesh ) 
      stx#=-.5 
      sty#=stx 
      stp#=Float(1)/Float(segs) 
      y#=sty 
       
      For a=0 To segs 
         x#=stx 
         v#=a/Float(segs) 
         For b=0 To segs 
            u#=b/Float(segs) 
            AddVertex(surf,x,y,0.5,u,v) 
            x=x+stp 
         Next 
         y=y+stp 
      Next 
       
      For a=0 To segs-1 
         For b=0 To segs-1 
            v0=a*(segs+1)+b:v1=v0+1 
            v2=(a+1)*(segs+1)+b+1:v3=v2-1 
            AddTriangle( surf,v0,v1,v2 ) 
            AddTriangle( surf,v0,v2,v3 ) 
         Next 
      Next 
       
      RotateMesh mesh,180,0,0 
       
   Next 
    
   ; scale uniform to -1 to +1 space in X/Y/Z dimensions 
   FitMesh mesh,-1,-1,-1,2,2,2,1 
    
   Return mesh 
*/
scene::IMesh* createSegUnitCubeMesh(const core::vector3df& size)
{
	scene::SMeshBuffer* buffer = new scene::SMeshBuffer();

	// Create indices
	const u16 u[36] = {   0,2,1,   0,3,2,   1,5,4,   1,2,5,   4,6,7,   4,5,6, 
            7,3,0,   7,6,3,   9,5,2,   9,8,5,   0,11,10,   0,10,7};

	buffer->Indices.set_used(36);

	for (u32 i=0; i<36; ++i)
		buffer->Indices[i] = u[i];


	// Create vertices
	video::SColor clr(255,255,255,255);

	buffer->Vertices.reallocate(12);
	//FrontFace
	buffer->Vertices.push_back(video::S3DVertex(0,0,0, -1,-1,-1, clr, 0, 1));
	buffer->Vertices.push_back(video::S3DVertex(1,0,0,  1,-1,-1, clr, 1, 1));
	buffer->Vertices.push_back(video::S3DVertex(1,1,0,  1, 1,-1, clr, 1, 0));
	buffer->Vertices.push_back(video::S3DVertex(0,1,0, -1, 1,-1, clr, 0, 0));
	
	buffer->Vertices.push_back(video::S3DVertex(1,0,1,  1,-1, 1, clr, 0, 1));
	buffer->Vertices.push_back(video::S3DVertex(1,1,1,  1, 1, 1, clr, 0, 0));
	buffer->Vertices.push_back(video::S3DVertex(0,1,1, -1, 1, 1, clr, 1, 0));
	buffer->Vertices.push_back(video::S3DVertex(0,0,1, -1,-1, 1, clr, 1, 1));

	buffer->Vertices.push_back(video::S3DVertex(0,1,1, -1, 1, 1, clr, 0, 1));
	buffer->Vertices.push_back(video::S3DVertex(0,1,0, -1, 1,-1, clr, 1, 1));
	buffer->Vertices.push_back(video::S3DVertex(1,0,1,  1,-1, 1, clr, 1, 0));
	buffer->Vertices.push_back(video::S3DVertex(1,0,0,  1,-1,-1, clr, 0, 0));

	// Recalculate bounding box
	buffer->BoundingBox.reset(0,0,0);

	for (u32 i=0; i<12; ++i)
	{
		buffer->Vertices[i].Pos -= core::vector3df(0.5f, 0.5f, 0.5f);
		buffer->Vertices[i].Pos *= size;
		buffer->BoundingBox.addInternalPoint(buffer->Vertices[i].Pos);
	}

	scene::SMesh* mesh = new scene::SMesh;
	mesh->addMeshBuffer(buffer);
	buffer->drop();

	mesh->recalculateBoundingBox();
	return mesh;
}

void createSegUnitCubeMeshNew(u32 segments)
{
	f32 pointDist = 1.0 / segments;
	video::SColor clr(255,255,255,255);
	scene::SMeshBuffer* buffer = new scene::SMeshBuffer();
	
	//create flat plane vertice positions
	f32 currentX = 0.0f;
	f32 currentY = 1.0f;
	
	core::array<core::vector3df> flatPlane;
	
	for(u32 y=0; y<segments+1; y++)
	{
		for(u32 x=0; x<segments+1; x++)
		{
			core::vector3df currentPos = core::vector3df(currentX,currentY,0);
			flatPlane.push_back(currentPos);
			currentX += pointDist;
			printf("%f,%f,%f\n",currentPos.X,currentPos.Y,currentPos.Z);
		}
		currentX = 0.0f;
		currentY -= pointDist;
		printf("\n");
	}
	printf("%d\n",flatPlane.size());
}