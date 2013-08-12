#include "Controller.h"
#include "../BaconInternal.h"
#include <cmath>
using namespace std;

#include <Windows.h>
#include <Xinput.h>

#pragma comment(lib, "Xinput9_1_0.lib")

struct XInputState
{
    WORD m_Buttons;
    float m_LeftThumbX;
    float m_LeftThumbY;
    float m_RightThumbX;
    float m_RightThumbY;
    float m_LeftTrigger;
    float m_RightTrigger;
};

struct XInputController
{
    int m_XInputIndex;
    XInputState m_State;
};
static XInputController s_Controllers[MaxControllerCount];

static int GetControllerIndex(int xinputIndex)
{
    for (int i = 0; i < MaxControllerCount; ++i)
    {
        if (s_Controllers[i].m_XInputIndex == xinputIndex)
            return i;
    }

    return -1;
}

static void OnControllerConnected(int xinputIndex, const XINPUT_GAMEPAD& state)
{
    // If already connected, ignore
    if (GetControllerIndex(xinputIndex) != -1)
        return;

    // Assign local controller index
    int controller = Controller_GetAvailableIndex(xinputIndex);
    if (controller == -1)
        return;

    ZeroMemory(&s_Controllers[controller], sizeof(XInputController));
    s_Controllers[controller].m_XInputIndex = xinputIndex;
    Controller_SetProvider(controller, ControllerProvider_XInput);
}

static void OnControllerDisconnected(int controller)
{
    s_Controllers[controller].m_XInputIndex = -1;
    Controller_SetProvider(controller, ControllerProvider_None);
}

static void NormalizeAxis(SHORT x, SHORT y, SHORT deadZone, float& outX, float& outY)
{
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ee417001(v=vs.85).aspx

    float LX = (float)x;
    float LY = (float)y;

    //determine how far the controller is pushed
    float magnitude = sqrtf(LX*LX + LY*LY);

    //determine the direction the controller is pushed
    outX = LX / magnitude;
    outY = LY / magnitude;

    float normalizedMagnitude = 0.f;

    //check if the controller is outside a circular dead zone
    if (magnitude > deadZone)
    {
        //clip the magnitude at its expected maximum value
        if (magnitude > 32767) magnitude = 32767;

        //adjust magnitude relative to the end of the dead zone
        magnitude -= deadZone;

        //optionally normalize the magnitude with respect to its expected range
        //giving a magnitude value of 0.0 to 1.0
        normalizedMagnitude = magnitude / (32767 - deadZone);
    }
    else //if the controller is in the deadzone zero out the magnitude
    {
        magnitude = 0.0;
        normalizedMagnitude = 0.0;
    }

    outX *= normalizedMagnitude;
    outY *= normalizedMagnitude;
}

static void NormalizeTrigger(BYTE trigger, BYTE threshold, float& outTrigger)
{
    if (trigger > threshold)
        outTrigger = (float)(trigger - threshold) / (float)(255 - threshold);
    else
        outTrigger = 0.f;
}

static void DispatchAxis(int controller, int axis, XInputState const& oldState, XInputState const& newState, float XInputState::* member)
{
    if (oldState.*member != newState.*member && g_ControllerAxisHandler)
        g_ControllerAxisHandler(controller, axis, newState.*member);
}

static void DispatchButton(int controller, int button, WORD changedButtons, WORD buttons, int mask)
{
    if ((changedButtons & mask) && g_ControllerButtonHandler)
        g_ControllerButtonHandler(controller, button, (buttons & mask) ? 1 : 0);
}

static void UpdateController(int controller, const XINPUT_GAMEPAD& gamepad)
{
    XInputState const& oldState = s_Controllers[controller].m_State;
    XInputState state;
    state.m_Buttons = gamepad.wButtons;
    NormalizeAxis(gamepad.sThumbLX, gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, state.m_LeftThumbX, state.m_LeftThumbY);
    NormalizeAxis(gamepad.sThumbRX, gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, state.m_RightThumbX, state.m_RightThumbY);
    NormalizeTrigger(gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, state.m_LeftTrigger);
    NormalizeTrigger(gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, state.m_RightTrigger);

    DispatchAxis(controller, Bacon_Controller_Axis_LeftThumbX, oldState, state, &XInputState::m_LeftThumbX);
    DispatchAxis(controller, Bacon_Controller_Axis_LeftThumbY, oldState, state, &XInputState::m_LeftThumbY);
    DispatchAxis(controller, Bacon_Controller_Axis_RightThumbX, oldState, state, &XInputState::m_RightThumbX);
    DispatchAxis(controller, Bacon_Controller_Axis_RightThumbY, oldState, state, &XInputState::m_RightThumbY);
    DispatchAxis(controller, Bacon_Controller_Axis_LeftTrigger, oldState, state, &XInputState::m_LeftTrigger);
    DispatchAxis(controller, Bacon_Controller_Axis_RightTrigger, oldState, state, &XInputState::m_RightTrigger);

    WORD changedButtons = state.m_Buttons ^ oldState.m_Buttons;
    if (changedButtons)
    {
        DispatchButton(controller, Bacon_Controller_Button_DpadUp, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_DPAD_UP);
        DispatchButton(controller, Bacon_Controller_Button_DpadDown, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_DPAD_DOWN);
        DispatchButton(controller, Bacon_Controller_Button_DpadLeft, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_DPAD_LEFT);
        DispatchButton(controller, Bacon_Controller_Button_DpadRight, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_DPAD_RIGHT);
        DispatchButton(controller, Bacon_Controller_Button_Start, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_START);
        DispatchButton(controller, Bacon_Controller_Button_Back, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_BACK);
        DispatchButton(controller, Bacon_Controller_Button_LeftThumb, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_LEFT_THUMB);
        DispatchButton(controller, Bacon_Controller_Button_RightThumb, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_RIGHT_THUMB);
        DispatchButton(controller, Bacon_Controller_Button_LeftShoulder, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_LEFT_SHOULDER);
        DispatchButton(controller, Bacon_Controller_Button_RightShoulder, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_RIGHT_SHOULDER);
        DispatchButton(controller, Bacon_Controller_Button_ActionDown, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_A);
        DispatchButton(controller, Bacon_Controller_Button_ActionRight, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_B);
        DispatchButton(controller, Bacon_Controller_Button_ActionLeft, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_X);
        DispatchButton(controller, Bacon_Controller_Button_ActionUp, changedButtons, state.m_Buttons, XINPUT_GAMEPAD_Y);
    }
    
    s_Controllers[controller].m_State = state;
}

void XInputController_Init()
{
    for (int i = 0; i < MaxControllerCount; ++i)
        s_Controllers[i].m_XInputIndex = -1;
}

void XInputController_Shutdown()
{

}

void XInputController_EnumDevices()
{
    for (int i = 0; i < XUSER_MAX_COUNT; ++i)
    {
        XINPUT_STATE state;
        if (XInputGetState(i, &state) == ERROR_SUCCESS)
            OnControllerConnected(i, state.Gamepad);
    }
}

void XInputController_Update()
{
    for (int i = 0; i < MaxControllerCount; ++i)
    {
        if (s_Controllers[i].m_XInputIndex == -1)
            continue;

        XINPUT_STATE state;
        if (XInputGetState(s_Controllers[i].m_XInputIndex, &state) != ERROR_SUCCESS)
        {
            OnControllerDisconnected(i);
            continue;
        }

        UpdateController(i, state.Gamepad);
    }
}

static const int SupportedAxesMask =
    Bacon_Controller_Axis_LeftThumbX |
    Bacon_Controller_Axis_LeftThumbY |
    Bacon_Controller_Axis_RightThumbX |
    Bacon_Controller_Axis_RightThumbY |
    Bacon_Controller_Axis_LeftTrigger |
    Bacon_Controller_Axis_RightTrigger;

static const int SupportedButtonsMask =
    Bacon_Controller_Button_DpadUp |
    Bacon_Controller_Button_DpadDown |
    Bacon_Controller_Button_DpadLeft |
    Bacon_Controller_Button_DpadRight |
    Bacon_Controller_Button_Start |
    Bacon_Controller_Button_Back |
    Bacon_Controller_Button_LeftThumb |
    Bacon_Controller_Button_RightThumb |
    Bacon_Controller_Button_LeftShoulder |
    Bacon_Controller_Button_RightShoulder |
    Bacon_Controller_Button_ActionDown |
    Bacon_Controller_Button_ActionRight | 
    Bacon_Controller_Button_ActionLeft |
    Bacon_Controller_Button_ActionUp;

int XInputController_GetControllerPropertyInt(int controller, int property, int* outValue)
{
    switch (property)
    {
    case Bacon_Controller_Property_VendorId:
    case Bacon_Controller_Property_ProductId:
        // TODO 
        *outValue = 0;
        return Bacon_Error_None;
    case Bacon_Controller_Property_SupportedAxesMask:
        *outValue = SupportedAxesMask;
        return Bacon_Error_None;
    case Bacon_Controller_Property_SupportedButtonsMask:
        *outValue = SupportedButtonsMask;
        return Bacon_Error_None;
    case Bacon_Controller_Property_Profile:
        *outValue = Bacon_Controller_Profile_Extended;
        return Bacon_Error_None;
    }

    return Bacon_Error_InvalidArgument;
}

static const char Name[] = "Xbox 360 Controller";

int XInputController_GetControllerPropertyString(int controller, int property, char* outBuffer, int* inOutBufferSize)
{
    switch (property)
    {
    case Bacon_Controller_Property_Name:
        strncpy_s(outBuffer, *inOutBufferSize, Name, _TRUNCATE);
        *inOutBufferSize = min(*inOutBufferSize, BACON_ARRAY_COUNT(Name));
        return Bacon_Error_None;
    }
    return Bacon_Error_InvalidArgument;
}
