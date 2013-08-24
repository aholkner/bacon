#include <Bacon/Bacon.h>

#include <cassert>
#include <cstdio>
using namespace std;

int g_Kitten;
int g_Font;
int g_Buffer;

struct Glyph
{
	Glyph() : m_Image(0) { }
	int m_Image;
	float m_OffsetX, m_OffsetY;
	float m_Advance;
};
Glyph g_Glyphs[26];

int s_NumGlyphs = 1;

void Tick()
{
	static float time = 0.f;
	time += 0.01f;
	Bacon_Clear(0.3f, 0.3f, 0.3f, 1.f);
	Bacon_SetColor(1, 1, 1, 1);
	
	float x = 10.f, y = 82.f;
	for (int i = 0; i < s_NumGlyphs; ++i)
	{
		if (!g_Glyphs[i].m_Image)
			break;
		
		int w, h;
		Bacon_GetImageSize(g_Glyphs[i].m_Image, &w, &h);
		Bacon_DrawImage(g_Glyphs[i].m_Image,
						x + g_Glyphs[i].m_OffsetX,
						y - g_Glyphs[i].m_OffsetY,
						x + g_Glyphs[i].m_OffsetX + w,
						y - g_Glyphs[i].m_OffsetY + h);
		x += g_Glyphs[i].m_Advance;
	}

    int kw, kh;
    Bacon_GetImageSize(g_Kitten, &kw, &kh);
    Bacon_DrawImage(g_Kitten, 100.f, 100.f, 100.f + kw, 100.f + kh);
	
	Bacon_SetBlending(Bacon_Blend_One, Bacon_Blend_One);
	Bacon_SetColor(1, 0, 0, 1);
	Bacon_FillRect(25, 225, 50, 250);
	Bacon_SetColor(0, 1, 1, 1);
	Bacon_DrawRect(25, 225, 50, 250);
	
    int keyState;
    Bacon_GetKeyState(Key_Space, &keyState);
    if (keyState)
		Bacon_DrawDebugOverlay();
}

void OnControllerConnected(int controller, int connected)
{
    if (connected)
        printf("connected: %d\n", controller);
    else
        printf("disconnected: %d\n", controller);
}

void OnControllerButton(int controller, int button, int value)
{
	printf("%d %d %d\n", controller, button, value);
}

void OnControllerAxis(int controller, int axis, float value)
{
	printf("%d %d %f\n", controller, axis, value);
}

void OnKey(int key, int value)
{
    if (key == 'f' && value)
    {
        static bool fullscreen = false;
        fullscreen = !fullscreen;
        Bacon_SetWindowFullscreen(fullscreen);
    }
	if (key == 'z' && value)
	{
		static float s = 64.f;
		Glyph& glyph = g_Glyphs[0];
		Bacon_UnloadImage(glyph.m_Image);
		Bacon_GetGlyph(g_Font, s, 'a', &glyph.m_Image, &glyph.m_OffsetX, &glyph.m_OffsetY, &glyph.m_Advance);
		s += 10.f;
	}
}

void OnLogMessage(int level, const char* message)
{
    printf("%d: %s\n", level, message);
}

int main(int argc, const char * argv[])
{
	Bacon_Init();
    Bacon_SetLogCallback(OnLogMessage);
    Bacon_SetKeyEventHandler(OnKey);
	Bacon_SetControllerButtonEventHandler(OnControllerButton);
	Bacon_SetControllerAxisEventHandler(OnControllerAxis);
	Bacon_SetControllerConnectedEventHandler(OnControllerConnected);
	Bacon_SetWindowResizable(true);
	Bacon_SetWindowSize(512, 512);

	int error;

	int sound;
	error = Bacon_LoadSound(&sound, "res/PowerChorus2.ogg", Bacon_SoundFlags_FormatOgg | Bacon_SoundFlags_Stream);
	assert(!error);
	
	int voice;
	error = Bacon_CreateVoice(&voice, sound, Bacon_VoiceFlags_Loop);
	assert(!error);
	//error = Bacon_PlayVoice(voice);
	assert(!error);

	error = Bacon_LoadFont(&g_Font, "res/DejaVuSans.ttf");
	assert(!error);
	
	float ascent, descent;
	error = Bacon_GetFontMetrics(g_Font, 64.f, &ascent, &descent);

	int g = 0;
	for (char c : "a")
	{
		if (!c)
			break;
		Glyph& glyph = g_Glyphs[g++];
		error = Bacon_GetGlyph(g_Font, 32, c, &glyph.m_Image, &glyph.m_OffsetX, &glyph.m_OffsetY, &glyph.m_Advance);
	}
	
	Bacon_LoadImage(&g_Kitten, "res/ball.png", Bacon_ImageFlags_PremultiplyAlpha | Bacon_ImageFlags_DiscardBitmap | Bacon_ImageFlags_Atlas);
	
	Bacon_SetTickCallback(Tick);
	Bacon_Run();
    return 0;
}

