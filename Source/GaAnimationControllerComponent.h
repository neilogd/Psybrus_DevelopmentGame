/**************************************************************************
*
* File:		GaAnimationControllerComponent.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Camera component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaAnimationControllerComponent_H__
#define __GaAnimationControllerComponent_H__

#include "Psybrus.h"

#include "System/Scene/Animation/ScnAnimation.h"
#include "System/Scene/Animation/ScnAnimationComponent.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef CsResourceRef< class GaAnimationControllerComponent > GaAnimationControllerComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaAnimationControllerComponent
class GaAnimationControllerComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaAnimationControllerComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcF32 Tick );

	virtual void						onAttach( ScnEntityWeakRef Parent );
	
private:
	class ScnAnimationTreeBlendNode*	pRootTrack_;
	class ScnAnimationTreeTrackNode*	pIdleTrack_;
	class ScnAnimationTreeTrackNode*	pReloadTrack_;

	ScnAnimationRef						AnimIdle_;
	ScnAnimationRef						AnimFire_;
	ScnAnimationRef						AnimReload_;
};

#endif

