#include "Bacon.h"
#include "BaconInternal.h"
#include "HandleArray.h"
using namespace Bacon;

#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <FreeImage/FreeImage.h>

using namespace std;

namespace {
	
	const int Dpi = 96;

	struct Font
	{
		FT_Face m_Face;
	};
	
	struct Impl
	{
		FT_Library m_Library;
		
		HandleArray<Font> m_Fonts;
	};
	static Impl* s_Impl = nullptr;
	
}

void Fonts_Init()
{
	s_Impl = new Impl;
	FT_Init_FreeType(&s_Impl->m_Library);
	s_Impl->m_Fonts.Reserve(16);
}

void Fonts_Shutdown()
{
	FT_Done_FreeType(s_Impl->m_Library);
	delete s_Impl;
}

int Bacon_LoadFont(int* outHandle, const char* path)
{
	if (!outHandle || !path)
		return Bacon_Error_InvalidArgument;
	
	FT_Face face;
	FT_Error error = FT_New_Face(s_Impl->m_Library, path, 0, &face);
	if (error == FT_Err_Unknown_File_Format)
		return Bacon_Error_UnsupportedFormat;
	else if (error != FT_Err_Ok)
		return Bacon_Error_Unknown;

	*outHandle = s_Impl->m_Fonts.Alloc();
	Font* font = s_Impl->m_Fonts.Get(*outHandle);
	font->m_Face = face;

	return Bacon_Error_None;
}

int Bacon_UnloadFont(int handle)
{
	Font* font = s_Impl->m_Fonts.Get(handle);
	if (!font)
		return Bacon_Error_InvalidHandle;
	
	FT_Done_Face(font->m_Face);
	s_Impl->m_Fonts.Free(handle);
	
	return Bacon_Error_None;
}

int Bacon_GetFontMetrics(int handle, float size, int* outAscent, int* outDescent)
{
	if (!outAscent || !outDescent || size <= 0.f)
		return Bacon_Error_InvalidArgument;
	
	Font* font = s_Impl->m_Fonts.Get(handle);
	if (!font)
		return Bacon_Error_InvalidHandle;
	
	FT_Face face = font->m_Face;
	if (FT_Set_Char_Size(face, 0, (int)(size * 64), Dpi, Dpi))
		return Bacon_Error_InvalidFontSize;
	
	*outAscent = (int)(face->size->metrics.ascender / 64);
	*outDescent = (int)(face->size->metrics.descender / 64);
	
	return Bacon_Error_None;
}

int Bacon_GetGlyph(int handle, float size, int character, int flags, int* outImage,
			 int* outOffsetX, int* outOffsetY, int* outAdvance)
{
	if (!outImage || !outOffsetX || !outOffsetY || !outAdvance)
		return Bacon_Error_InvalidArgument;
	
	Font* font = s_Impl->m_Fonts.Get(handle);
	if (!font)
		return Bacon_Error_InvalidHandle;
	
	FT_Face face = font->m_Face;
	if (FT_Set_Char_Size(face, 0, (int)(size * 64), Dpi, Dpi))
		return Bacon_Error_InvalidFontSize;

	int loadFlags = FT_LOAD_RENDER | FT_LOAD_COLOR;
	if (flags & Bacon_FontFlags_LightHinting)
		loadFlags |= FT_LOAD_TARGET_LIGHT;
	FT_Load_Char(face, character, loadFlags);

	if (face->glyph->bitmap.width && face->glyph->bitmap.rows)
	{
		int width = face->glyph->bitmap.width;
		int height = face->glyph->bitmap.rows;
		if (Bacon_CreateImage(outImage, width, height, Bacon_ImageFlags_Atlas | Bacon_ImageFlags_DiscardBitmap))
			return Bacon_Error_Unknown;
		
		int bpp = 0;
		switch (face->glyph->bitmap.pixel_mode)
		{
			case FT_PIXEL_MODE_MONO:
				bpp = 1;
				break;
			case FT_PIXEL_MODE_GRAY:
				bpp = 8;
				break;
			case FT_PIXEL_MODE_BGRA:
				bpp = 32;
				break;
			default:
				return Bacon_Error_Unknown;
		}
		
		FIBITMAP* bmp = FreeImage_ConvertFromRawBits(face->glyph->bitmap.buffer,
													 width, height, face->glyph->bitmap.pitch, bpp, 0, 0, 0, TRUE);
		if (bpp == 1)
		{
			RGBQUAD *palette = FreeImage_GetPalette(bmp);
			memset(&palette[0], 0, sizeof(RGBQUAD));
			memset(&palette[1], 255, sizeof(RGBQUAD));
		}
		FIBITMAP* bmp32 = FreeImage_ConvertTo32Bits(bmp);
		FreeImage_SetChannel(bmp32, bmp, FICC_ALPHA);
		FreeImage_Unload(bmp);
		Graphics_SetImageBitmap(*outImage, bmp32);
		
		*outOffsetX = (float)face->glyph->bitmap_left;
		*outOffsetY = (float)face->glyph->bitmap_top;
	}
	else
	{
		*outImage = 0;
	}
	
	*outAdvance = (int)(face->glyph->advance.x / 64);
	
	return Bacon_Error_None;
}