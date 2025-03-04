//2016-01-18 Mon.

#pragma once

typedef unsigned __int32 uint32;
typedef float    vec2_t[2];
typedef float    vec3_t[3];
typedef float    vec4_t[4];
typedef vec2_t   float2;
typedef vec3_t   float3;
typedef vec4_t   float4;

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned int   dword;

typedef unsigned short uint16;

/*
================================================================================
Common
================================================================================
*/
extern const wchar_t * DATA_DIR;
extern const wchar_t * TEXTURE_DIR;
extern const wchar_t * MODEL_DIR;
extern const wchar_t * SHADER_DIR;

void   Com_Init();
void   Com_Shutdown();
uint32 Com_GetMS();

/*
================================================================================
math
================================================================================
*/
#define MATH_PI            3.141592654f   //same as D3DX_PI

int    Math_IsPowerOf2(int n);
float  Math_DegToRad(float d);
float  Math_RadToDeg(float r);

/*
================================================================================
file
================================================================================
*/

struct File {
	size_t size;
	byte * data;
};

bool   File_Read(const wchar_t *fileName, File &file);
void   File_Free(File &file);
