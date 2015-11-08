/**************************************************************************
*
* File:		GaTestComputeComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaTestComputeComponent.h"

#include "System/Scene/Rendering/ScnShaderFileData.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include "System/Content/CsPackage.h"
#include "System/Content/CsCore.h"

#include "Base/BcRandom.h"
#include "Base/BcHalf.h"
#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// GaVertex
struct GaVertex
{
	GaVertex()
	{}

	GaVertex( MaVec4d Position, MaVec4d Normal, MaVec4d Tangent, MaVec4d Colour, MaVec4d TexCoord ):
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
	MaVec4d TexCoord_;
};

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( GaTestComputeComponent );

void GaTestComputeComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "ComputeShader_", &GaTestComputeComponent::ComputeShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "Material_", &GaTestComputeComponent::Material_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),

		new ReField( "ObjectUniformBuffer_", &GaTestComputeComponent::ObjectUniformBuffer_, bcRFF_TRANSIENT ),
		new ReField( "TestUniformBuffer_", &GaTestComputeComponent::TestUniformBuffer_, bcRFF_TRANSIENT ),
		new ReField( "IndexBuffer_", &GaTestComputeComponent::IndexBuffer_, bcRFF_TRANSIENT ),
		new ReField( "VertexBuffer_", &GaTestComputeComponent::VertexBuffer_, bcRFF_TRANSIENT ),
		new ReField( "VertexDeclaration_", &GaTestComputeComponent::VertexDeclaration_, bcRFF_TRANSIENT ),
		new ReField( "MaterialComponent_", &GaTestComputeComponent::MaterialComponent_, bcRFF_TRANSIENT ),
	};
		
	ReRegisterClass< GaTestComputeComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaTestComputeComponent::GaTestComputeComponent():
	ObjectUniformBuffer_( nullptr ),
	TestUniformBuffer_( nullptr ),
	IndexBuffer_( nullptr ),
	VertexBuffer_( nullptr ),
	VertexDeclaration_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaTestComputeComponent::~GaTestComputeComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// render
//virtual 
void GaTestComputeComponent::render( ScnRenderContext & RenderContext )
{
	Super::render( RenderContext );

	RsCore::pImpl()->updateBuffer( 
		ObjectUniformBuffer_,
		0, sizeof( ScnShaderObjectUniformBlockData ),
		RsResourceUpdateFlags::ASYNC,
		[]( RsBuffer* Buffer, const RsBufferLock& Lock )
		{
			auto UniformBlock = reinterpret_cast< ScnShaderObjectUniformBlockData* >( Lock.Buffer_ );
			UniformBlock->WorldTransform_ = MaMat4d();
		} );


	// Set skinning parameters.
	MaterialComponent_->setObjectUniformBlock( ObjectUniformBuffer_ );
	
	// Set texture.
	MaterialComponent_->setTexture( "aDiffuseTex", ComputeOutputTextures_[ ComputeTextureIdx_ ] );
			
	// Set material components for view.
	RenderContext.pViewComponent_->setMaterialParameters( MaterialComponent_ );
			
	// Bind material.
	MaterialComponent_->bind( RenderContext.pFrame_, RenderContext.Sort_ );

	// Render primitive.				
	RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
		[
			ComputeProgram = ComputeShader_->getProgram( ScnShaderPermutationFlags::NONE ),
			ComputeOutputBuffer = ComputeOutputBuffer_,
			ComputeOutputTextures = ComputeOutputTextures_,
			ComputeTextureIdx = ComputeTextureIdx_,
			VertexBuffer = VertexBuffer_,
			VertexDeclaration = VertexDeclaration_
		]
		( RsContext* Context )
		{
			PSY_PROFILER_SECTION( RenderRoot, "GaTestComputeComponentRenderNode::render" );
			auto& Features = Context->getFeatures();
			if( Features.ComputeShaders_ )
			{
				RsProgramBindingDesc Bindings;
				BcU32 iBufferSlot = ComputeProgram->findShaderResourceSlot( "iBuffer" );
				BcU32 oBufferSlot = ComputeProgram->findUnorderedAccessSlot( "oBuffer" );
				BcU32 iTextureSlot = ComputeProgram->findShaderResourceSlot( "iTexture" );
				BcU32 oTextureSlot = ComputeProgram->findUnorderedAccessSlot( "oTexture" );

				if( iBufferSlot != BcErrorCode )
				{
					Bindings.setShaderResourceView( iBufferSlot, VertexBuffer );
				}

				if( iTextureSlot != BcErrorCode )
				{
					Bindings.setShaderResourceView( iTextureSlot, ComputeOutputTextures[ ComputeTextureIdx ]->getTexture() );
				}

				if( oBufferSlot != BcErrorCode )
				{
					Bindings.setUnorderedAccessView( oBufferSlot, ComputeOutputBuffer );
				}

				if( oTextureSlot != BcErrorCode )
				{
					Bindings.setUnorderedAccessView( oTextureSlot, ComputeOutputTextures[ 1 - ComputeTextureIdx ]->getTexture() );
				}

				const auto& texDesc = ComputeOutputTextures[ 1 - ComputeTextureIdx ]->getTexture()->getDesc();

				Context->dispatchCompute( ComputeProgram, Bindings, texDesc.Width_, texDesc.Height_, 1 );
				Context->setVertexBuffer( 0, VertexBuffer, sizeof( GaVertex ) );
			}
			else
			{
				Context->setVertexBuffer( 0, VertexBuffer, sizeof( GaVertex ) );
			}

			Context->setVertexDeclaration( VertexDeclaration );
			Context->drawPrimitives( RsTopologyType::TRIANGLE_STRIP, 0, 4 );
		} );
	ComputeTextureIdx_ = 1 - ComputeTextureIdx_;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaTestComputeComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );


	ObjectUniformBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc(
			RsBufferType::UNIFORM,
			RsResourceCreationFlags::STREAM,
			sizeof( ScnShaderObjectUniformBlockData ) ) );

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

		BcU32 VertexBufferSize = 4 * sizeof( GaVertex );
	VertexBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc( 
			RsResourceBindFlags::VERTEX_BUFFER | RsResourceBindFlags::SHADER_RESOURCE,
			RsResourceCreationFlags::STATIC,
			VertexBufferSize ) );

	ComputeOutputBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc( 
			RsResourceBindFlags::VERTEX_BUFFER | RsResourceBindFlags::UNORDERED_ACCESS,
			RsResourceCreationFlags::STATIC,
			VertexBufferSize ) );

	BcBool RandomTex = BcTrue;
	for( auto& ComputeOutputTexture : ComputeOutputTextures_ )
	{
		ComputeOutputTexture = ScnTexture::New( 
			RsTextureDesc( 
				RsTextureType::TEX2D, 
				RsResourceCreationFlags::STATIC, 
				RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::UNORDERED_ACCESS,
				RsTextureFormat::R32F,
				1,
				512, 512, 1 ) );

		RsCore::pImpl()->updateTexture( 
			ComputeOutputTexture->getTexture(),
			ComputeOutputTexture->getTexture()->getSlice(),
			RsResourceUpdateFlags::ASYNC,
			[ RandomTex ]( RsTexture* Texture, const RsTextureLock& Lock )
			{
				const auto& Desc = Texture->getDesc();
				for( BcU32 Y = 0; Y < Desc.Height_; ++Y )
				{
					BcF32* Data = reinterpret_cast< BcF32* >( 
						reinterpret_cast< BcU8* >( Lock.Buffer_ ) + Y * Lock.Pitch_ );
					for( BcU32 X = 0; X < Desc.Width_; ++X )
					{
						if( RandomTex && BcRandom::Global.randRealRange( 0.0f, 1.0f ) > 0.99f )
						{
							if( BcRandom::Global.randRealRange( 0.0f, 1.0f ) >= 0.5f )
							{
								*Data++ = ( 1.0f );
							}
							else
							{
								*Data++ = ( -1.0f );
							}
						}
						else
						{
							*Data++ = BcF32ToHalf( 0.0f );
						}
					}
				}
			} );
		RandomTex = BcFalse;
	}

	VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration( RsVertexDeclarationDesc( 5 )
		.addElement( RsVertexElement( 0,  0, 4, RsVertexDataType::FLOAT32,    RsVertexUsage::POSITION, 0 ) )
		.addElement( RsVertexElement( 0, 16, 4, RsVertexDataType::FLOAT32,    RsVertexUsage::NORMAL, 0 ) )
		.addElement( RsVertexElement( 0, 32, 4, RsVertexDataType::FLOAT32,    RsVertexUsage::TANGENT, 0 ) )
		.addElement( RsVertexElement( 0, 48, 4, RsVertexDataType::FLOAT32,    RsVertexUsage::COLOUR, 0 ) )
		.addElement( RsVertexElement( 0, 64, 4, RsVertexDataType::FLOAT32,    RsVertexUsage::TEXCOORD, 0 ) ) );

	RsCore::pImpl()->updateBuffer( 
		VertexBuffer_,
		0, VertexBufferSize,
		RsResourceUpdateFlags::ASYNC,
		[ VertexBufferSize ]( RsBuffer* Buffer, const RsBufferLock& Lock )
		{
			auto Vertices = reinterpret_cast< GaVertex* >( Lock.Buffer_ );
			memset( Vertices, 0, VertexBufferSize );
			*Vertices++ = GaVertex( MaVec4d( -10.0f, -10.0f,  0.0f,  1.0f ), MaVec4d( 0.0f, 0.0f, 1.0f,  1.0f ), MaVec4d( 1.0f, 0.0f, 0.0f,  1.0f ), MaVec4d( 1.0f, 1.0f, 1.0f, 1.0f ), MaVec4d( 0.0f, 0.0f, 0.0f, 0.0f ) );
			*Vertices++ = GaVertex( MaVec4d(  10.0f, -10.0f,  0.0f,  1.0f ), MaVec4d( 0.0f, 0.0f, 1.0f,  1.0f ), MaVec4d( 1.0f, 0.0f, 0.0f,  1.0f ), MaVec4d( 1.0f, 1.0f, 1.0f, 1.0f ), MaVec4d( 1.0f, 0.0f, 0.0f, 0.0f ) );
			*Vertices++ = GaVertex( MaVec4d( -10.0f,  10.0f,  0.0f,  1.0f ), MaVec4d( 0.0f, 0.0f, 1.0f,  1.0f ), MaVec4d( 1.0f, 0.0f, 0.0f,  1.0f ), MaVec4d( 1.0f, 1.0f, 1.0f, 1.0f ), MaVec4d( 0.0f, 1.0f, 0.0f, 0.0f ) );
			*Vertices++ = GaVertex( MaVec4d(  10.0f,  10.0f,  0.0f,  1.0f ), MaVec4d( 0.0f, 0.0f, 1.0f,  1.0f ), MaVec4d( 1.0f, 0.0f, 0.0f,  1.0f ), MaVec4d( 1.0f, 1.0f, 1.0f, 1.0f ), MaVec4d( 1.0f, 1.0f, 0.0f, 0.0f ) );
		} );

	RsCore::pImpl()->updateBuffer( 
		ComputeOutputBuffer_,
		0, VertexBufferSize,
		RsResourceUpdateFlags::ASYNC,
		[ VertexBufferSize ]( RsBuffer* Buffer, const RsBufferLock& Lock )
		{
			auto Vertices = reinterpret_cast< GaVertex* >( Lock.Buffer_ );
			memset( Vertices, 0, VertexBufferSize );
		} );

	ScnShaderPermutationFlags ShaderPermutation = 
		ScnShaderPermutationFlags::MESH_STATIC_3D |
		ScnShaderPermutationFlags::RENDER_FORWARD |
		ScnShaderPermutationFlags::LIGHTING_NONE;

	// Attach a new material component.
	MaterialComponent_ = Parent->attach< ScnMaterialComponent >(
		BcName::INVALID, Material_, ShaderPermutation );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaTestComputeComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	if(	ComputeOutputTextures_[0] )
	{
		ComputeOutputTextures_[0]->markDestroy();
		ComputeOutputTextures_[0] = nullptr;
	}

	if(	ComputeOutputTextures_[1] )
	{
		ComputeOutputTextures_[1]->markDestroy();
		ComputeOutputTextures_[1] = nullptr;
	}

	Parent->detach( MaterialComponent_ );
	MaterialComponent_ = nullptr;

	RsCore::pImpl()->destroyResource( VertexDeclaration_ );
	RsCore::pImpl()->destroyResource( ComputeOutputBuffer_ );
	RsCore::pImpl()->destroyResource( VertexBuffer_ );
	RsCore::pImpl()->destroyResource( IndexBuffer_ );
	RsCore::pImpl()->destroyResource( ObjectUniformBuffer_ );
}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
MaAABB GaTestComputeComponent::getAABB() const
{
	return MaAABB();
}