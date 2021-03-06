/**************************************************************************
*
* File:		GaTestParticleComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaTestParticleComponent.h"

#include "System/Scene/Rendering/ScnShaderFileData.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include "System/Content/CsPackage.h"
#include "System/Content/CsCore.h"

#include "Base/BcProfiler.h"
#include "Base/BcRandom.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( GaTestParticleComponent );

void GaTestParticleComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Sound_", &GaTestParticleComponent::Sound_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
	};

	ReRegisterClass< GaTestParticleComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( 0 ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaTestParticleComponent::GaTestParticleComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaTestParticleComponent::~GaTestParticleComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaTestParticleComponent::update( BcF32 Tick )
{
	Super::update( Tick );

	static BcRandom Rand;

	static float Ticker = 0.0f;
	Ticker += Tick;
	if( Ticker > 0.01f )
	{
		Ticker -= 0.01f;
		//SoundEmitter_->setPitch( ( Rand.randReal() + 1.1f ) * 0.5f );
		SoundEmitter_->play( Sound_ );
	}

	ScnParticle* Particle = nullptr;
	if( ParticleSystem_->allocParticle( Particle ) )
	{
		Particle->Position_ = MaVec3d( 0.0f, 0.0f, 0.0f );
		Particle->CurrentTime_ = 0.0f;

		Particle->Position_ = MaVec3d( 0.0f, 0.0f, 0.0f );
		Particle->Velocity_ = ( MaVec3d( Rand.randReal(), Rand.randReal(), Rand.randReal() ) ).normal() * 10.0f;
		Particle->Acceleration_ = -Particle->Velocity_ * 0.5f;
		Particle->Scale_ = MaVec2d( 2.0f, 2.0f );
		Particle->MinScale_ = MaVec2d( 2.0f, 2.0f );
		Particle->MaxScale_ = MaVec2d( 0.0f, 0.0f );
		Particle->Rotation_ = Rand.randReal();
		Particle->RotationMultiplier_ = Rand.randReal();
		Particle->Colour_ = RsColour::WHITE * 1.0f;
		Particle->MinColour_ = RsColour::WHITE * 1.0f;
		Particle->MaxColour_ = RsColour( Rand.randReal(), Rand.randReal(), Rand.randReal(), 0.0f ) ;
		Particle->TextureIndex_ = 0;
		Particle->CurrentTime_ = 0.0f;
		Particle->MaxTime_ = 5.0f;
		Particle->Alive_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaTestParticleComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	ParticleSystem_ = Parent->getComponentAnyParentByType< ScnParticleSystemComponent >();	
	SoundEmitter_ = Parent->getComponentAnyParentByType< ScnSoundEmitterComponent >();
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaTestParticleComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
MaAABB GaTestParticleComponent::getAABB() const
{
	return MaAABB();
}
