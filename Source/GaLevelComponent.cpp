/**************************************************************************
*
* File:		GaLevelComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	11/01/13	
* Description:
*		Level component.
*		
*
*
* 
**************************************************************************/

#include "GaLevelComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( GaLevelComponent );

void GaLevelComponent::StaticRegisterClass()
{
	ReRegisterClass< GaLevelComponent, Super >()
		.addAttribute( new ScnComponentAttribute( 0 ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaLevelComponent::GaLevelComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaLevelComponent::~GaLevelComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaLevelComponent::update( BcF32 Tick )
{
	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaLevelComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	BcU32 Idx = 0;
	BcF32 Width = 1024.0f;

	for( BcF32 X = -Width; X <= Width; X += 64.0f )
	{
		ScnEntitySpawnParams EntityParams = 
		{
			"default", "AnimatedEntity", BcName( "AnimatedEntity", Idx++ ),
			MaMat4d(),
			getParentEntity()
		};

		EntityParams.Transform_.translation( MaVec3d( 0.0f, 0.0f, X ) );
		ScnCore::pImpl()->spawnEntity( EntityParams );
	}

	PSY_LOG( "Spawned %u animated entities.\n", Idx );
}
