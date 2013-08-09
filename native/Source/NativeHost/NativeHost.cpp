#include <Bacon/Bacon.h>

#include <cassert>
#include <cstdio>
using namespace std;

int g_Kitten;
int g_Font;
int g_Buffer;
int g_Glyph;
void Tick()
{
	static float time = 0.f;
	time += 0.01f;
	Bacon_Clear(0.3f, 0.3f, 0.3f, 1.f);
	Bacon_SetColor(1, 1, 1, 1);
	
	int w, h;
	Bacon_GetImageSize(g_Glyph, &w, &h);
	Bacon_DrawImage(g_Glyph, 10.f, 10.f, 10.f + w, 10.f + h);

    int kw, kh;
    Bacon_GetImageSize(g_Kitten, &kw, &kh);

    int keyState;
    Bacon_GetKeyState(Key_Space, &keyState);
    if (keyState)
        Bacon_DrawImage(g_Kitten, 100.f, 100.f, 100.f + kw, 100.f + kh);
}

void OnControllerConnected(int controller, int connected)
{
	int mask;
	Bacon_GetControllerPropertyInt(controller, Bacon_Controller_Property_SupportedButtonsMask, &mask);
	if (!connected)
		return;
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
}

int main(int argc, const char * argv[])
{
	Bacon_Init();
    Bacon_SetKeyEventHandler(OnKey);
	Bacon_SetControllerButtonEventHandler(OnControllerButton);
	Bacon_SetControllerAxisEventHandler(OnControllerAxis);
	Bacon_SetControllerConnectedEventHandler(OnControllerConnected);
	
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
	
	float offsetX, offsetY, advance;
	error = Bacon_GetGlyph(g_Font, 64.f, 'A', &g_Glyph, &offsetX, &offsetY, &advance);
	
	Bacon_LoadImage(&g_Kitten, "res/kitten.png", Bacon_ImageFlags_PremultiplyAlpha | Bacon_ImageFlags_DiscardBitmap);
	
	Bacon_SetTickCallback(Tick);
	Bacon_Run();
    return 0;
}

