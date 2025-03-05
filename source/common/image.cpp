/******************************************************************************
 * @file	image.cpp
 * @brief
 *****************************************************************************/

#include "common.h"

#if defined(__linux__)
# define BI_RGB        0L
# define BI_RLE8       1L
#endif

#pragma pack(push, 1)
struct bmpfilehead_s {
	word	bfType;
	dword	bfSize;
	word	bfReserved1;
	word	bfReserved2;
	dword	bfOffBits;
};

struct bmpinfohead_s {
	dword	biSize;
	int		biWidth;
	int		biHeight;
	word	biPlanes;
	word	biBitCount;
	dword	biCompression;
	dword	biSizeImage;
	int		biXPelsPerMeter;
	int		biYPelsPerMeter;
	dword	biClrUsed;
	dword	biClrImportant;
};
#pragma pack(pop)

bool Image_Load(const wchar_t *fileName, Image &image) {
	memset(&image, 0, sizeof(image));

	File file;
	if (!File_Read(fileName, file, false)) {
		return false;
	}

	bmpfilehead_s * filehead = (bmpfilehead_s*)file.data;
	bmpinfohead_s * infohead = (bmpinfohead_s*)(filehead + 1);

	if (8 == infohead->biBitCount) {
		if (BI_RGB == infohead->biCompression) { // uncompressed mode
			int src_line_len = (infohead->biWidth + 3) & ~3;

			int valid_size = sizeof(bmpfilehead_s) + sizeof(bmpinfohead_s) + 4 * 256 + src_line_len * infohead->biHeight;
			if (valid_size != (int)file.original_size) {
				File_Free(file);
				return false;
			}

			image.cx = infohead->biWidth;
			image.cy = infohead->biHeight;
			image.bits = 24;
			image.pixels = (byte*)malloc(infohead->biWidth * 3 * infohead->biHeight);

			byte * palette = (byte*)(infohead + 1);
			byte * src = palette + 4 * 256;

			int dst_line_len = infohead->biWidth * 3;

			for (int h = 0; h < infohead->biHeight; ++h) { //OpenGL store bottom row first, same as BMP
				unsigned char * src_line = src + src_line_len * h;
				unsigned char * dst_line = image.pixels + dst_line_len * h;

				for (int w = 0; w < infohead->biWidth; ++w) {
					unsigned char idx = src_line[w];

					unsigned char * src_clr = palette + idx * 4;
					unsigned char * dst_clr = dst_line + w * 3;

					dst_clr[0] = src_clr[2]; //red
					dst_clr[1] = src_clr[1]; //green
					dst_clr[2] = src_clr[0]; //blue
				}
			}
		}
		else if (BI_RLE8 == infohead->biCompression) { // http://msdn.microsoft.com/en-us/library/windows/desktop/dd183383%28v=vs.85%29.aspx
			image.cx = infohead->biWidth;
			image.cy = infohead->biHeight;
			image.bits = 24;
			image.pixels = (byte*)malloc(infohead->biWidth * 3 * infohead->biHeight);

			memset(image.pixels, 255, infohead->biWidth * 3 * infohead->biHeight); //fill to white

			byte * palette = (byte*)(infohead + 1);
			byte * src = palette + 4 * 256;

			int dst_line_len = infohead->biWidth * 3;

			byte * s = src;

			int line = 0;
			int clrxpos = 0;
			byte * dst_line = image.pixels;

			bool breakloop = false;
			while (!breakloop) {
				byte byte1 = s[0];
				byte byte2 = s[1];
				s += 2;

				if (byte1 > 0) {
					byte runlen = byte1;
					byte clridx = byte2;

					byte * src_clr = palette + clridx * 4;

					for (int i = 0; i < (int)runlen; ++i) {
						byte * dst_clr = dst_line + clrxpos * 3;

						dst_clr[0] = src_clr[2]; // red channel
						dst_clr[1] = src_clr[1]; // green channel
						dst_clr[2] = src_clr[0]; // blue channel

						clrxpos++;
					}
				}
				else { // 0 == byte1
					if (byte2 >= 0x03) {
						byte clrlen = byte2;
						for (int i = 0; i < (int)clrlen; ++i) {
							byte clridx = s[i];

							byte * src_clr = palette + clridx * 4;
							byte * dst_clr = dst_line + clrxpos * 3;

							dst_clr[0] = src_clr[2]; // red channel
							dst_clr[1] = src_clr[1]; // green channel
							dst_clr[2] = src_clr[0]; // blue channel

							clrxpos++;
						}
						s += clrlen;
					}
					else {
						switch (byte2) {
						case 0: // end of line
							line++;
							dst_line = (byte*)image.pixels + dst_line_len * line;
							if (clrxpos != infohead->biWidth) {
								free(image.pixels);
								image.pixels = nullptr;
								File_Free(file);
								// SYS_ERROR(L"bad data\n");
								return false;
							}
							clrxpos = 0;
							break;
						case 1: // end of bitmap
							breakloop = true;
							break;
						case 2: // Delta.The 2 bytes following the escape contain unsigned values indicating the horizontal and vertical offsets of the next pixel from the current position.
							free(image.pixels);
							image.pixels = nullptr;
							File_Free(file);
							// SYS_ERROR(L"did not know how to handle delta yet\n");
							return false;
						}
					}
				}
			}
		}
		else {
			File_Free(file);
			// SYS_ERROR(L"unsupported compression mode %d\n", infohead->biCompression);
			return false;
		}

	}
	else if (16 == infohead->biBitCount) {
		int src_line_len = (infohead->biWidth * 2 + 3) & ~3;

		int valid_size = sizeof(bmpfilehead_s) + sizeof(bmpinfohead_s) + src_line_len * infohead->biHeight;
		if (valid_size > (int)file.original_size) {
			File_Free(file);
			// SYS_ERROR(L"bad size\n");
			return false;
		}

		image.cx = infohead->biWidth;
		image.cy = infohead->biHeight;
		image.bits = 24;
		image.pixels = (byte*)malloc(infohead->biWidth * 3 * infohead->biHeight);

		byte * src = (byte*)(infohead + 1);

		int dst_line_len = infohead->biWidth * 3;

		for (int h = 0; h < infohead->biHeight; ++h) { // OpenGL store bottom row first, same as BMP
			byte * src_line = src + src_line_len * h;
			byte * dst_line = (byte*)image.pixels + dst_line_len * h;

			for (int w = 0; w < infohead->biWidth; ++w) {
				uint16_t src_clr = *(uint16_t*)(src_line + w * 2);
				byte * dst_clr = dst_line + w * 3;

				byte src_b = (byte)(src_clr & 0x001f);
				byte src_g = (byte)((src_clr & 0x07e0) >> 5);
				byte src_r = (byte)((src_clr & 0xf800) >> 11);

				dst_clr[0] = src_r; // red channel
				dst_clr[1] = src_g; // green channel
				dst_clr[2] = src_b; // blue channel
			}
		}
	}
	else if (24 == infohead->biBitCount) {
		int src_line_len = (infohead->biWidth * 3 + 3) & ~3;

		int valid_size = sizeof(bmpfilehead_s) + sizeof(bmpinfohead_s) + src_line_len * infohead->biHeight;
		if (valid_size > (int)file.original_size) {
			File_Free(file);
			// SYS_ERROR(L"bad size\n");
			return false;
		}

		image.cx = infohead->biWidth;
		image.cy = infohead->biHeight;
		image.bits = 24;
		image.pixels = (byte*)malloc(infohead->biWidth * 3 * infohead->biHeight);

		byte * src = (byte*)(infohead + 1);

		int dst_line_len = infohead->biWidth * 3;

		for (int h = 0; h < infohead->biHeight; ++h) { // OpenGL store bottom row first, same as BMP
			byte * src_line = src + src_line_len * h;
			byte * dst_line = (byte*)image.pixels + dst_line_len * h;

			for (int w = 0; w < infohead->biWidth; ++w) {
				byte * src_clr = src_line + w * 3;
				byte * dst_clr = dst_line + w * 3;

				dst_clr[0] = src_clr[2]; // red channel
				dst_clr[1] = src_clr[1]; // green channel
				dst_clr[2] = src_clr[0]; // blue channel
			}
		}
	}
	else if (32 == infohead->biBitCount) {
		int src_line_len = infohead->biWidth * 4;

		int valid_size = sizeof(bmpfilehead_s) + sizeof(bmpinfohead_s) + src_line_len * infohead->biHeight;
		if (valid_size > (int)file.original_size) {
			File_Free(file);
			// SYS_ERROR(L"bad size\n");
			return false;
		}

		image.cx = infohead->biWidth;
		image.cy = infohead->biHeight;
		image.bits = 24;
		image.pixels = (byte*)malloc(infohead->biWidth * 4 * infohead->biHeight);

		byte * src = (byte*)(infohead + 1);

		int dst_line_len = infohead->biWidth * 4;

		for (int h = 0; h < infohead->biHeight; ++h) { // OpenGL store bottom row first, same as BMP
			byte * src_line = src + src_line_len * h;
			byte * dst_line = (byte*)image.pixels + dst_line_len * h;

			for (int w = 0; w < infohead->biWidth; ++w) {
				byte * src_clr = src_line + w * 4;
				byte * dst_clr = dst_line + w * 4;
				dst_clr[0] = src_clr[2]; // red channel
				dst_clr[1] = src_clr[1]; // green channel
				dst_clr[2] = src_clr[0]; // blue channel
				dst_clr[3] = src_clr[3]; // alpha channel
			}
		}
	}

	File_Free(file);
	return true;
}

bool Image_Save(const wchar_t *fileName, const Image &image) {
	if (image.bits != 24 && image.bits != 32) {
		return false;
	}

	FILE * file = nullptr;
	_wfopen_s(&file, fileName, L"wb");
	if (!file) {
		return false;
	}

	int bytesPerPixel = image.bits >> 3;
	int srclinewidth = image.cx * bytesPerPixel;
	int dstlinewidth = image.cx * 3;
	dstlinewidth = (dstlinewidth + 3) & ~3;

	bmpfilehead_s filehead;

	filehead.bfType = 0x4d42;
	filehead.bfOffBits = sizeof(bmpfilehead_s) + sizeof(bmpinfohead_s);
	filehead.bfSize = sizeof(bmpfilehead_s) + sizeof(bmpinfohead_s) + dstlinewidth * image.cy;
	filehead.bfReserved1 = 0;
	filehead.bfReserved2 = 0;

	bmpinfohead_s infohead;

	infohead.biBitCount = image.bits;
	infohead.biClrImportant = 0;
	infohead.biClrUsed = 0;
	infohead.biCompression = 0;
	infohead.biHeight = image.cy;
	infohead.biPlanes = 1;
	infohead.biSize = sizeof(infohead);
	infohead.biSizeImage = 0;
	infohead.biWidth = image.cx;
	infohead.biXPelsPerMeter = 0;
	infohead.biYPelsPerMeter = 0;

	byte * dst = (byte*)malloc(image.cy * dstlinewidth);
	for (int h = 0; h < image.cy; ++h) {
		byte * dstline = dst + h * dstlinewidth;
		const byte * srcline = image.pixels + h * srclinewidth;
		for (int w = 0; w < image.cx; ++w) {
			unsigned char * dstclr = dstline + w * bytesPerPixel;
			const unsigned char * srcclr = srcline + w * bytesPerPixel;
			dstclr[0] = srcclr[2]; // B
			dstclr[1] = srcclr[1]; // G
			dstclr[2] = srcclr[0]; // R
			if (bytesPerPixel > 3) {
				dstclr[3] = srcclr[3]; // A
			}
		}
	}

	fwrite(&filehead, sizeof(filehead), 1, file);
	fwrite(&infohead, sizeof(infohead), 1, file);
	fwrite(dst, image.cy * dstlinewidth, 1, file);
	fclose(file);
	file = nullptr;

	free(dst);
	dst = nullptr;

	return true;
}

void Image_Free(Image &image) {
	if (image.pixels) {
		free(image.pixels);
		image.pixels = nullptr;
	}
	image.cx = image.cy = image.bits = 0;
}
