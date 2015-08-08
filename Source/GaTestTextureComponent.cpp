/**************************************************************************
*
* File:		GaTestTextureComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaTestTextureComponent.h"

#include "System/Scene/Rendering/ScnShaderFileData.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include "System/Content/CsPackage.h"
#include "System/Content/CsCore.h"

#include "System/SysKernel.h"

#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// GaVertex
struct GaVertex
{
	GaVertex()
	{}

	GaVertex( MaVec4d Position, MaVec4d Normal, MaVec4d Tangent, MaVec4d Colour, MaVec2d TexCoord ):
		Position_( Position ),
		Normal_( Normal ),
		Tangent_( Tangent ),
		Colour_( Colour ),
		TexCoord_( TexCoord )
	{}

	MaVec4d Position_;
	MaVec4d Normal_;
	MaVec4d Tangent_;
	MaVec4d Colour_;
	MaVec2d TexCoord_;
};

//////////////////////////////////////////////////////////////////////////
// GaTestTextureBlockData
REFLECTION_DEFINE_BASIC( GaTestTextureBlockData );

void GaTestTextureBlockData::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "ColourTransform_", &GaTestTextureBlockData::UVWOffset_ ),
	};

	ReRegisterClass< GaTestTextureBlockData >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( GaTestTextureComponent );

void GaTestTextureComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Material1D_", &GaTestTextureComponent::Material1D_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "Material2D_", &GaTestTextureComponent::Material2D_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "Material3D_", &GaTestTextureComponent::Material3D_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),

		new ReField( "ObjectUniformBuffer_", &GaTestTextureComponent::ObjectUniformBuffer_, bcRFF_TRANSIENT ),
		new ReField( "TestUniformBuffer_", &GaTestTextureComponent::TestUniformBuffer_, bcRFF_TRANSIENT ),
		new ReField( "IndexBuffer_", &GaTestTextureComponent::IndexBuffer_, bcRFF_TRANSIENT ),
		new ReField( "VertexBuffer_", &GaTestTextureComponent::VertexBuffer_, bcRFF_TRANSIENT ),
		new ReField( "VertexDeclaration_", &GaTestTextureComponent::VertexDeclaration_, bcRFF_TRANSIENT ),
		new ReField( "MaterialComponent1D_", &GaTestTextureComponent::MaterialComponent1D_, bcRFF_TRANSIENT ),
		new ReField( "MaterialComponent2D_", &GaTestTextureComponent::MaterialComponent2D_, bcRFF_TRANSIENT ),
		new ReField( "MaterialComponent3D_", &GaTestTextureComponent::MaterialComponent3D_, bcRFF_TRANSIENT ),
		new ReField( "Texture1D_", &GaTestTextureComponent::Texture1D_, bcRFF_TRANSIENT ),
		new ReField( "Texture2D_", &GaTestTextureComponent::Texture2D_, bcRFF_TRANSIENT ),
		new ReField( "Texture3D_", &GaTestTextureComponent::Texture3D_, bcRFF_TRANSIENT ),
	};
		
	ReRegisterClass< GaTestTextureComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaTestTextureComponent::GaTestTextureComponent():
	ObjectUniformBuffer_( nullptr ),
	TestUniformBuffer_( nullptr ),
	IndexBuffer_( nullptr ),
	VertexBuffer_( nullptr ),
	VertexDeclaration_( nullptr ),
	Texture1D_( nullptr ),
	Texture2D_( nullptr ),
	Texture3D_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaTestTextureComponent::~GaTestTextureComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// drawTest
void GaTestTextureComponent::drawTest(
		const MaMat4d& Transform, ScnMaterialComponent* Material,
		class ScnViewComponent* pViewComponent, RsFrame* pFrame, 
		RsRenderSort Sort )
{
	if( Material )
	{
		// Set parameters.
		Material->setObjectUniformBlock( ObjectUniformBuffer_ );
		Material->setUniformBlock( "GaTestTextureBlockData", TestUniformBuffer_ );
		
		// Set material components for view.
		pViewComponent->setMaterialParameters( Material );
				
		// Bind material.
		Material->bind( pFrame, Sort );

		// Render primitive.				
		pFrame->queueRenderNode( Sort,
			[ this, Transform ]( RsContext* Context )
			{
				PSY_PROFILER_SECTION( RenderRoot, "GaTestTextureComponentRenderNode::render" );

				Context->updateBuffer( 
					ObjectUniformBuffer_,
					0, sizeof( ScnShaderObjectUniformBlockData ),
					RsResourceUpdateFlags::ASYNC,
					[ Transform ]( RsBuffer* Buffer, const RsBufferLock& Lock )
					{
						auto UniformBlock = reinterpret_cast< ScnShaderObjectUniformBlockData* >( Lock.Buffer_ );
						UniformBlock->WorldTransform_ = Transform;
					} );

				Context->setVertexBuffer( 0, VertexBuffer_, sizeof( GaVertex ) );
				Context->setVertexDeclaration( VertexDeclaration_ );
				Context->drawPrimitives( RsTopologyType::TRIANGLE_STRIP, 0, 4 );
			} );
	}
}

//////////////////////////////////////////////////////////////////////////
// render
//virtual 
void GaTestTextureComponent::render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{
	Super::render( pViewComponent, pFrame, Sort );

	RsCore::pImpl()->updateBuffer( 
		TestUniformBuffer_,
		0, sizeof( GaTestTextureBlockData ),
		RsResourceUpdateFlags::ASYNC,
		[]( RsBuffer* Buffer, const RsBufferLock& Lock )
		{
			auto UniformBlock = reinterpret_cast< GaTestTextureBlockData* >( Lock.Buffer_ );
			static BcF32 Timer = 0.0f;
			Timer += SysKernel::pImpl()->getFrameTime() * 0.05f;
			UniformBlock->UVWOffset_ = MaVec4d( Timer, Timer * 0.5f, Timer * 2.0f, 0.0f );
		} );

	MaMat4d Transform;

	Transform.translation( MaVec3d( -2.0f, 0.0f, 0.0f ) );
	drawTest( Transform, MaterialComponent1D_, pViewComponent, pFrame, Sort );

	Transform.translation( MaVec3d( 0.0f, 0.0f, 0.0f ) );
	drawTest( Transform, MaterialComponent2D_, pViewComponent, pFrame, Sort );

	Transform.translation( MaVec3d( 2.0f, 0.0f, 0.0f ) );
	drawTest( Transform, MaterialComponent3D_, pViewComponent, pFrame, Sort );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaTestTextureComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );


	ObjectUniformBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc(
			RsBufferType::UNIFORM,
			RsResourceCreationFlags::STREAM,
			sizeof( ScnShaderObjectUniformBlockData ) ) );


	TestUniformBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc(
			RsBufferType::UNIFORM,
			RsResourceCreationFlags::STREAM,
			sizeof( GaTestTextureBlockData ) ) );

	BcU32 IndexBufferSize = sizeof( BcU16 ) * 4;
	IndexBuffer_ = RsCore::pImpl()->createBuffer(
		RsBufferDesc( RsBufferType::INDEX, RsResourceCreationFlags::STATIC, IndexBufferSize ) );

	RsCore::pImpl()->updateBuffer( 
		IndexBuffer_, 0, IndexBufferSize, RsResourceUpdateFlags::ASYNC,
		[]( RsBuffer* Buffer, const RsBufferLock& BufferLock )
		{
			BcU16* Indices = reinterpret_cast< BcU16* >( BufferLock.Buffer_ );
			Indices[ 0 ] = 0;
			Indices[ 1 ] = 1;
			Indices[ 2 ] = 2;
			Indices[ 3 ] = 3;
		} );

	BcU32 VertexBufferSize = 2048 * sizeof( GaVertex );
	VertexBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc( 
			RsBufferType::VERTEX,
			RsResourceCreationFlags::STATIC,
			VertexBufferSize ) );

	VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration( RsVertexDeclarationDesc( 5 )
		.addElement( RsVertexElement( 0,  0, 4, RsVertexDataType::FLOAT32,    RsVertexUsage::POSITION, 0 ) )
		.addElement( RsVertexElement( 0, 16, 4, RsVertexDataType::FLOAT32,    RsVertexUsage::NORMAL, 0 ) )
		.addElement( RsVertexElement( 0, 32, 4, RsVertexDataType::FLOAT32,    RsVertexUsage::TANGENT, 0 ) )
		.addElement( RsVertexElement( 0, 48, 4, RsVertexDataType::FLOAT32,    RsVertexUsage::COLOUR, 0 ) )
		.addElement( RsVertexElement( 0, 64, 2, RsVertexDataType::FLOAT32,    RsVertexUsage::TEXCOORD, 0 ) ) );

	RsCore::pImpl()->updateBuffer( 
		VertexBuffer_,
		0, VertexBufferSize,
		RsResourceUpdateFlags::ASYNC,
		[]( RsBuffer* Buffer, const RsBufferLock& Lock )
		{
			auto Vertices = reinterpret_cast< GaVertex* >( Lock.Buffer_ );
			*Vertices++ = GaVertex( MaVec4d( -1.0f, -1.0f,  0.0f,  1.0f ) * 10.0f, MaVec4d( 0.0f, 0.0f, 1.0f,  1.0f ), MaVec4d( 1.0f, 0.0f, 0.0f,  1.0f ), MaVec4d( 1.0f, 1.0f, 1.0f, 1.0f ), MaVec2d( 0.0f, 0.0f ) );
			*Vertices++ = GaVertex( MaVec4d(  1.0f, -1.0f,  0.0f,  1.0f ) * 10.0f, MaVec4d( 0.0f, 0.0f, 1.0f,  1.0f ), MaVec4d( 1.0f, 0.0f, 0.0f,  1.0f ), MaVec4d( 1.0f, 1.0f, 1.0f, 1.0f ), MaVec2d( 1.0f, 0.0f ) );
			*Vertices++ = GaVertex( MaVec4d( -1.0f,  1.0f,  0.0f,  1.0f ) * 10.0f, MaVec4d( 0.0f, 0.0f, 1.0f,  1.0f ), MaVec4d( 1.0f, 0.0f, 0.0f,  1.0f ), MaVec4d( 1.0f, 1.0f, 1.0f, 1.0f ), MaVec2d( 0.0f, 1.0f ) );
			*Vertices++ = GaVertex( MaVec4d(  1.0f,  1.0f,  0.0f,  1.0f ) * 10.0f, MaVec4d( 0.0f, 0.0f, 1.0f,  1.0f ), MaVec4d( 1.0f, 0.0f, 0.0f,  1.0f ), MaVec4d( 1.0f, 1.0f, 1.0f, 1.0f ), MaVec2d( 1.0f, 1.0f ) );
		} );

	ScnShaderPermutationFlags ShaderPermutation = 
		ScnShaderPermutationFlags::MESH_STATIC_3D |
		ScnShaderPermutationFlags::RENDER_FORWARD |
		ScnShaderPermutationFlags::LIGHTING_NONE;

	// Setup materials & textures.
	if( Material1D_ )
	{
		MaterialComponent1D_ = Parent->attach< ScnMaterialComponent >(
			BcName::INVALID, Material1D_, ShaderPermutation );

		// Create texture.
		Texture1D_ = new ScnTexture( 32, 1, RsTextureFormat::R8G8B8A8 );
		auto Slice = Texture1D_->getTexture()->getSlice( 0 );
		RsCore::pImpl()->updateTexture( 
			Texture1D_->getTexture(),
			Slice,
			RsResourceUpdateFlags::ASYNC,
			[]( RsTexture* Texture, const RsTextureLock& Lock )
			{
				const auto& Desc = Texture->getDesc();
				BcU32* Data = reinterpret_cast< BcU32* >( 
					reinterpret_cast< BcU8* >( Lock.Buffer_ ) );
				for( BcU32 X = 0; X < Desc.Width_; ++X )
				{
					const BcU32 XDiv = X / 4;
					*Data++ = ( ( ( XDiv ) & 1 ) == 0 ) ? 0xffffffff : 0xff000000;
				}
			} );

		// Bind.
		MaterialComponent1D_->setTexture( "aDiffuseTex", Texture1D_ );
	}
	if( Material2D_ )
	{
		MaterialComponent2D_ = Parent->attach< ScnMaterialComponent >(
			BcName::INVALID, Material2D_, ShaderPermutation );

		// Create texture.
		Texture2D_ = new ScnTexture( 32, 32, 1, RsTextureFormat::R8G8B8A8 );
		auto Slice = Texture2D_->getTexture()->getSlice( 0 );
		RsCore::pImpl()->updateTexture( 
			Texture2D_->getTexture(),
			Slice,
			RsResourceUpdateFlags::ASYNC,
			[]( RsTexture* Texture, const RsTextureLock& Lock )
			{
				const auto& Desc = Texture->getDesc();
				for( BcU32 Y = 0; Y < Desc.Height_; ++Y )
				{
					BcU32* Data = reinterpret_cast< BcU32* >( 
						reinterpret_cast< BcU8* >( Lock.Buffer_ ) + Y * Lock.Pitch_ );
					for( BcU32 X = 0; X < Desc.Width_; ++X )
					{
						const BcU32 XDiv = X / 4;
						const BcU32 YDiv = Y / 4;
						*Data++ = ( ( ( XDiv + YDiv ) & 1 ) == 0 ) ? 0xffffffff : 0xff000000;
					}
				}
			} );

		// Bind.
		MaterialComponent2D_->setTexture( "aDiffuseTex", Texture2D_ );
	}
	if( Material3D_ )
	{
		MaterialComponent3D_ = Parent->attach< ScnMaterialComponent >(
			BcName::INVALID, Material3D_, ShaderPermutation );

		// Create texture.
		Texture3D_ = new ScnTexture( 32, 32, 32, 1, RsTextureFormat::R8G8B8A8 );
		auto Slice = Texture3D_->getTexture()->getSlice( 0 );
		RsCore::pImpl()->updateTexture( 
			Texture3D_->getTexture(),
			Slice,
			RsResourceUpdateFlags::ASYNC,
			[]( RsTexture* Texture, const RsTextureLock& Lock )
			{
				const auto& Desc = Texture->getDesc();
				for( BcU32 Z = 0; Z < Desc.Depth_; ++Z )
				{
					BcU32* SliceData = reinterpret_cast< BcU32* >( 
						reinterpret_cast< BcU8* >( Lock.Buffer_ ) + Z * Lock.SlicePitch_ );
					for( BcU32 Y = 0; Y < Desc.Height_; ++Y )
					{
						BcU32* Data = reinterpret_cast< BcU32* >( 
							reinterpret_cast< BcU8* >( SliceData ) + Y * Lock.Pitch_ );
						for( BcU32 X = 0; X < Desc.Width_; ++X )
						{
							const BcU32 XDiv = X / 4;
							const BcU32 YDiv = Y / 4;
							const BcU32 ZDiv = Z / 4;
							*Data++ = ( ( ( XDiv + YDiv + ZDiv ) & 1 ) == 0 ) ? 0xffffffff : 0xff000000;
						}
					}
				}
			} );

		// Bind.
		MaterialComponent3D_->setTexture( "aDiffuseTex", Texture3D_ );
	}

}
	
//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaTestTextureComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	if(	Texture1D_ )
	{
		Texture1D_->markDestroy();
		Texture1D_ = nullptr;
	}

	if(	Texture2D_ )
	{
		Texture2D_->markDestroy();
		Texture2D_ = nullptr;
	}

	if(	Texture3D_ )
	{
		Texture3D_->markDestroy();
		Texture3D_ = nullptr;
	}

	if( MaterialComponent1D_ )
	{
		Parent->detach( MaterialComponent1D_ );
		MaterialComponent1D_ = nullptr;
	}

	if( MaterialComponent2D_ )
	{
		Parent->detach( MaterialComponent2D_ );
		MaterialComponent2D_ = nullptr;
	}

	if( MaterialComponent3D_ )
	{
		Parent->detach( MaterialComponent3D_ );
		MaterialComponent3D_ = nullptr;
	}

	RsCore::pImpl()->destroyResource( VertexDeclaration_ );
	RsCore::pImpl()->destroyResource( VertexBuffer_ );
	RsCore::pImpl()->destroyResource( IndexBuffer_ );
	RsCore::pImpl()->destroyResource( ObjectUniformBuffer_ );
	RsCore::pImpl()->destroyResource( TestUniformBuffer_ );
}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
MaAABB GaTestTextureComponent::getAABB() const
{
	return MaAABB();
}