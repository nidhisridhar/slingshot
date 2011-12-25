#pragma once

#include <ISceneNode.h>
#include "CMeshBuffer.h"

//////////////////////////////////////////////////////////////////////////////////
//Frei nach http://irrlicht.sourceforge.net/docu/example003.html


//
class CIndexedPrimitiveNode : public irr::scene::ISceneNode
{
   public:
      //
      CIndexedPrimitiveNode(irr::scene::ISceneNode* parent,
							irr::scene::ISceneManager* mgr,
							irr::s32 id,
							irr::scene::SMeshBuffer* pMesh,
							irr::u32 primitiveCount,
							irr::scene::E_PRIMITIVE_TYPE primitiveType);
      ~CIndexedPrimitiveNode();

      //
      void setNewMesh(irr::scene::SMeshBuffer* pMesh,irr::u32 primitiveCount)
      {
         m_pMesh->drop();
         m_pMesh = pMesh;
         m_pMesh->grab();
         m_primitiveCount = primitiveCount;
      }

      //
      void setPrimitiveType(irr::scene::E_PRIMITIVE_TYPE type) { m_primitiveType = type; }

      //zu ¸berschreiben aus Basisklasse
      virtual void OnRegisterSceneNode();
       
      //zu ¸berschreiben aus Basisklasse
      virtual void render();

      //zu ¸berschreiben aus Basisklasse
      virtual const irr::core::aabbox3d<irr::f32>& getBoundingBox() const;

      //zu ¸berschreiben aus Basisklasse
      virtual irr::u32 getMaterialCount() const;

      //zu ¸berschreiben aus Basisklasse
      virtual irr::video::SMaterial& getMaterial(irr::u32 i);


   private:
      //das zu rendernde Mesh samt Material
      irr::scene::SMeshBuffer* m_pMesh;
      //der gew‰hlte Primitiv-Typ
      irr::scene::E_PRIMITIVE_TYPE m_primitiveType;
      //Anzahl der zu rendernden Primitive
      irr::u32 m_primitiveCount;
}; 