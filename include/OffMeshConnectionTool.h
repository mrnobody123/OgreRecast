//----------------------------------------------------------------------------------//
// OgreRecast Demo - A demonstration of integrating Recast Navigation Meshes		//
//					 with the Ogre3D rendering engine.								//
//																					//
//	This file was either Created by or Modified by :								//
//													Paul A Wilson 					//
//			All contents are Copyright (C) 2010 Paul A Wilson						//
//			Except where otherwise mentioned or where previous						//
//			copyright exists. In the case of pre-existing copyrights				//
//			all rights remain with the original Authors, and this is				//
//			to be considered a derivative work.										//
//																					//
//	Contact Email	:	paulwilson77@dodo.com.au									//
//																					//
// This 'SOFTWARE' is provided 'AS-IS', without any express or implied				//
// warranty.  In no event will the authors be held liable for any damages			//
// arising from the use of this software.											//
// Permission is granted to anyone to use this software for any purpose,			//
// including commercial applications, and to alter it and redistribute it			//
// freely, subject to the following restrictions:									//
// 1. The origin of this software must not be misrepresented; you must not			//
//    claim that you wrote the original software. If you use this software			//
//    in a product, an acknowledgment in the product documentation would be			//
//    appreciated but is not required.												//
// 2. Altered source versions must be plainly marked as such, and must not be		//
//    misrepresented as being the original software.								//
// 3. This notice may not be removed or altered from any source distribution.		//
//																					//
//----------------------------------------------------------------------------------//


//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef _H_OFFMESHCONNECTIONTOOL_H
#define _H_OFFMESHCONNECTIONTOOL_H

#include "OgreTemplate.h"



// Tool to create off-mesh connection for InputGeom

class OffMeshConnectionTool : public SampleTool
{
	OgreTemplate* m_sample;
	float m_hitPos[3];
	bool m_hitPosSet;
	bool m_bidir;
	unsigned char m_oldFlags;

	DebugDrawGL* dd;
	DebugDrawGL* ddOffMesh;
	
public:
	OffMeshConnectionTool();
	~OffMeshConnectionTool();
	
	virtual int type() { return TOOL_OFFMESH_CONNECTION; }
	virtual void init(OgreTemplate* sample);
	virtual void reset();
	virtual void handleMenu();
	virtual void handleClick(const float* p, bool shift);
	virtual void handleStep();
	virtual void handleRender(float _timeSinceLastFrame);

	virtual void setBiDirection(bool _setBiDir) { m_bidir = _setBiDir; }
};

#endif // _H_OFFMESHCONNECTIONTOOL_H
