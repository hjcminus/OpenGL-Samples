//2016-01-20 Wed.

#include "Precompiled.h"
#include <mmsystem.h>

#pragma comment(lib, "winmm")
/*
================================================================================
Common
================================================================================
*/
const wchar_t * DATA_DIR;
const wchar_t * TEXTURE_DIR;
const wchar_t * MODEL_DIR;
const wchar_t * SHADER_DIR;

static void ExtractParentDir(const wchar_t *path, wchar_t *dir) {
	dir[0] = 0;
	int l = (int)wcslen(path);
	for (int i = l - 1; i >= 0; --i) {
		if (path[i] == L'\\' || path[i] == L'/') {
			int fileDirElementCount = i;
			memcpy(dir, path, fileDirElementCount * sizeof(wchar_t));
			dir[fileDirElementCount] = 0;
			break;
		}
	}
}

void Com_Init() {
	timeBeginPeriod(1);

	static wchar_t dataDir[MAX_PATH];
	static wchar_t textureDir[MAX_PATH];
	static wchar_t modelDir[MAX_PATH];
	static wchar_t shaderDir[MAX_PATH];

	wchar_t buffer1[MAX_PATH], buffer2[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), buffer1, MAX_PATH);

	ExtractParentDir(buffer1, buffer2); //.../build/x64/Debug
	ExtractParentDir(buffer2, buffer1); //.../build/x64
	ExtractParentDir(buffer1, buffer2); //.../build
	ExtractParentDir(buffer2, buffer1);

	wsprintf(dataDir,    L"%s/data",          buffer1);
	wsprintf(textureDir, L"%s/data/textures", buffer1);
	wsprintf(modelDir, L"%s/data/models", buffer1);
	wsprintf(shaderDir, L"%s/data/shaders", buffer1);

	DATA_DIR    = dataDir;
	TEXTURE_DIR = textureDir;
	MODEL_DIR   = modelDir;
	SHADER_DIR  = shaderDir;
}

void Com_Shutdown() {
	timeEndPeriod(1);
}

uint32 Com_GetMS() {
	return timeGetTime();
}

/*
================================================================================
math
================================================================================
*/
int Math_IsPowerOf2(int n) {
	return n && !(n & (n - 1));
}

float Math_DegToRad(float d) {
	return d * MATH_PI / 180.0f;
}

float Math_RadToDeg(float r) {
	return r * 180.0f / MATH_PI;
}

/*
================================================================================
file
================================================================================
*/
bool File_Read(const wchar_t *fileName, File &file) {
	memset(&file, 0, sizeof(file));

	FILE * f = nullptr;
	_wfopen_s(&f, fileName, L"rb");
	if (!f) {
		return false;
	}

	fseek(f, 0, SEEK_END);
	file.size = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (file.size) {
		file.data = (byte*)malloc(file.size);
		fread(file.data, 1, file.size, f);
		fclose(f);
		f = nullptr;
	}

	return file.size > 0;
}

void File_Free(File &file) {
	if (file.data) {
		free(file.data);
		file.data = nullptr;
	}
	file.size = 0;
}
