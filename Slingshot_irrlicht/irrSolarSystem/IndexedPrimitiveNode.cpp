#include "IndexedPrimitiveNode.h"
#include "ISceneManager.h"
#include "IVideoDriver.h"


using namespace irr;
using namespace scene;

//
CIndexedPrimitiveNode::CIndexedPrimitiveNode(irr::scene::ISceneNode* parent,
											 irr::scene::ISceneManager* mgr,
											 irr::s32 id,
											 scene::SMeshBuffer* pMesh,
											 irr::u32 primitiveCount,
											 irr::scene::E_PRIMITIVE_TYPE primitiveType) : scene::ISceneNode(parent,
																						mgr,id),
																						m_pMesh(pMesh),
																						m_primitiveCount(primitiveCount),
																						m_primitiveType(primitiveType)
{
	
	this->setAutomaticCulling(EAC_OFF);
	//pMesh->grab();
	//m_primitiveType  = scene::EPT_LINES;   
}


//
CIndexedPrimitiveNode::~CIndexedPrimitiveNode()
{
   m_pMesh->drop();
   //this->remove();
}

//
void CIndexedPrimitiveNode::OnRegisterSceneNode()
{
   if (IsVisible)
	   SceneManager->registerNodeForRendering(this, ESNRP_SOLID);

   ISceneNode::OnRegisterSceneNode();
}

//
void CIndexedPrimitiveNode::render()
{
   if (!m_pMesh->getVertexCount() || !m_pMesh->getIndexCount()) return;


   video::IVideoDriver* driver = SceneManager->getVideoDriver();
   driver->setMaterial(m_pMesh->getMaterial());
   driver->setTransform(video::ETS_WORLD,AbsoluteTransformation);
   driver->drawVertexPrimitiveList(m_pMesh->getVertices(),
									m_pMesh->getVertexCount(),
									m_pMesh->getIndices(),
									m_primitiveCount,
									m_pMesh->getVertexType(),
									m_primitiveType,
									m_pMesh->getIndexType());   

}

//
const core::aabbox3d<f32>& CIndexedPrimitiveNode::getBoundingBox() const
{
   return m_pMesh->getBoundingBox();
}

//
irr::u32 CIndexedPrimitiveNode::getMaterialCount() const
{
   return 1;
}

//
irr::video::SMaterial& CIndexedPrimitiveNode::getMaterial(irr::u32 i)
{
   return m_pMesh->getMaterial();
}       
