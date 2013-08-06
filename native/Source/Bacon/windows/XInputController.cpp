#include "Controller.h"

void XInputController_Init()
{


}
void XInputController_Shutdown()
{

}

void XInputController_Update()
{

}

int XInputController_GetControllerPropertyInt(int controller, int property, int* outValue)
{
    return Bacon_Error_InvalidArgument;
}

int XInputController_GetControllerPropertyString(int controller, int property, char* outBuffer, int* inOutBufferSize)
{
    return Bacon_Error_InvalidArgument;
}
