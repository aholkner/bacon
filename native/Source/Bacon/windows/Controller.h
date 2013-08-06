#pragma once

#include "../Bacon.h"

void DirectInputController_Init();
void DirectInputController_Shutdown();
void DirectInputController_Update();
int DirectInputController_GetControllerPropertyInt(int controller, int property, int* outValue);
int DirectInputController_GetControllerPropertyString(int controller, int property, char* outBuffer, int* inOutBufferSize);

void XInputController_Init();
void XInputController_Shutdown();
void XInputController_Update();
int XInputController_GetControllerPropertyInt(int controller, int property, int* outValue);
int XInputController_GetControllerPropertyString(int controller, int property, char* outBuffer, int* inOutBufferSize);

enum ControllerProvider
{
    ControllerProvider_None,
    ControllerProvider_DirectInput,
    ControllerProvider_XInput,
};

int Controller_GetAvailableIndex(int preferredIndex);
void Controller_SetProvider(int controller, ControllerProvider provider);

extern Bacon_ControllerConnectedEventHandler g_ControllerConnectedHandler;
extern Bacon_ControllerButtonEventHandler g_ControllerButtonHandler;
extern Bacon_ControllerAxisEventHandler g_ControllerAxisHandler;
