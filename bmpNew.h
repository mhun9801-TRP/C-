#pragma once
#include <stdio.h>
#include <fstream> // suh
using namespace std; // suh
#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER_ {
	unsigned short		bfType;
	unsigned long		bfSize;
	unsigned short		bfReserved1;
	unsigned short		bfReserved2;
	unsigned long		bfOffBits;
} BITMAPFILEHEADER_;

typedef struct tagBITMAPINFOHEADER_ {
	unsigned long       biSize;
	long				biWidth;
	long				biHeight;
	unsigned short      biPlanes;
	unsigned short      biBitCount;
	unsigned long       biCompression;
	unsigned long       biSizeImage;
	long				biXPelsPerMeter;
	long				biYPelsPerMeter;
	unsigned long       biClrUsed;
	unsigned long       biClrImportant;
} BITMAPINFOHEADER_;

typedef struct tagRGBQUAD_ {
	unsigned char    rgbBlue;
	unsigned char    rgbGreen;
	unsigned char    rgbRed;
	unsigned char    rgbReserved;
} RGBQUAD_;
#pragma pack(pop)

#define BI_RGB_        0L

int GetBmp24Size(int nW, int nH) {
	return (nW * 3 + 3) / 4 * 4 * nH;
}

int GetBmp24Pos(int nW, int nH, int x, int y) {
	return (nW * 3 + 3) / 4 * 4 * (nH - 1 - y) + x * 3;
}

bool WriteBmp(const char *FileName, unsigned char *Image1D, int nW, int nH) {
	unsigned long dwBitsSize;
	unsigned long size;

	size = GetBmp24Size(nW, nH);
	dwBitsSize = sizeof(BITMAPFILEHEADER_) + sizeof(BITMAPINFOHEADER_) + size;

	// 헤더 정보 저장
	BITMAPINFOHEADER_ bmiHeader;
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER_);
	bmiHeader.biWidth = nW;
	bmiHeader.biHeight = nH;
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 24;
	bmiHeader.biCompression = BI_RGB_;
	bmiHeader.biSizeImage = size;
	bmiHeader.biXPelsPerMeter = 2000;
	bmiHeader.biYPelsPerMeter = 2000;
	bmiHeader.biClrUsed = 0;
	bmiHeader.biClrImportant = 0;

	BITMAPFILEHEADER_ bmiFileHeader;
	bmiFileHeader.bfType = 'M' * 0x0100 + 'B';
	bmiFileHeader.bfSize = dwBitsSize;
	bmiFileHeader.bfReserved1 = 0;
	bmiFileHeader.bfReserved2 = 0;

	bmiFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER_) + sizeof(BITMAPINFOHEADER_);

	//FILE *fp;

	//fopen_s(&fp, FileName, "wb");
	ofstream fp(FileName, ios::binary | ios::out);

	if (!fp) return false;

	//fwrite(&bmiFileHeader, sizeof(BITMAPFILEHEADER_), 1, fp);
	//fwrite(&bmiHeader, sizeof(BITMAPINFOHEADER_), 1, fp);
	//fwrite(Image1D, sizeof(unsigned char), size, fp);
	fp.write((char*)&bmiFileHeader, sizeof(BITMAPFILEHEADER_)); // suh
	fp.write((char*)&bmiHeader, sizeof(BITMAPINFOHEADER_));// suh
	fp.write((char*)Image1D, sizeof(unsigned char)*size);// suh

	//fclose(fp);
	fp.close(); // suh

	return true;
}

unsigned char *ReadBmp(const char *FileName, int *pW, int *pH) {
	BITMAPFILEHEADER_ bmiFileHeader;
	BITMAPINFOHEADER_ bmiHeader;

	//FILE *fp;
	int nColors;

	//fopen_s(&fp, FileName, "rb");
	ifstream fp(FileName, ios::binary | ios::in); // suh
	if (!fp) return NULL;

	//fread(&bmiFileHeader, sizeof(BITMAPFILEHEADER_), 1, fp);
	fp.read((char*)&bmiFileHeader, sizeof(BITMAPFILEHEADER_)); // suh
	if (bmiFileHeader.bfType != 'M' * 0x0100 + 'B') {
		//fclose(fp);
		fp.close(); // suh
		return NULL;
	}

	//fread(&bmiHeader, sizeof(BITMAPINFOHEADER_), 1, fp);
	fp.read((char*)&bmiHeader, sizeof(BITMAPINFOHEADER_)); // suh
	if (bmiHeader.biCompression != BI_RGB_) {
		//fclose(fp);
		fp.close(); // suh
		return NULL;
	}

	if (bmiHeader.biClrUsed == 0)
		nColors = 1 << bmiHeader.biBitCount;
	else
		nColors = bmiHeader.biClrUsed;

	RGBQUAD_ *Palette = NULL;

	switch (bmiHeader.biBitCount) {
	case 24:
		break;

	case 1:
	case 4:
	case 8:
		Palette = new RGBQUAD_[nColors];

		int i;
		for (i = 0; i < nColors; i++) {
			unsigned char r, g, b, temp;

			//fread(&b, sizeof(unsigned char), 1, fp);
			//fread(&g, sizeof(unsigned char), 1, fp);
			//fread(&r, sizeof(unsigned char), 1, fp);
			//fread(&temp, sizeof(unsigned char), 1, fp);
			fp.read((char*)&b, sizeof(unsigned char)); // suh
			fp.read((char*)&g, sizeof(unsigned char)); // suh
			fp.read((char*)&r, sizeof(unsigned char)); // suh
			fp.read((char*)&temp, sizeof(unsigned char)); // suh

			Palette[i].rgbRed = r;
			Palette[i].rgbGreen = g;
			Palette[i].rgbBlue = b;
		}
		break;
	}

	//fseek(fp, bmiFileHeader.bfOffBits, SEEK_SET);
	fp.close(); // suh
	ifstream fp2(FileName, ios::binary | ios::in); // suh

	*pW = bmiHeader.biWidth;
	*pH = bmiHeader.biHeight;

	unsigned char *Image1D = NULL;

	Image1D = new unsigned char[GetBmp24Size(*pW, *pH)];
	fp2.read((char*)Image1D,bmiFileHeader.bfOffBits); // suh

	if (bmiHeader.biBitCount == 24)
		fp2.read((char*)Image1D, *pH * ((*pW * 3 + 3) / 4 * 4)); // suh
		//fread(Image1D, sizeof(unsigned char), *pH*((*pW * 3 + 3) / 4 * 4), fp);

	long Row, Col;

	for (Row = 0; Row < bmiHeader.biHeight; Row++)
	{
		if (bmiHeader.biBitCount != 24)
		{
			int BitCount = 0;
			unsigned int mask = (1 << bmiHeader.biBitCount) - 1;

			unsigned char ReadByte = 0;
			int ReadByteCnt = 0;

			for (Col = 0; Col < *pW; Col++)
			{
				int PaletteIndex = 0;

				if (BitCount <= 0) {
					BitCount = 8;
					//fread(&ReadByte, sizeof(unsigned char), 1, fp);
					fp2.read((char*)&ReadByte, sizeof(unsigned char));//suh
					ReadByteCnt++;
				}

				BitCount -= bmiHeader.biBitCount;

				PaletteIndex = (ReadByte >> BitCount) & mask;

				int Pos;

				Pos = (((*pW * 3 + 3) / 4 * 4) * Row) + Col * 3;
				Image1D[Pos++] = Palette[PaletteIndex].rgbBlue;
				Image1D[Pos++] = Palette[PaletteIndex].rgbGreen;
				Image1D[Pos] = Palette[PaletteIndex].rgbRed;
			}

			while (ReadByteCnt & 3)
			{
				char temp;
				//fread(&temp, sizeof(char), 1, fp);
				fp2.read((char*)&temp, sizeof(char));//suh
				ReadByteCnt++;
			}
		}
	}

	if (Palette) delete[] Palette;

	//fclose(fp);
	fp2.close(); // suh

	return Image1D;
}
