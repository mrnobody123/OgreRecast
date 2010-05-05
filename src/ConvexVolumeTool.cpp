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

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#include "Ogre.h"
#include "ConvexVolumeTool.h"
#include "InputGeom.h"
#include "OgreTemplate.h"
#include "SharedData.h"
#include "Recast.h"
#include "RecastDebugDraw.h"
#include "DetourDebugDraw.h"

#include "GUIManager.h"

#ifdef WIN32
#	define snprintf _snprintf
#endif

// Quick and dirty convex hull.

// Returns true if 'c' is left of line 'a'-'b'.
inline bool left(const float* a, const float* b, const float* c)
{ 
	const float u1 = b[0] - a[0];
	const float v1 = b[2] - a[2];
	const float u2 = c[0] - a[0];
	const float v2 = c[2] - a[2];
	return u1 * v2 - v1 * u2 < 0;
}

// Returns true if 'a' is more lower-left than 'b'.
inline bool cmppt(const float* a, const float* b)
{
	if (a[0] < b[0]) return true;
	if (a[0] > b[0]) return false;
	if (a[2] < b[2]) return true;
	if (a[2] > b[2]) return false;
	return false;
}
// Calculates convex hull on xz-plane of points on 'pts',
// stores the indices of the resulting hull in 'out' and
// returns number of points on hull.
static int convexhull(const float* pts, int npts, int* out)
{
	// Find lower-leftmost point.
	int hull = 0;
	for (int i = 1; i < npts; ++i)
		if (cmppt(&pts[i*3], &pts[hull*3]))
			hull = i;
	// Gift wrap hull.
	int endpt = 0;
	int i = 0;
	do
	{
		out[i++] = hull;
		endpt = 0;
		for (int j = 1; j < npts; ++j)
			if (hull == endpt || left(&pts[hull*3], &pts[endpt*3], &pts[j*3]))
				endpt = j;
		hull = endpt;
	}
	while (endpt != out[0]);

	return i;
}

static int pointInPoly(int nvert, const float* verts, const float* p)
{
	int i, j, c = 0;
	for (i = 0, j = nvert-1; i < nvert; j = i++)
	{
		const float* vi = &verts[i*3];
		const float* vj = &verts[j*3];
		if (((vi[2] > p[2]) != (vj[2] > p[2])) &&
			(p[0] < (vj[0]-vi[0]) * (p[2]-vi[2]) / (vj[2]-vi[2]) + vi[0]) )
			c = !c;
	}
	return c;
}


ConvexVolumeTool::ConvexVolumeTool() :
	m_sample(0),
	m_areaType(SAMPLE_POLYAREA_GRASS),
	m_boxHeight(6.0f),
	m_boxDescent(1.0f),
	m_npts(0),
	m_nhull(0),
	dd(0),
	ddPoint(0)
{
}

ConvexVolumeTool::~ConvexVolumeTool()
{
	if(dd)
	{
		delete dd;
		dd = NULL;
	}
	if(ddPoint)
	{
		delete ddPoint;
		ddPoint = NULL;
	}
}

void ConvexVolumeTool::init(OgreTemplate* sample)
{
	m_sample = sample;
	dd = new DebugDrawGL();
	dd->setMaterialScript(Ogre::String("OffMeshConnection"));
	dd->setOffset(0.36f);
		

	ddPoint = new DebugDrawGL();
	ddPoint->setMaterialScript(Ogre::String("ConvexPoints"));
	ddPoint->setOffset(0.38f);
}

void ConvexVolumeTool::reset()
{
	m_npts = 0;
	m_nhull = 0;
	if(dd)
	{
		delete dd;
		dd = NULL;
	}
	if(ddPoint)
	{
		delete ddPoint;
		ddPoint = NULL;
	}
}

void ConvexVolumeTool::handleMenu()
{
}

void ConvexVolumeTool::handleClick(const float* p, bool shift)
{
	if (!m_sample) return;
	InputGeom* geom = m_sample->getInputGeom();
	if (!geom) return;

	if (shift)
	{
		// Delete
		int nearestIndex = -1;
		const ConvexVolume* vols = geom->getConvexVolumes();
		for (int i = 0; i < geom->getConvexVolumeCount(); ++i)
		{
			if (pointInPoly(vols[i].nverts, vols[i].verts, p) &&
				p[1] >= vols[i].hmin, p[1] <= vols[i].hmax)
			{
				nearestIndex = i;
			}
		}
		// If end point close enough, delete it.
		if (nearestIndex != -1)
		{
			geom->deleteConvexVolume(nearestIndex);
		}
	}
	else
	{
		// Create

		// If clicked on that last pt, create the shape.
		if (m_npts && rcVdistSqr(p, &m_pts[(m_npts-1)*3]) < rcSqr(0.2f))
		{
			if (m_nhull > 2)
			{
				// Create shape.
				float verts[MAX_PTS*3];
				for (int i = 0; i < m_nhull; ++i)
					rcVcopy(&verts[i*3], &m_pts[m_hull[i]*3]);

				float minh = FLT_MAX, maxh = 0;
				for (int i = 0; i < m_nhull; ++i)
					minh = rcMin(minh, verts[i*3+1]);
				minh -= m_boxDescent;
				maxh = minh + m_boxHeight;

				geom->addConvexVolume(verts, m_nhull, minh, maxh, (unsigned char)m_areaType);
			}

			m_npts = 0;
			m_nhull = 0;
		}
		else
		{
			// Add new point 
			if (m_npts < MAX_PTS)
			{
				rcVcopy(&m_pts[m_npts*3], p);
				m_npts++;
				// Update hull.
				if (m_npts > 1)
					m_nhull = convexhull(m_pts, m_npts, m_hull);
				else
					m_nhull = 0;
			}
			// MODIFICATION BY PAUL A WILSON
			// if we have more than the maximum allowed points, finish the shape
			// automatically - without this, we have to reset the tool if we exceed
			// the MAX_PTS variable
			if (m_npts >= MAX_PTS)
			{
				if (m_nhull > 2)
				{
					// Create shape.
					float verts[MAX_PTS*3];
					for (int i = 0; i < m_nhull; ++i)
						rcVcopy(&verts[i*3], &m_pts[m_hull[i]*3]);

					float minh = FLT_MAX, maxh = 0;
					for (int i = 0; i < m_nhull; ++i)
						minh = rcMin(minh, verts[i*3+1]);
					minh -= m_boxDescent;
					maxh = minh + m_boxHeight;

					geom->addConvexVolume(verts, m_nhull, minh, maxh, (unsigned char)m_areaType);
				}
				m_npts = 0;
				m_nhull = 0;
			}
		}		
	}

}

void ConvexVolumeTool::setClearShape(void)
{
	m_npts = 0;
	m_nhull = 0;
}

void ConvexVolumeTool::handleStep()
{
}

void ConvexVolumeTool::handleRender()
{
	dd->clear();
	ddPoint->clear();

	// Find height extents of the shape.
	float minh = FLT_MAX, maxh = 0;
	for (int i = 0; i < m_npts; ++i)
		minh = rcMin(minh, m_pts[i*3+1]);
	minh -= m_boxDescent;
	maxh = minh + m_boxHeight;

	ddPoint->begin(DU_DRAW_POINTS, 20.0f);
	for (int i = 0; i < m_npts; ++i)
	{
		unsigned int col = duRGBA(255,255,255,255);
		if (i == m_npts-1)
			col = duRGBA(240,32,16,255);
		ddPoint->vertex(m_pts[i*3+0],m_pts[i*3+1]+0.1f,m_pts[i*3+2], Ogre::ColourValue(0.9, 0.2, 0.1));
	}
	ddPoint->end();

	dd->begin(DU_DRAW_LINES, 6.0f);
	for (int i = 0, j = m_nhull-1; i < m_nhull; j = i++)
	{
		const float* vi = &m_pts[m_hull[j]*3];
		const float* vj = &m_pts[m_hull[i]*3];
		dd->vertex(vj[0],minh,vj[2], duRGBA(255,255,255,64));
		dd->vertex(vi[0],minh,vi[2], duRGBA(255,255,255,64));
		dd->vertex(vj[0],maxh,vj[2], duRGBA(255,255,255,64));
		dd->vertex(vi[0],maxh,vi[2], duRGBA(255,255,255,64));
		dd->vertex(vj[0],minh,vj[2], duRGBA(255,255,255,64));
		dd->vertex(vj[0],maxh,vj[2], duRGBA(255,255,255,64));
	}
	dd->end();	
}