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

/* Copyright Steve Rabin, 2005. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2005"
 */

#pragma once

#include "msg.h"
#include "singleton.h"
#include <list>

typedef std::list<MSG_Object*> MessageContainer;

class MsgRoute : public STM::Singleton <MsgRoute>
{
public:

	MsgRoute( void ) {}
	~MsgRoute( void );

	void DeliverDelayedMessages( void );
	void SendMsg( float delay, MSG_Name name,
	              objectID receiver, objectID sender, 
	              Scope_Rule rule, unsigned int scope,
	              MSG_Data data, bool timer, bool cc );
	
	void SendMsgBroadcast( MSG_Object & msg, unsigned int type = 0 );
	
	//Removing delayed messages
	void RemoveMsg( MSG_Name name, objectID receiver, objectID sender, bool timer );
	void PurgeScopedMsg( objectID receiver );

private:

	MessageContainer m_delayedMessages;

	void RouteMsg( MSG_Object & msg );

};