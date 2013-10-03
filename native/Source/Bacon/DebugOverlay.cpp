#include "Bacon.h"
#include "BaconInternal.h"

#include <string>
#include <vector>
using namespace std;

struct Glyph
{
    Glyph()
        : m_Image(0)
        , m_Advance(0.f)
        , m_OffsetX(0.f)
        , m_OffsetY(0.f)
    {
    }

    int m_Image;
    int m_Advance;
    int m_OffsetX;
    int m_OffsetY;
    int m_Width;
    int m_Height;
};

enum CounterType
{
    CounterType_Integer,
    CounterType_Float,
};

struct Counter
{
    Counter(CounterType type, string const& label)
        : m_Type(type)
        , m_Label(label)
        , m_Value(0.f)
    { }
    CounterType m_Type;
    string m_Label;
    float m_Value;
};

struct Impl
{
    Glyph m_FontGlyphs[128];
    int m_FontAscent;
    int m_FontDescent;
    vector<Counter> m_Counters;
    bool m_Visible;

    int m_DebugCounter_FPS;
    int m_DebugCounter_MSPF;
};

static Impl* s_Impl = nullptr;

void Debug_Init()
{
    s_Impl = new Impl;
    s_Impl->m_Visible = false;
    s_Impl->m_DebugCounter_FPS = DebugOverlay_CreateCounter("FPS");
    s_Impl->m_DebugCounter_MSPF = DebugOverlay_CreateCounter("MSPF");
}

void Debug_Shutdown()
{
    delete s_Impl;
}

void DebugOverlay_Init()
{
    // Load debug font glyphs
    const char debugChars[] = " 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}-_=+;:'\",./<>?!@#$%^&*()`~";
    int font;
    float fontSize = 12.f;
    Bacon_GetDefaultFont(&font);
    Bacon_GetFontMetrics(font, fontSize, &s_Impl->m_FontAscent, &s_Impl->m_FontDescent);
    for (int i = 0; i < BACON_ARRAY_COUNT(debugChars); ++i)
    {
        Glyph& g = s_Impl->m_FontGlyphs[debugChars[i]];
        Bacon_GetGlyph(font, fontSize, debugChars[i], 0, &g.m_Image, &g.m_OffsetX, &g.m_OffsetY, &g.m_Advance);
        Bacon_GetImageSize(g.m_Image, &g.m_Width, &g.m_Height);
    }
}

void DebugOverlay_Shutdown()
{
}

void DebugOverlay_Toggle()
{
    s_Impl->m_Visible = !s_Impl->m_Visible;
}

int DebugOverlay_CreateCounter(const char* label)
{
    s_Impl->m_Counters.push_back(Counter(CounterType_Integer, label));
    return (int)s_Impl->m_Counters.size() - 1;
}

int DebugOverlay_CreateFloatCounter(const char* label)
{
    s_Impl->m_Counters.push_back(Counter(CounterType_Float, label));
    return (int)s_Impl->m_Counters.size() - 1;
}

void DebugOverlay_SetCounter(int counter, float value)
{
    s_Impl->m_Counters[counter].m_Value = value;
}

void DebugOverlay_AddCounter(int counter, float value)
{
    s_Impl->m_Counters[counter].m_Value += value;
}

static void DrawString(const char* str, float& x, float& y)
{
    for (const char* c = str; *c; ++c)
    {
        const Glyph& g = s_Impl->m_FontGlyphs[*c];
        if (g.m_Image)
            Bacon_DrawImage(g.m_Image, x + g.m_OffsetX, y - g.m_OffsetY, x + g.m_OffsetX + g.m_Width, y - g.m_OffsetY + g.m_Height);
        x += g.m_Advance;
    }
}

static void DrawCounter(Counter const& counter, float& x, float& y)
{
    float left = x;
    y += s_Impl->m_FontAscent;
    DrawString(counter.m_Label.c_str(), x, y);
    char valueString[24];
    const char* formatStr = ": %.0f";
    if (counter.m_Type == CounterType_Float)
        formatStr = ": %.2f";
#if WIN32
    sprintf_s(valueString, formatStr, counter.m_Value);
#else
    sprintf(valueString, formatStr, counter.m_Value);
#endif
    DrawString(valueString, x, y);
    y -= s_Impl->m_FontDescent;
    x = left;
}

static float s_LastFrameTime;

static void UpdateFPS()
{
    // Measure frame-to-frame time
    float time;
    Platform_GetPerformanceTime(time);
    DebugOverlay_SetCounter(s_Impl->m_DebugCounter_MSPF, (time - s_LastFrameTime) * 1000.f);
    DebugOverlay_SetCounter(s_Impl->m_DebugCounter_FPS, 1.f / (time - s_LastFrameTime));
    s_LastFrameTime = time;
}

void DebugOverlay_Draw()
{
    UpdateFPS();

    if (!s_Impl->m_Visible)
        return;

    int width, height;
    Bacon_GetWindowSize(&width, &height);

	Bacon_SetShader(0);
	Bacon_SetBlending(Bacon_Blend_One, Bacon_Blend_OneMinusSrcAlpha);
    Bacon_SetColor(0.1, 0.1, 0.1, 0.9);
    Bacon_FillRect(20, 20, width - 20, height - 20);
	
    Bacon_SetColor(1, 1, 1, 1);

    float x = 30;
    float y = 30;
    for (size_t i = 0; i < s_Impl->m_Counters.size(); ++i)
        DrawCounter(s_Impl->m_Counters[i], x, y);

    if (true)
    {
        int atlasImage;
        int atlasIndex = 0;
        while (Bacon_DebugGetTextureAtlasImage(&atlasImage, atlasIndex++) == Bacon_Error_None)
        {
            int atlasWidth, atlasHeight;
            Bacon_GetImageSize(atlasImage, &atlasWidth, &atlasHeight);
            Bacon_SetColor(0, 0, 0, 1);
            Bacon_FillRect(x, y, x + atlasWidth, y + atlasHeight);
            Bacon_SetColor(1, 1, 1, 1);
            Bacon_DrawImage(atlasImage, x, y, x + atlasWidth, y + atlasHeight);
            x += atlasWidth + 10;
        }
    }
}