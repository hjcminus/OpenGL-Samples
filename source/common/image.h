//2016-01-20 Wed.

#pragma once

struct Image {
	int    cx;
	int    cy;
	int    bits; //24 / 32: rgb/rgba
	byte * pixels;
};

bool Image_Load(const wchar_t *fileName, Image &image);
bool Image_Save(const wchar_t *fileName, const Image &image);
void Image_Free(Image &image);
