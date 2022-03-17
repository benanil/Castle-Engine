// This code contains NVIDIA Confidential Information and is disclosed
// under the Mutual Non-Disclosure Agreement.
//
// Notice
// ALL NVIDIA DESIGN SPECIFICATIONS AND CODE ("MATERIALS") ARE PROVIDED "AS IS" NVIDIA MAKES
// NO REPRESENTATIONS, WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ANY IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
//
// NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. No third party distribution is allowed unless
// expressly authorized by NVIDIA.  Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (C) 2016, NVIDIA Corporation. All rights reserved.
//
// NVIDIA Corporation and its licensors retain all intellectual property and proprietary
// rights in and to this software and related documentation and any modifications thereto.
// Any use, reproduction, disclosure or distribution of this software and related
// documentation without an express license agreement from NVIDIA Corporation is
// strictly prohibited.


/*===========================================================================
                                  GFSDK_ShadowLib.h
=============================================================================

GFSDK_ShadowLib written by Jon Story
-----------------------------------------
This library provides an API for rendering depth / frustum trace / ray trace maps.
It supports a large number of techniques for both spot and direction cascades.

NOTE:
1) The library tracks and restores all graphics API state it uses. With the exception of currently bound RTVs and UAVS, which will be set to NULL.
2) All matrices are expected to be row major.

The typical expected use scenario is:

GameEngine::OnCreateDevice()
{
	GFSDK_ShadowLib_GetVersion()
	GFSDK_ShadowLib_Create()
	GFSDK_ShadowLib_Context::AddBuffer()

	// Optional - if not providing your own shadow maps
	GFSDK_ShadowLib_Context::AddMap()
}

GameEngine::OnResizeWindow()
{
	GFSDK_ShadowLib_Context::RemoveBuffer()
	GFSDK_ShadowLib_Context::AddBuffer()
	GFSDK_ShadowLib_Context::RemoveMap()
	GFSDK_ShadowLib_Context::AddMap()
}

GameEngine::OnRender()
{
	GFSDK_ShadowLib_Context::SetMapRenderParams()
	GFSDK_ShadowLib_Context::UpdateMapBounds()
	GFSDK_ShadowLib_Context::InitializeMapRendering()
	GFSDK_ShadowLib_Context::BeginMapRendering()
	
	// Your light space draw calls go here

	GFSDK_ShadowLib_Context::EndMapRendering()
	
	
	GFSDK_ShadowLib_Context::ClearBuffer()
	GFSDK_ShadowLib_Context::RenderBuffer()
	GFSDK_ShadowLib_Context::FinalizeBuffer()
	
	// Optional - if not wanting to perform a custom modulate with scene color
	GFSDK_ShadowLib_Context::ModulateBuffer()	
}

GameEngine::OnDestroyDevice()
{
	GFSDK_ShadowLib_Context::RemoveMap()			
	GFSDK_ShadowLib_Context::RemoveBuffer()
	GFSDK_ShadowLib_Context::Destroy()
}

ENGINEERING CONTACT
Jon Story (NVIDIA Senior Developer Technology Engineer)
jons@nvidia.com

===========================================================================*/

#pragma once

// defines general GFSDK includes and structs
#include "GFSDK_ShadowLib_Common.h"


/*===========================================================================
Version
===========================================================================*/

// The major, minor and CL version of this library header file
#define GFSDK_SHADOWLIB_MAJOR_VERSION	3
#define GFSDK_SHADOWLIB_MINOR_VERSION	0
#define GFSDK_SHADOWLIB_CHANGE_LIST "$Change: 20516079 $"


/*===========================================================================
Version structure
===========================================================================*/
struct GFSDK_ShadowLib_Version
{
	unsigned int uMajor;
	unsigned int uMinor;
};


/*===========================================================================
Return codes for the lib
===========================================================================*/
enum GFSDK_ShadowLib_Status
{
	// Exceeded primitive limit for FT map rendering
    GFSDK_ShadowLib_Status_Exceeded_Max_Primitive_Count					=  1,	
	// Success
    GFSDK_ShadowLib_Status_Ok											=  0,	
	// Fail
    GFSDK_ShadowLib_Status_Fail											= -1,   
	// Mismatch between header and dll
	GFSDK_ShadowLib_Status_Invalid_Version								= -2,	
	// One or more invalid parameters
	GFSDK_ShadowLib_Status_Invalid_Parameter							= -3,	
	// Failed to allocate a resource
	GFSDK_ShadowLib_Status_Out_Of_Memory								= -4,	
	// The library is being used by another thread
	GFSDK_ShadowLib_Busy												= -5,
	// glewInit failed
	GFSDK_ShadowLib_GLEW_INIT_FAILED									= -6,
	// glewIsSupported("GL_VERSION_3_2") returned false
    GFSDK_ShadowLib_GL_CORE_VERSION_NOT_SUPPORTED						= -7,               
};


/*===========================================================================
Filtering technique types
===========================================================================*/
enum GFSDK_ShadowLib_TechniqueType
{
	// Hard shadow
	GFSDK_ShadowLib_TechniqueType_Hard,
	// PCF shadow
	GFSDK_ShadowLib_TechniqueType_PCF,
	// PCSS+ shadow 
	GFSDK_ShadowLib_TechniqueType_PCSS,
	// Hard ray trace shadow
	GFSDK_ShadowLib_TechniqueType_RT,
	// Hybrid ray traced shadow
	GFSDK_ShadowLib_TechniqueType_HRTS,
	// Hard frustum trace
	GFSDK_ShadowLib_TechniqueType_FT,
	// Hybrid frustum traced shadow
	GFSDK_ShadowLib_TechniqueType_HFTS,
	GFSDK_ShadowLib_TechniqueType_Max,
};


/*===========================================================================
Map render types
===========================================================================*/
enum GFSDK_ShadowLib_MapRenderType
{
	// Depth rendering
	GFSDK_ShadowLib_MapRenderType_Depth,
	// Stores primitives for ray tracing
	GFSDK_ShadowLib_MapRenderType_RT,
	// Frustum traces primitives
	GFSDK_ShadowLib_MapRenderType_FT,
	GFSDK_ShadowLib_MapRenderType_Max,
};


/*===========================================================================
Accepted shadow map view types
===========================================================================*/
enum GFSDK_ShadowLib_ViewType
{
	// Must use GFSDK_ShadowLib_MapType_Texture with GFSDK_ShadowLib_ViewType_Single
	GFSDK_ShadowLib_ViewType_Single		= 1,	
	GFSDK_ShadowLib_ViewType_Cascades_2	= 2,
	GFSDK_ShadowLib_ViewType_Cascades_3	= 3,
	GFSDK_ShadowLib_ViewType_Cascades_4	= 4,
};


/*===========================================================================
Light source types
===========================================================================*/
enum GFSDK_ShadowLib_LightType
{
	GFSDK_ShadowLib_LightType_Directional,
	GFSDK_ShadowLib_LightType_Spot,
	GFSDK_ShadowLib_LightType_Max,
};


/*===========================================================================
Per-sample mode to use
===========================================================================*/
enum GFSDK_ShadowLib_MSAARenderMode
{
	// Every pixel in the shadow buffer is considered to be complex
	// and will be run at sample frequency
	GFSDK_ShadowLib_MSAARenderMode_BruteForce,
	// The application provides a stencil mask and ref values which
	// indicate which pixels are considered complex / simple
	GFSDK_ShadowLib_MSAARenderMode_ComplexPixelMask,
	GFSDK_ShadowLib_MSAARenderMode_Max,
};


/*===========================================================================
Modulate buffer mask
===========================================================================*/
enum GFSDK_ShadowLib_ModulateBufferMask
{
	GFSDK_ShadowLib_ModulateBufferMask_RGB,
	GFSDK_ShadowLib_ModulateBufferMask_R,
	GFSDK_ShadowLib_ModulateBufferMask_G,
	GFSDK_ShadowLib_ModulateBufferMask_B,
	GFSDK_ShadowLib_ModulateBufferMask_A,
	GFSDK_ShadowLib_ModulateBufferMask_Max,
};


/*===========================================================================
debug review type
===========================================================================*/
enum GFSDK_ShadowLib_DebugViewType
{
	GFSDK_ShadowLib_DebugViewType_None,
	GFSDK_ShadowLib_DebugViewType_Cascades,
	GFSDK_ShadowLib_DebugViewType_EyeDepth,
	GFSDK_ShadowLib_DebugViewType_EyeViewZ,
	GFSDK_ShadowLib_DebugViewType_FrustumTraceNodeList,
	GFSDK_ShadowLib_DebugViewType_Max,
};


/*===========================================================================
Conservative raster type
===========================================================================*/
enum GFSDK_ShadowLib_ConservativeRasterType
{
	GFSDK_ShadowLib_ConservativeRasterType_HW,
	GFSDK_ShadowLib_ConservativeRasterType_SW,
	GFSDK_ShadowLib_ConservativeRasterType_None,
	GFSDK_ShadowLib_ConservativeRasterType_Max,
};


/*===========================================================================
Culling mode
===========================================================================*/
enum GFSDK_ShadowLib_CullModeType
{
	GFSDK_ShadowLib_CullModeType_Front,
	GFSDK_ShadowLib_CullModeType_Back,
	GFSDK_ShadowLib_CullModeType_None,
	GFSDK_ShadowLib_CullModeType_Max,
};


/*===========================================================================
Cascaded shadow map type
===========================================================================*/
enum GFSDK_ShadowLib_CascadedShadowMapType
{
	// Frusta supplied by the user (i.e use you pre-existing cascade setup)
	GFSDK_ShadowLib_CascadedShadowMapType_UserDefined,
	// Calculates frusta based upon the supplied linear Z range, but calculates exact XY bounds
	// REQUIRES GPU to CPU read back!
	GFSDK_ShadowLib_CascadedShadowMapType_SampleDistribution,
	GFSDK_ShadowLib_CascadedShadowMapType_Max,
};


/*===========================================================================
Render times
===========================================================================*/
enum GFSDK_ShadowLib_RenderTimesType
{
	GFSDK_ShadowLib_RenderTimesType_SDSM_Bounds,
	GFSDK_ShadowLib_RenderTimesType_DynamicReprojection,
	GFSDK_ShadowLib_RenderTimesType_Frustum_Trace_List,
	GFSDK_ShadowLib_RenderTimesType_Clear_DepthMap,
	GFSDK_ShadowLib_RenderTimesType_Clear_FrustumTraceMap,
	GFSDK_ShadowLib_RenderTimesType_Clear_RayTraceMap,
	GFSDK_ShadowLib_RenderTimesType_DepthMap_0,
	GFSDK_ShadowLib_RenderTimesType_DepthMap_1,
	GFSDK_ShadowLib_RenderTimesType_DepthMap_2,
	GFSDK_ShadowLib_RenderTimesType_DepthMap_3,
	GFSDK_ShadowLib_RenderTimesType_FrustumTraceMap,
	GFSDK_ShadowLib_RenderTimesType_RayTraceMap,
	GFSDK_ShadowLib_RenderTimesType_Shadow_Buffer,
	GFSDK_ShadowLib_RenderTimesType_Max,
};


/*===========================================================================
Depth type
===========================================================================*/
enum GFSDK_ShadowLib_DepthType
{
	// The HW depth buffer ( values in the range 0 to 1 )
	GFSDK_ShadowLib_DepthType_DepthBuffer,
	// Camera/Eye space Z ( Values in the range 0 to FarPlane )
	GFSDK_ShadowLib_DepthType_EyeViewZ,
	GFSDK_ShadowLib_DepthType_Max,
};


/*===========================================================================
Frustum
===========================================================================*/
struct GFSDK_ShadowLib_Frustum
{
	gfsdk_F32		fLeft;
	gfsdk_F32		fRight;
	gfsdk_F32		fTop;
	gfsdk_F32		fBottom;
	gfsdk_F32		fNear;
	gfsdk_F32		fFar;
	// Label this cascade as fully shadowed
	gfsdk_bool		bFullyShadowed;
	// Indciate if this cascade should be cleared
	gfsdk_bool		bClear;
	// Indicate if this is considered a valid cascade
	gfsdk_bool		bValid;
				
	// Defaults
	GFSDK_ShadowLib_Frustum()
	{
		fLeft			= 0;
		fRight			= 0;
		fTop			= 0;
		fBottom			= 0;
		fNear			= 0;
		fFar			= 0;
		bFullyShadowed	= false;
		bClear			= true;
		bValid			= true;
	}
};


/*===========================================================================
Light descriptor
===========================================================================*/
struct GFSDK_ShadowLib_LightDesc
{
	GFSDK_ShadowLib_LightType	eLightType;
	gfsdk_float3				v3LightPos[GFSDK_ShadowLib_ViewType_Cascades_4];
	gfsdk_float3				v3LightLookAt[GFSDK_ShadowLib_ViewType_Cascades_4];
	gfsdk_F32					fLightSize;
	
	// Defaults
	GFSDK_ShadowLib_LightDesc()
	{
		eLightType				= GFSDK_ShadowLib_LightType_Directional;
		v3LightPos[0]			= GFSDK_One_Vector3;
		v3LightPos[1]			= GFSDK_One_Vector3;
		v3LightPos[2]			= GFSDK_One_Vector3;
		v3LightPos[3]			= GFSDK_One_Vector3;
		v3LightLookAt[0]		= GFSDK_Zero_Vector3;
		v3LightLookAt[1]		= GFSDK_Zero_Vector3;
		v3LightLookAt[2]		= GFSDK_Zero_Vector3;
		v3LightLookAt[3]		= GFSDK_Zero_Vector3;
		fLightSize				= 1.0f;
	}
};

/*===========================================================================
Shadow buffer descriptor
===========================================================================*/
struct GFSDK_ShadowLib_BufferDesc
{
	// Should match the size of the input depth buffer
	gfsdk_U32							uResolutionWidth;
	gfsdk_U32							uResolutionHeight;
	// The actual region of the buffer to be used
	gfsdk_U32							uViewportTop;
	gfsdk_U32							uViewportLeft;
	gfsdk_U32							uViewportBottom;
	gfsdk_U32							uViewportRight;
	// Sample count
	gfsdk_U32							uSampleCount;
	
	// Defaults
	GFSDK_ShadowLib_BufferDesc()
	{
		uResolutionWidth	= 0;
		uResolutionHeight	= 0;
		uViewportTop		= 0;
		uViewportLeft		= 0;
		uViewportBottom		= 0;
		uViewportRight		= 0;
		uSampleCount		= 1;
	}
};


/*===========================================================================
Depth buffer descriptor
===========================================================================*/
struct GFSDK_ShadowLib_DepthBufferDesc
{
	GFSDK_ShadowLib_DepthType			eDepthType;
	// The depth SRV from which depth values will be read.
	// This is expected to be where you place an MSAA depth buffer if requiring MSAA rendering,
	// Otherwise it should be the single sample depth buffer.
	GFSDK_ShadowLib_ShaderResourceView	DepthSRV;
	// A single sample version of depth, which is required when using MSAA rendering
	GFSDK_ShadowLib_ShaderResourceView	ResolvedDepthSRV;
	// A read only depth stencil view, which is required for the optimized per sample
	// render path and the stencil channel is expected to contain a mask for complex
	// pixels.
	GFSDK_ShadowLib_DepthStencilView	ReadOnlyDSV;
	// The 2 ref values used during stencil testing to signal which pixels are considered
	// complex and which simple
	gfsdk_U32							uComplexRefValue;
	gfsdk_U32							uSimpleRefValue;
			
	// Defaults
	GFSDK_ShadowLib_DepthBufferDesc()
	{
		eDepthType						= GFSDK_ShadowLib_DepthType_DepthBuffer;
		uComplexRefValue				= 0x01;
		uSimpleRefValue					= 0x00;
	}
};


/*===========================================================================
Ray trace map descriptor
===========================================================================*/
struct GFSDK_ShadowLib_RayTraceMapDesc
{
	// Set to true if you intend to use GFSDK_ShadowLib_TechniqueType_RayTrace, then the additional surfaces
	// required by this technique will be created alongside the shadow map
	gfsdk_bool							bRequirePrimitiveMap;
	gfsdk_U32							uResolutionWidth;
	gfsdk_U32							uResolutionHeight;
	// Total number of primitives for ray tracing. Ignored if bRequirePrimitiveMap == false. 
	// Max = 2000000.
	gfsdk_U32							uMaxNumberOfPrimitives;
	// How many primitives can a single light space texel index.
	// Ignored if bRequirePrimitiveMap == false.
	gfsdk_U32							uMaxNumberOfPrimitivesPerPixel;

	// Defaults
	GFSDK_ShadowLib_RayTraceMapDesc()
	{
		bRequirePrimitiveMap			= false;
		uResolutionWidth				= 0;
		uResolutionHeight				= 0;
		uMaxNumberOfPrimitives			= 0;
		uMaxNumberOfPrimitivesPerPixel	= 0;
	}
};


/*===========================================================================
Frustum trace map descriptor
===========================================================================*/
struct GFSDK_ShadowLib_FrustumTraceMapDesc
{
	// Set to true if you intend to use GFSDK_ShadowLib_TechniqueType_FrustumTrace or
	// GFSDK_ShadowLib_TechniqueType_HFTS, then the additional surfaces
	// required by this technique will be created alongside the shadow map
	gfsdk_bool							bRequireFrustumTraceMap;
	// Resolution of the light space map
	gfsdk_U32							uResolutionWidth;
	gfsdk_U32							uResolutionHeight;
	// Detects high cost areas in light space and dynamically generates new cascade(s) to
	// reproject the hot spot across a larger area in light space.
	// Valid values are 1 or 2.
	gfsdk_U32							uDynamicReprojectionCascades;
	// Granularity in light space at which to detect high cost areas
	// Valid values are 1 to 16.
	gfsdk_U32							uQuantizedListLengthTexelDimension;
		
	// Defaults
	GFSDK_ShadowLib_FrustumTraceMapDesc()
	{
		bRequireFrustumTraceMap				= false;
		uResolutionWidth					= 0;
		uResolutionHeight					= 0;
		uDynamicReprojectionCascades		= 2;
		uQuantizedListLengthTexelDimension	= 4;
	}
};


/*===========================================================================
Map descriptor
===========================================================================*/
struct GFSDK_ShadowLib_MapDesc
{
	gfsdk_U32							uResolutionWidth;
	gfsdk_U32							uResolutionHeight;
	GFSDK_ShadowLib_ViewType			eViewType;
	GFSDK_ShadowLib_RayTraceMapDesc		RayTraceMapDesc;
	GFSDK_ShadowLib_FrustumTraceMapDesc FrustumTraceMapDesc; 
	
	// Defaults
	GFSDK_ShadowLib_MapDesc()
	{
		uResolutionWidth				= 0;
		uResolutionHeight				= 0;
		eViewType						= GFSDK_ShadowLib_ViewType_Single;
	}
};


/*===========================================================================
Ray trace map rendering params (used by GFSDK_ShadowLib_TechniqueType_RT & GFSDK_ShadowLib_TechniqueType_HRTS)
===========================================================================*/
struct GFSDK_ShadowLib_RayTraceMapRenderParams
{
	GFSDK_ShadowLib_ConservativeRasterType			eConservativeRasterType;
	GFSDK_ShadowLib_CullModeType					eCullModeType;
	// Self hit epsilon
	gfsdk_F32										fHitEpsilon;
	
	// Defaults
	GFSDK_ShadowLib_RayTraceMapRenderParams()
	{
		eConservativeRasterType						= GFSDK_ShadowLib_ConservativeRasterType_HW;
		eCullModeType								= GFSDK_ShadowLib_CullModeType_Back;
		fHitEpsilon									= 0.00001f;
	}
};


/*===========================================================================
Frustum trace map rendering params (used by GFSDK_ShadowLib_TechniqueType_FT & GFSDK_ShadowLib_TechniqueType_HFTS)
===========================================================================*/
struct GFSDK_ShadowLib_FrustumTraceMapRenderParams
{
	GFSDK_ShadowLib_ConservativeRasterType			eConservativeRasterType;
	GFSDK_ShadowLib_CullModeType					eCullModeType;
	// Self hit epsilon
	gfsdk_F32										fHitEpsilon;
	gfsdk_bool										bUseDynamicReprojection;
	// List length at which a reprojection will occur 
	gfsdk_U32										uListLengthTolerance;
	// Maximum allowed prims to frustum trace
	gfsdk_U32										uMaxPrimitiveCount;

		
	// Defaults
	GFSDK_ShadowLib_FrustumTraceMapRenderParams()
	{
		eConservativeRasterType						= GFSDK_ShadowLib_ConservativeRasterType_HW;
		eCullModeType								= GFSDK_ShadowLib_CullModeType_None;
		fHitEpsilon									= 0.001f;
		bUseDynamicReprojection						= true;
		uListLengthTolerance						= 1536;
		uMaxPrimitiveCount							= 1000000;
	}
};


/*===========================================================================
PCSS penumbra params
===========================================================================*/
struct GFSDK_ShadowLib_PCSSPenumbraParams
{
	// The World space blocker depth value at which maximum penumbra will occur
	gfsdk_F32	fMaxThreshold;
	// The World space blocker depth value at which penumbra size will no longer grow
	gfsdk_F32	fMaxClamp;
	// The minimum penumbra size, as a percentage of light size - so that one you do not end up with zero
	// filtering at blocker depth zero
	gfsdk_F32	fMinSizePercent[GFSDK_ShadowLib_ViewType_Cascades_4];
	// The percentage of penumbra size below which the fMinWeightExponent function is applied. This stops 
	// the entire shadow from being affected.
	gfsdk_F32	fMinWeightThresholdPercent;
	// Alters the penumbra such that it can be artficially shrunk based upon the blocker depth lerp factor.
	// Used by HRTS and HFTS to stop the PCSS result from appearing outside the RT / FT result
	// close to blocker depth 0.
	// Shrunk_PCSSShadow = saturate( PCSSShadow * lerp( fShiftMin, fShiftMax, saturate( BlockerLerp / fShiftMaxLerpThreshold ) );    
	gfsdk_F32	fShiftMin;
	gfsdk_F32	fShiftMax;
	gfsdk_F32	fShiftMaxLerpThreshold;

	// Defaults
	GFSDK_ShadowLib_PCSSPenumbraParams()
	{
		fMaxThreshold				= 100.0f;
		fMaxClamp					= 100.0f;
		fMinSizePercent[0]			= 1.0f;
		fMinSizePercent[1]			= 1.0f;
		fMinSizePercent[2]			= 1.0f;
		fMinSizePercent[3]			= 1.0f;
		fMinWeightThresholdPercent	= 10.0f;
		fShiftMin					= 2.0f;
		fShiftMax					= 1.0f;
		fShiftMaxLerpThreshold		= 0.01f;
	}
};


/*===========================================================================
Z bias params
===========================================================================*/
struct GFSDK_ShadowLib_ZBiasParams
{
	// Passed to the hw through D3D11_RASTERIZER_DESC.DepthBias  
	gfsdk_S32									iDepthBias;	
	// Passed to the hw through D3D11_RASTERIZER_DESC.SlopeScaledDepthBias
	gfsdk_F32									fSlopeScaledDepthBias;	
	// Distance in eye space biasing, following the formula:
	// TexCoord.z -= ( fDistanceBiasMin + ( fDistanceBiasFactor * pow( g_fEyeFarPlane / fDistanceBiasThreshold, fDistanceBiasPower ) ) );
	gfsdk_F32									fDistanceBiasMin;
	gfsdk_F32									fDistanceBiasFactor;
	gfsdk_F32									fDistanceBiasThreshold;
	gfsdk_F32									fDistanceBiasPower;
	// Use the classic receiver plane bias technique in addition to the above
	gfsdk_bool									bUseReceiverPlaneBias;
	
	// Defaults
	GFSDK_ShadowLib_ZBiasParams()
	{
		iDepthBias									= 1000;
		fSlopeScaledDepthBias						= 8.0f;
		fDistanceBiasMin							= 0.0000001f;
		fDistanceBiasFactor							= 0.0000001f;
		fDistanceBiasThreshold						= 1000.0f;
		fDistanceBiasPower							= 3.0f;
		bUseReceiverPlaneBias						= false;
	}
};


/*===========================================================================
Shadow map rendering params
===========================================================================*/
struct GFSDK_ShadowLib_MapRenderParams
{
	GFSDK_ShadowLib_DepthBufferDesc				DepthBufferDesc;
	GFSDK_ShadowLib_PCSSPenumbraParams			PCSSPenumbraParams;
	GFSDK_ShadowLib_TechniqueType				eTechniqueType;
	gfsdk_float4x4								m4x4EyeViewMatrix;
	gfsdk_float4x4								m4x4EyeProjectionMatrix;
	// World space axis aligned bounding box that encapsulates the shadow map scene geometry for spot lights
	gfsdk_float3								v3WorldSpaceBBox[2];	
	GFSDK_ShadowLib_LightDesc					LightDesc;	
	// Defines the range of cascades (GFSDK_ShadowLib_CascadedShadowMapType_SampleDistribution)
	gfsdk_F32									fCascadeZLinearScale[GFSDK_ShadowLib_ViewType_Cascades_4];
	// Z bias params
	GFSDK_ShadowLib_ZBiasParams					ZBiasParams;
	// Required for GFSDK_ShadowLib_TechniqueType_RayTrace, otherwise ignored
	GFSDK_ShadowLib_RayTraceMapRenderParams		RayTraceMapRenderParams;
	// Required for GFSDK_ShadowLib_TechniqueType_FrustumTrace & GFSDK_ShadowLib_TechniqueType_HFTS, otherwise ignored
	GFSDK_ShadowLib_FrustumTraceMapRenderParams	FrustumTraceMapRenderParams;
	GFSDK_ShadowLib_CascadedShadowMapType		eCascadedShadowMapType;
	// Defines the bounds of cascades (GFSDK_ShadowLib_CascadedShadowMapType_UserDefined)
	GFSDK_ShadowLib_Frustum						UserDefinedFrustum[GFSDK_ShadowLib_ViewType_Cascades_4];
	// Percentage of the view distance (the eye far plane) to consider for cascades
	gfsdk_F32									fCascadeMaxDistancePercent;
	GFSDK_ShadowLib_CullModeType				eCullModeType;
	
	// Defaults
	GFSDK_ShadowLib_MapRenderParams()
	{
		eTechniqueType								= GFSDK_ShadowLib_TechniqueType_PCSS;
		m4x4EyeViewMatrix							= GFSDK_Identity_Matrix;
		m4x4EyeProjectionMatrix						= GFSDK_Identity_Matrix;
		v3WorldSpaceBBox[0] = v3WorldSpaceBBox[1]	= GFSDK_Zero_Vector3;
		fCascadeZLinearScale[0]						= 0.20f;
		fCascadeZLinearScale[1]						= 0.40f;
		fCascadeZLinearScale[2]						= 0.60f;
		fCascadeZLinearScale[3]						= 1.00f;
		eCascadedShadowMapType						= GFSDK_ShadowLib_CascadedShadowMapType_SampleDistribution;
		fCascadeMaxDistancePercent					= 100.0f;
		eCullModeType								= GFSDK_ShadowLib_CullModeType_None;
	}
};


/*===========================================================================
Shadow buffer rendering params
===========================================================================*/
struct GFSDK_ShadowLib_BufferRenderParams
{
	GFSDK_ShadowLib_MSAARenderMode		eMSAARenderMode;
	// Cascade border and blending controls
	gfsdk_F32							fCascadeBorderPercent;
	gfsdk_F32							fCascadeBlendPercent;
	// Visualize various debug views in the shadow buffer (see GFSDK_ShadowLib_DebugViewType)
	GFSDK_ShadowLib_DebugViewType		eDebugViewType;

	// Defaults
	GFSDK_ShadowLib_BufferRenderParams()
	{
		eMSAARenderMode					= GFSDK_ShadowLib_MSAARenderMode_BruteForce;
		fCascadeBorderPercent			= 1.0f;
		fCascadeBlendPercent			= 2.0f;
		eDebugViewType					= GFSDK_ShadowLib_DebugViewType_None;
	}
};


/*===========================================================================
GFSDK_ShadowLib interface
===========================================================================*/
class GFSDK_ShadowLib_Context
{
public:


/*===========================================================================
Call once on device destruction to release the lib.
All resources held by the context will be released/deleted including the context itself
===========================================================================*/
virtual GFSDK_ShadowLib_Status Destroy() = 0;


/*===========================================================================
Call this function to find out what kind of conservative raster is supported
===========================================================================*/
virtual GFSDK_ShadowLib_Status GetConservativeRasterType( 
	GFSDK_ShadowLib_ConservativeRasterType* pConservativeRasterType ) = 0;


/*===========================================================================
Creates various maps, based upon the descriptor, and 
adds it to an internal list of maps. Returns a shadow map
handle to the caller.
===========================================================================*/
virtual GFSDK_ShadowLib_Status AddMap(	
	// IN: Describes the type of shadow map desired (see GFSDK_ShadowLib_MapDesc) 
  	const GFSDK_ShadowLib_MapDesc* __GFSDK_RESTRICT__ const		pShadowMapDesc,
	// IN: Describes the type of shadow buffer desired (see GFSDK_ShadowLib_BufferDesc) 
  	const GFSDK_ShadowLib_BufferDesc* __GFSDK_RESTRICT__ const	pShadowBufferDesc,
	// OUT: A handle to the created shadow map
	GFSDK_ShadowLib_Map** __GFSDK_RESTRICT__ const				ppShadowMapHandle ) = 0;


/*===========================================================================
Removes the shadow map (defined by the provided handle) from the lib's 
internal list of shadow maps
===========================================================================*/
virtual GFSDK_ShadowLib_Status RemoveMap(	
	// IN/OUT: All resources held by the handle will be released/deleted including the handle itself
	GFSDK_ShadowLib_Map** __GFSDK_RESTRICT__ const		ppShadowMapHandle ) = 0;   


/*===========================================================================
Creates a shadow buffer, based upon the descriptor, and 
adds it to an internal list of shadow buffers. Returns a shadow buffer
handle to the caller.
===========================================================================*/
virtual GFSDK_ShadowLib_Status AddBuffer(	
	// IN: Describes the type of shadow buffer desired (see GFSDK_ShadowLib_BufferDesc) 
  	const GFSDK_ShadowLib_BufferDesc* __GFSDK_RESTRICT__ const	pShadowBufferDesc,
	// IN/OUT: A handle to the created shadow buffer
	GFSDK_ShadowLib_Buffer** __GFSDK_RESTRICT__ const			ppShadowBufferHandle ) = 0;


/*===========================================================================
Removes the shadow buffer (defined by the provided handle) from the lib's 
internal list of shadow buffers
===========================================================================*/
virtual GFSDK_ShadowLib_Status RemoveBuffer(	
	// IN/OUT: All resources held by the handle will be released/deleted including the handle itself
	GFSDK_ShadowLib_Buffer** __GFSDK_RESTRICT__ const	ppShadowBufferHandle ) = 0;   


/*===========================================================================
Sets internal rendering parameters, which take immediate effect
===========================================================================*/
virtual GFSDK_ShadowLib_Status SetMapRenderParams(
	// IN: Handle of the shadow map object this update will apply to
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const					pShadowMapHandle,
	// IN: The render params instructing the lib how to render the map (see GFSDK_ShadowLib_MapRenderParams)
	const GFSDK_ShadowLib_MapRenderParams* __GFSDK_RESTRICT__ const	pShadowMapRenderParams ) = 0;


/*===========================================================================
Updates the light space frusta for subsequent rendering
===========================================================================*/
virtual GFSDK_ShadowLib_Status UpdateMapBounds(
	// IN: Handle of the shadow map object this update will apply to
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const					pShadowMapHandle,
	// OUT: Light view matrix calculated by the lib based upon map rednering params.
	// Must point to an array of size 4 of this type. Can't be NULL
	gfsdk_float4x4*													pm4x4LightViewMatrix,
	// OUT: Light projection matrix calculated by the lib based upon map rednering params.
	// Must point to an array of size 4 of this type. Can't be NULL.
	gfsdk_float4x4*													pm4x4LightProjectionMatrix,
	// OUT: Render frustum calculated by the lib based upon map rendering params.
	// Must point to an array of size 4 of this type. Can't be NULL.
	GFSDK_ShadowLib_Frustum*										pRenderFrustum ) = 0;


/*===========================================================================
Must be called once before rendering a map of the type specified.
This clears internal surfaces, and in the case of FT / HFTS performs GPU work to
construct screen to light space mapping lists.
===========================================================================*/
virtual GFSDK_ShadowLib_Status InitializeMapRendering(
	// IN: Handle of the shadow map object this update will apply to
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const					pShadowMapHandle,
	GFSDK_ShadowLib_MapRenderType									eMapRenderType ) = 0;


/*===========================================================================
Sets up all render state for rendering a map of the type provided.
Ideally call once before submitting all primitives for this map.
===========================================================================*/
virtual GFSDK_ShadowLib_Status BeginMapRendering(
	// IN: Handle of the shadow map object this update will apply to
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const					pShadowMapHandle,
	GFSDK_ShadowLib_MapRenderType									eMapRenderType,
	gfsdk_U32														uView ) = 0;


/*===========================================================================
Call to change the culling.
Can be called on a per draw call basis - but sorting by cull type is recommended.
===========================================================================*/
virtual GFSDK_ShadowLib_Status SetMapRenderingCullMode(
	// IN: Handle of the shadow map object this update will apply to
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const					pShadowMapHandle,
	GFSDK_ShadowLib_MapRenderType									eMapRenderType,
	GFSDK_ShadowLib_CullModeType									eCullModeType ) = 0;


/*===========================================================================
You must call this method after every draw call is made to a map of the specified type.
===========================================================================*/
virtual GFSDK_ShadowLib_Status IncrementMapPrimitiveCounter(
	// IN: Handle of the shadow map object this update will apply to
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const					pShadowMapHandle,
	GFSDK_ShadowLib_MapRenderType									eMapRenderType,
	gfsdk_U32														uNumPrimitives ) = 0;


/*===========================================================================
Call when rendering of a map is complete
===========================================================================*/
virtual GFSDK_ShadowLib_Status EndMapRendering(
	// IN: Handle of the shadow map object this update will apply to
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const					pShadowMapHandle,
	GFSDK_ShadowLib_MapRenderType									eMapRenderType,
	gfsdk_U32														uView ) = 0;


/*===========================================================================
Clears the specified shadow buffer
===========================================================================*/
virtual GFSDK_ShadowLib_Status ClearBuffer(	
	// IN: Handle of the shadow buffer you wish to clear
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const	pShadowBufferHandle ) = 0;


/*===========================================================================
Accumulates shadows in the specified buffer (with min blending), 
using the given technique on the given shadow map. This function may be 
called multiple times with different shadow maps, and techniques to 
accumulate shadowed regions of the screen.
===========================================================================*/
virtual GFSDK_ShadowLib_Status RenderBuffer(	
	// IN: Handle of the shadow map you wish to use
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const						pShadowMapHandle,
	// IN: Handle of the shadow buffer you wish to use
	GFSDK_ShadowLib_Buffer* __GFSDK_RESTRICT__ const					pShadowBufferHandle,
	// IN: The render params instructing the lib how to render the buffer (see GFSDK_ShadowLib_BufferRenderParams)
	const GFSDK_ShadowLib_BufferRenderParams* __GFSDK_RESTRICT__ const	pShadowBufferRenderParams ) = 0;


/*===========================================================================
Once done with accumulating shadows in the buffer, call this function to 
finalize the accumulated result and get back the shadow buffer SRV
===========================================================================*/
virtual GFSDK_ShadowLib_Status FinalizeBuffer(	
	// IN: Handle of the shadow buffer you wish to finalize
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const				pShadowBufferHandle,
	// OUT: A platform specific shadow buffer texture - which can the be used to modulate with scene color
	GFSDK_ShadowLib_ShaderResourceView*	__GFSDK_RESTRICT__ const	pShadowBufferSRV ) = 0;


/*===========================================================================
Combines the finalized shadow buffer with the color render target provided, 
using the supplied parameters
===========================================================================*/
virtual GFSDK_ShadowLib_Status ModulateBuffer(	
	// IN: Handle of the shadow buffer you wish to modulate with scene color
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const					pShadowBufferHandle,
	// IN: Platform specific color texture to modulate with
	const GFSDK_ShadowLib_RenderTargetView* __GFSDK_RESTRICT__ const	pColorRTV,
	// IN: The color of the shadow to be used in the modulation
	gfsdk_float3														v3ShadowColor,
	// IN: The write mask to use during modulative blending 
	GFSDK_ShadowLib_ModulateBufferMask									eModulateMask ) = 0;


/*===========================================================================
Get shadow map data
===========================================================================*/
virtual GFSDK_ShadowLib_Status GetMapData(	
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const					pShadowMapHandle,
	GFSDK_ShadowLib_ShaderResourceView*	__GFSDK_RESTRICT__ const	pShadowMapSRV,
	gfsdk_float4x4* __GFSDK_RESTRICT__ const						pLightViewMatrix,								
	gfsdk_float4x4*	__GFSDK_RESTRICT__ const						pLightProjMatrix,
	GFSDK_ShadowLib_Frustum* __GFSDK_RESTRICT__ const				pFrusta ) = 0;


/*===========================================================================
Development mode functions...
===========================================================================*/


/*===========================================================================
Enable perf makers
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeEnablePerfMarkers(	
	gfsdk_bool															bEnable ) = 0;


/*===========================================================================
Enable perf counters
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeEnablePerfCounters(	
	gfsdk_bool															bEnable ) = 0;


/*===========================================================================
Display the shadow map
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeDisplayMap(	
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const					pShadowBufferHandle,
	const GFSDK_ShadowLib_RenderTargetView* __GFSDK_RESTRICT__ const	pRTV,
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const						pShadowMapHandle,
	gfsdk_U32															uMapID,
	// The origin is considered to be: DX = Top Left, GL = Bottom Left 
	gfsdk_U32															uPosX,
	gfsdk_U32															uPosY,
	gfsdk_F32															fScale ) = 0;


/*===========================================================================
Display the shadow map frustum
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeDisplayMapFrustum(	
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const					pShadowBufferHandle,
	const GFSDK_ShadowLib_RenderTargetView* __GFSDK_RESTRICT__ const	pRTV,
	const GFSDK_ShadowLib_DepthStencilView* __GFSDK_RESTRICT__ const	pDSV,
	GFSDK_ShadowLib_Map* __GFSDK_RESTRICT__ const						pShadowMapHandle,
	gfsdk_U32															uMapID,
	gfsdk_float3														v3Color ) = 0;


/*===========================================================================
Display the finalized shadow buffer
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeDisplayBuffer(	
	GFSDK_ShadowLib_Buffer*	__GFSDK_RESTRICT__ const					pShadowBufferHandle,
	const GFSDK_ShadowLib_RenderTargetView* __GFSDK_RESTRICT__ const	pRTV,
	gfsdk_float2														v2Scale,
	const GFSDK_ShadowLib_ScissorRect*	__GFSDK_RESTRICT__ const		pSR ) = 0;


/*===========================================================================
Get time information for the specified ID
===========================================================================*/
virtual GFSDK_ShadowLib_Status DevModeGetTimeStampInfo(
	GFSDK_ShadowLib_RenderTimesType									eID,
	float*															pTimeInMS ) = 0;

};


#ifdef __cplusplus
extern "C" {
#endif


/*===========================================================================
Use this function to get the version of the DLL being used,
you should compare this to the version numbers you see in the header
file. If there is a mismatch please contact devrel.
===========================================================================*/
__GFSDK_SHADOWLIB_EXTERN_INTERFACE__ GFSDK_ShadowLib_GetDLLVersion( 
	// OUT: The version information of the DLL being used
	GFSDK_ShadowLib_Version* __GFSDK_RESTRICT__ const	pVersion );


/*===========================================================================
Call once on device creation to initialize the lib
===========================================================================*/
__GFSDK_SHADOWLIB_EXTERN_INTERFACE__ GFSDK_ShadowLib_Create(	
	// IN: Pass in the version numbers defined in this header file (these will be checked against the DLL version)
	const GFSDK_ShadowLib_Version* __GFSDK_RESTRICT__ const			pVersion, 
	// OUT: The library context which will be used during all susequent calls into the library   
	GFSDK_ShadowLib_Context** __GFSDK_RESTRICT__ const				ppContext,
	// IN: Used to pass in platform specific graphics device/context (see GFSDK_ShadowLib_Common.h)   
	const GFSDK_ShadowLib_DeviceContext* __GFSDK_RESTRICT__ const	pPlatformDevice,					
	// IN: Optionally provide your own custom allocator for the library to use
	gfsdk_new_delete_t*												customAllocator = NULL );


#ifdef __cplusplus
}; //extern "C" {
#endif


/*===========================================================================
EOF
===========================================================================*/