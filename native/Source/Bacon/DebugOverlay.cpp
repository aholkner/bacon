#include "Bacon.h"
#include "BaconInternal.h"

int Bacon_DrawDebugOverlay()
{
	Bacon_SetBlending(Bacon_Blend_One, Bacon_Blend_OneMinusSrcAlpha);

	Graphics_DrawDebugOverlay();
	return Bacon_Error_None;
}