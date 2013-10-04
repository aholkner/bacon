#include <Bacon/Bacon.h>
#include <Python.h>

PyDoc_VAR(s_BaconDoc) = PyDoc_STR("Internal linkage for bacon.native module");

static PyMethodDef s_BaconMethods[] = {
	NULL, NULL
};

static struct PyModuleDef s_BaconModule = {
    PyModuleDef_HEAD_INIT,
    "_bacon",
    s_BaconDoc,
    -1, /* multiple "initialization" just copies the module dict. */
    s_BaconMethods,
    NULL,
    NULL,
    NULL,
    NULL
};

#define EXPORT_BACON_FUNCTION(f) \
	PyDict_SetItemString(moduleDict, #f, PyLong_FromLong((long)f));


PyObject* BaconModule_Init()
{
	PyObject* module = PyModule_Create(&s_BaconModule);
	if (!module)
		return NULL;
	
	PyObject* moduleDict = PyModule_GetDict(module);
	
	EXPORT_BACON_FUNCTION(Bacon_Init);
	EXPORT_BACON_FUNCTION(Bacon_GetVersion);
	EXPORT_BACON_FUNCTION(Bacon_Init);
	EXPORT_BACON_FUNCTION(Bacon_Run);
    EXPORT_BACON_FUNCTION(Bacon_Stop);
	EXPORT_BACON_FUNCTION(Bacon_Shutdown);
	
    EXPORT_BACON_FUNCTION(Bacon_SetLogCallback);
    EXPORT_BACON_FUNCTION(Bacon_SetLogLevel);
	
	EXPORT_BACON_FUNCTION(Bacon_InternalTick);
	EXPORT_BACON_FUNCTION(Bacon_SetTickCallback);
	
	EXPORT_BACON_FUNCTION(Bacon_SetWindowResizeEventHandler);
    EXPORT_BACON_FUNCTION(Bacon_SetWindowTitle);
    EXPORT_BACON_FUNCTION(Bacon_SetWindowResizable);
	EXPORT_BACON_FUNCTION(Bacon_GetWindowSize);
	EXPORT_BACON_FUNCTION(Bacon_SetWindowSize);
	EXPORT_BACON_FUNCTION(Bacon_SetWindowFullscreen);
	EXPORT_BACON_FUNCTION(Bacon_GetWindowContentScale);
	EXPORT_BACON_FUNCTION(Bacon_SetWindowContentScale);
	
	EXPORT_BACON_FUNCTION(Bacon_CreateShader);
	EXPORT_BACON_FUNCTION(Bacon_EnumShaderUniforms);
	EXPORT_BACON_FUNCTION(Bacon_SetShaderUniform);
	EXPORT_BACON_FUNCTION(Bacon_CreateSharedShaderUniform);
	EXPORT_BACON_FUNCTION(Bacon_SetSharedShaderUniform);
	
	EXPORT_BACON_FUNCTION(Bacon_CreateImage);
	EXPORT_BACON_FUNCTION(Bacon_LoadImage);
	EXPORT_BACON_FUNCTION(Bacon_GetImageRegion);
	EXPORT_BACON_FUNCTION(Bacon_UnloadImage);
	EXPORT_BACON_FUNCTION(Bacon_GetImageSize);
	
    EXPORT_BACON_FUNCTION(Bacon_DebugGetTextureAtlasImage);
	
	EXPORT_BACON_FUNCTION(Bacon_PushTransform);
	EXPORT_BACON_FUNCTION(Bacon_PopTransform);
	EXPORT_BACON_FUNCTION(Bacon_Translate);
	EXPORT_BACON_FUNCTION(Bacon_Scale);
	EXPORT_BACON_FUNCTION(Bacon_Rotate);
	EXPORT_BACON_FUNCTION(Bacon_SetTransform);
	
	EXPORT_BACON_FUNCTION(Bacon_PushColor);
	EXPORT_BACON_FUNCTION(Bacon_PopColor);
	EXPORT_BACON_FUNCTION(Bacon_SetColor);
	EXPORT_BACON_FUNCTION(Bacon_MultiplyColor);
	
	EXPORT_BACON_FUNCTION(Bacon_Flush);
	EXPORT_BACON_FUNCTION(Bacon_Clear);
	EXPORT_BACON_FUNCTION(Bacon_SetFrameBuffer);
	EXPORT_BACON_FUNCTION(Bacon_SetViewport);
	EXPORT_BACON_FUNCTION(Bacon_SetShader);
	EXPORT_BACON_FUNCTION(Bacon_SetBlending);
	EXPORT_BACON_FUNCTION(Bacon_DrawImage);
	EXPORT_BACON_FUNCTION(Bacon_DrawImageRegion);
	EXPORT_BACON_FUNCTION(Bacon_DrawImageQuad);
	EXPORT_BACON_FUNCTION(Bacon_DrawLine);
	EXPORT_BACON_FUNCTION(Bacon_DrawRect);
	EXPORT_BACON_FUNCTION(Bacon_FillRect);
	
	// Fonts
	EXPORT_BACON_FUNCTION(Bacon_LoadFont);
	EXPORT_BACON_FUNCTION(Bacon_UnloadFont);
    EXPORT_BACON_FUNCTION(Bacon_GetDefaultFont);
	EXPORT_BACON_FUNCTION(Bacon_GetFontMetrics);
	EXPORT_BACON_FUNCTION(Bacon_GetGlyph);
					             
	
	// Keyboard
	EXPORT_BACON_FUNCTION(Bacon_GetKeyState);
	EXPORT_BACON_FUNCTION(Bacon_SetKeyEventHandler);
	
	// Mouse
	EXPORT_BACON_FUNCTION(Bacon_GetMousePosition);
	EXPORT_BACON_FUNCTION(Bacon_SetMouseButtonEventHandler);
	EXPORT_BACON_FUNCTION(Bacon_SetMouseScrollEventHandler);
	
	// Controller
	EXPORT_BACON_FUNCTION(Bacon_SetControllerConnectedEventHandler);
	EXPORT_BACON_FUNCTION(Bacon_SetControllerButtonEventHandler);
	EXPORT_BACON_FUNCTION(Bacon_SetControllerAxisEventHandler);
	EXPORT_BACON_FUNCTION(Bacon_GetControllerPropertyInt);
	EXPORT_BACON_FUNCTION(Bacon_GetControllerPropertyString);
	
	// Touch
	EXPORT_BACON_FUNCTION(Bacon_SetTouchEventHandler);
	EXPORT_BACON_FUNCTION(Bacon_GetTouchState);
	
	// Audio
	EXPORT_BACON_FUNCTION(Bacon_LoadSound);
	EXPORT_BACON_FUNCTION(Bacon_UnloadSound);
	EXPORT_BACON_FUNCTION(Bacon_PlaySound);
	
	EXPORT_BACON_FUNCTION(Bacon_CreateVoice);
	EXPORT_BACON_FUNCTION(Bacon_DestroyVoice);
	EXPORT_BACON_FUNCTION(Bacon_PlayVoice);
	EXPORT_BACON_FUNCTION(Bacon_StopVoice);
	EXPORT_BACON_FUNCTION(Bacon_SetVoiceGain);
	EXPORT_BACON_FUNCTION(Bacon_SetVoicePitch);
	EXPORT_BACON_FUNCTION(Bacon_SetVoicePan);
	EXPORT_BACON_FUNCTION(Bacon_SetVoiceLoopPoints);
	EXPORT_BACON_FUNCTION(Bacon_SetVoiceCallback);
	EXPORT_BACON_FUNCTION(Bacon_IsVoicePlaying);
	EXPORT_BACON_FUNCTION(Bacon_GetVoicePosition);
	EXPORT_BACON_FUNCTION(Bacon_SetVoicePosition);

	EXPORT_BACON_FUNCTION(Bacon_ExecuteCommands);

	return module;

}