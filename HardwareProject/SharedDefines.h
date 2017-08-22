// This file defines how code can be used with C++ and HLSL
#ifndef SHARED_DEFINES_H
#define SHARED_DEFINES_H

#ifdef __cplusplus
	#pragma once
	#include <DirectXMath.h>
	// lets us ensure constant buffers and their variables are 16byte aligned to HLSL 4-float registers
	#define _regAlign __declspec(align(16))

	typedef _regAlign unsigned int			uint;
	typedef _regAlign DirectX::XMFLOAT2		float2;
	typedef _regAlign DirectX::XMFLOAT3		float3;
	typedef _regAlign DirectX::XMFLOAT4		float4;
	typedef _regAlign DirectX::XMFLOAT4X4	float4x4;
	typedef _regAlign DirectX::XMMATRIX		matrix;
	// allows us to attach semantics to HLSL variables without bugging C++
	#define SEMANTIC(s_name) /* : s_name */

	#define CONSTANT_BUFFER_BEGIN(cb_name, reg) struct _regAlign cb_name {
	#define CONSTANT_BUFFER_END };

	#define VERTEX_BUFFER_BEGIN(vb_name) struct _regAlign vb_name{
	#define VERTEX_BUFFER_END };
#else
	//#pragma pack_matrix(row_major)

	// lets us ensure constant buffers and variables are 16byte aligned (HLSL will do this for us anyway)
	#define _regAlign /**/
	// allows us to attach semantics to HLSL variables without bugging C++
	#define SEMANTIC(s_name) : s_name
	// In HLSL constant buffers will be identified by their name and sequential ordering
	#define CONSTANT_BUFFER_BEGIN(cb_name, reg) cbuffer cb_name : register(reg){
	//#define CONSTANT_BUFFER_BEGIN(cb_name) cbuffer cb_name {
	#define CONSTANT_BUFFER_END }

	#define VERTEX_BUFFER_BEGIN(vb_name) struct vb_name{
	#define VERTEX_BUFFER_END };

#endif

// <Constant Buffers>
CONSTANT_BUFFER_BEGIN(cbMirror_Perspective, b0)
	matrix model;
	matrix view;
	matrix projection;
	uint SkyboxToggle;
CONSTANT_BUFFER_END

CONSTANT_BUFFER_BEGIN(cbMirror_lighting, b1)
	float3 direction;
	float4 ambient;
	float4 difuse;
CONSTANT_BUFFER_END

// </Constant Buffers>


// <Vertex Buffers>
VERTEX_BUFFER_BEGIN(VERTEX_3D)
	float3 pos SEMANTIC(POSITION);
	float2 uv SEMANTIC(TEXCOORD);
VERTEX_BUFFER_END

VERTEX_BUFFER_BEGIN(VERTEX_3D_NORM)
	float3 pos SEMANTIC(POSITION);
	float2 uv SEMANTIC(TEXCOORD);
	float3 norm SEMANTIC(NORMAL);
VERTEX_BUFFER_END
// </Vertex Buffers>

#endif //SHARED_DEFINES_H