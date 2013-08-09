#include "../Bacon.h"
#include "../BaconInternal.h"

#include <unordered_map>

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>

using namespace std;

static Bacon_ControllerConnectedEventHandler s_ConnectedHandler = nullptr;
static Bacon_ControllerButtonEventHandler s_ButtonHandler = nullptr;
static Bacon_ControllerAxisEventHandler s_AxisHandler = nullptr;

static IOHIDManagerRef s_HIDManager;

constexpr int MaxControllerCount = 4;

enum ControllerElementType
{
	ControllerElementType_Button,
	ControllerElementType_Axis,
	ControllerElementType_Hatswitch,
};

struct ControllerElement
{
	ControllerElementType m_Type;
	int m_Index;
	int m_MinValue;
	int m_MaxValue;
};

struct Controller
{
	IOHIDDeviceRef m_Device;
	int m_ButtonMask;
	int m_SupportedButtonsMask;
	int m_SupportedAxesMask;
	unordered_map<IOHIDElementRef, ControllerElement> m_Elements;
};
static Controller s_Controllers[MaxControllerCount];

static NSString* GetPropertyString(IOHIDDeviceRef device, CFStringRef property)
{
	CFTypeRef type = IOHIDDeviceGetProperty(device, property);
	if (type && CFGetTypeID(type) == CFStringGetTypeID())
		return (NSString*)type;
	return @"";
}

static int GetPreferredButtonIndex(uint32_t page, uint32_t usage)
{
	if (page == kHIDPage_GenericDesktop)
	{
		switch (usage)
		{
			case kHIDUsage_GD_DPadUp: return Bacon_Controller_Button_DpadUp;
			case kHIDUsage_GD_DPadDown: return Bacon_Controller_Button_DpadDown;
			case kHIDUsage_GD_DPadLeft: return Bacon_Controller_Button_DpadLeft;
			case kHIDUsage_GD_DPadRight: return Bacon_Controller_Button_DpadRight;

			case kHIDUsage_GD_Start: return Bacon_Controller_Button_Start;
			case kHIDUsage_GD_Select: return Bacon_Controller_Button_Select;
		}
	}
	else if (page == kHIDPage_Button)
	{
		return Bacon_Controller_Button_Misc0 << usage;
	}
	return Bacon_Controller_Button_Misc0;
}

static int GetPreferredAxisIndex(uint32_t page, uint32_t usage)
{
	if (page == kHIDPage_GenericDesktop)
	{
		switch (usage)
		{
			case kHIDUsage_GD_X: return Bacon_Controller_Axis_LeftThumbX;
			case kHIDUsage_GD_Y: return Bacon_Controller_Axis_LeftThumbY;
			case kHIDUsage_GD_Z: return Bacon_Controller_Axis_RightThumbX;
			case kHIDUsage_GD_Rz: return Bacon_Controller_Axis_RightThumbY;
		}
	}
	return Bacon_Controller_Axis_Misc0;
}

static int GetAvailableIndex(int mask, int preferredIndex, int miscIndex)
{
	int index = preferredIndex;
	if (mask & index)
		index = miscIndex;
	
	while (index && (mask & index))
		index <<= 1;
	
	return index;
}

static void AddControllerButton(int controllerIndex, IOHIDElementRef element)
{
	uint32_t page = IOHIDElementGetUsagePage(element);
	uint32_t usage = IOHIDElementGetUsage(element);

	Controller& controller = s_Controllers[controllerIndex];

	int preferredIndex = GetPreferredButtonIndex(page, usage);
	int elementIndex = GetAvailableIndex(controller.m_SupportedButtonsMask, preferredIndex, Bacon_Controller_Button_Misc0);
	
	if (!elementIndex)
		return;
	
	controller.m_Elements[element] = { ControllerElementType_Button, elementIndex, 0, 1 };
	controller.m_SupportedButtonsMask |= elementIndex;
}

static void AddControllerAxis(int controllerIndex, IOHIDElementRef element)
{
	uint32_t page = IOHIDElementGetUsagePage(element);
	uint32_t usage = IOHIDElementGetUsage(element);
	int logicalMin = (int)IOHIDElementGetLogicalMin(element);
	int logicalMax = (int)IOHIDElementGetLogicalMax(element);
	
	Controller& controller = s_Controllers[controllerIndex];

	// Treat hatswitch as dpad buttons
	if (page == kHIDPage_GenericDesktop &&
		usage == kHIDUsage_GD_Hatswitch &&
		(controller.m_SupportedButtonsMask & Bacon_Controller_ButtonMask_Dpad) == 0)
	{
		controller.m_Elements[element] = { ControllerElementType_Hatswitch, Bacon_Controller_ButtonMask_Dpad, logicalMin, logicalMax };
		controller.m_SupportedButtonsMask |= Bacon_Controller_ButtonMask_Dpad;
		return;
	}
	
	int preferredIndex = GetPreferredAxisIndex(page, usage);
	int elementIndex = GetAvailableIndex(controller.m_SupportedAxesMask, preferredIndex, Bacon_Controller_Axis_Misc0);
	
	if (!elementIndex)
		return;
	
	controller.m_Elements[element] = { ControllerElementType_Axis, elementIndex, logicalMin, logicalMax };
	controller.m_SupportedAxesMask |= elementIndex;
}

static void AddControllerElement(const void* elementPtr, void* context)
{
	// Add this element to its controller if it's a button or axis, and choose the
	// best axis or button index to assign
	int controllerIndex = (int)(size_t)context;
	IOHIDElementRef element = (IOHIDElementRef)elementPtr;
	IOHIDElementType elementType = IOHIDElementGetType(element);
	if (elementType == kIOHIDElementTypeInput_Button)
		AddControllerButton(controllerIndex, element);
	else if (elementType == kIOHIDElementTypeInput_Axis ||
			 elementType == kIOHIDElementTypeInput_Misc)
		AddControllerAxis(controllerIndex, element);
}

static int GetControllerIndexForDevice(IOHIDDeviceRef device)
{
	for (int i = 0; i < MaxControllerCount; ++i)
	{
		if (s_Controllers[i].m_Device == device)
			return i;
	}
	return -1;
}

static void OnDeviceMatched(void* context, IOReturn result, void* sender, IOHIDDeviceRef device)
{
	// Find a free controller index
	int index = GetControllerIndexForDevice(nullptr);
	if (index == -1)
	{
		// Too many controllers; ignore
		// TODO log
		return;
	}
	
	// Discover elements
	s_Controllers[index].m_Device = device;
	CFArrayRef elements = IOHIDDeviceCopyMatchingElements(device, nullptr, kIOHIDOptionsTypeNone);
	CFArrayApplyFunction(elements, CFRangeMake(0, CFArrayGetCount(elements)), AddControllerElement, (void*)(size_t)index);
	
	// Notify connection
	if (s_ConnectedHandler)
		s_ConnectedHandler(index, 1);
}

static void OnDeviceRemoved(void* context, IOReturn result, void* sender, IOHIDDeviceRef device)
{
	int index = GetControllerIndexForDevice(device);
	if (index == -1)
		return;
	
	s_Controllers[index].m_Device = nullptr;
	if (s_ConnectedHandler)
		s_ConnectedHandler(index, 0);
}

static int GetButtonMaskFromHatswitchValue(int hat)
{
	switch (hat)
	{
		case 0:
			return Bacon_Controller_Button_DpadUp;
		case 1:
			return Bacon_Controller_Button_DpadUp | Bacon_Controller_Button_DpadRight;
		case 2:
			return Bacon_Controller_Button_DpadRight;
		case 3:
			return Bacon_Controller_Button_DpadDown | Bacon_Controller_Button_DpadRight;
		case 4:
			return Bacon_Controller_Button_DpadDown;
		case 5:
			return Bacon_Controller_Button_DpadDown | Bacon_Controller_Button_DpadLeft;
		case 6:
			return Bacon_Controller_Button_DpadLeft;
		case 7:
			return Bacon_Controller_Button_DpadUp | Bacon_Controller_Button_DpadLeft;
		default:
			return 0;
	}
}

static void OnValueChanged(void* context, IOReturn result, void* sender, IOHIDValueRef value)
{
	IOHIDElementRef element = IOHIDValueGetElement(value);
	IOHIDDeviceRef device = IOHIDElementGetDevice(element);
	int controllerIndex = GetControllerIndexForDevice(device);
	if (controllerIndex < 0)
		return;
	
	Controller& controller = s_Controllers[controllerIndex];
	auto it = controller.m_Elements.find(element);
	if (it != controller.m_Elements.end())
	{
		int valueInt = (int)IOHIDValueGetIntegerValue(value);

		ControllerElement& e = it->second;
		if (e.m_Type == ControllerElementType_Button)
		{
			if (valueInt)
				controller.m_ButtonMask |= e.m_Index;
			else
				controller.m_ButtonMask &= ~e.m_Index;
			
			if (s_ButtonHandler)
				s_ButtonHandler(controllerIndex, e.m_Index, valueInt);
		}
		else if (e.m_Type == ControllerElementType_Axis)
		{
			if (s_AxisHandler)
				s_AxisHandler(controllerIndex, e.m_Index,
							  2.f * (valueInt - e.m_MinValue) / (float)(e.m_MaxValue - e.m_MinValue) - 1.f);
		}
		else if (e.m_Type == ControllerElementType_Hatswitch)
		{
			if (s_ButtonHandler)
			{
				int buttons = GetButtonMaskFromHatswitchValue(valueInt);
				int oldButtons = controller.m_ButtonMask & Bacon_Controller_ButtonMask_Dpad;
				int changedButtons = oldButtons ^ buttons;
				controller.m_ButtonMask = controller.m_ButtonMask & ~Bacon_Controller_ButtonMask_Dpad;
				controller.m_ButtonMask |= buttons;
				if (changedButtons & Bacon_Controller_Button_DpadUp)
					s_ButtonHandler(controllerIndex, Bacon_Controller_Button_DpadUp, bool(buttons & Bacon_Controller_Button_DpadUp));
				if (changedButtons & Bacon_Controller_Button_DpadDown)
					s_ButtonHandler(controllerIndex, Bacon_Controller_Button_DpadDown, bool(buttons & Bacon_Controller_Button_DpadDown));
				if (changedButtons & Bacon_Controller_Button_DpadLeft)
					s_ButtonHandler(controllerIndex, Bacon_Controller_Button_DpadLeft, bool(buttons & Bacon_Controller_Button_DpadLeft));
				if (changedButtons & Bacon_Controller_Button_DpadRight)
					s_ButtonHandler(controllerIndex, Bacon_Controller_Button_DpadRight, bool(buttons & Bacon_Controller_Button_DpadRight));
			}
		}
	}
}


void Controller_Init()
{
	for (int i = 0; i < MaxControllerCount; ++i)
	{
		s_Controllers[i].m_Device = nullptr;
		s_Controllers[i].m_ButtonMask = 0;
		s_Controllers[i].m_SupportedButtonsMask = 0;
		s_Controllers[i].m_SupportedAxesMask = 0;
	}
	
	s_HIDManager = IOHIDManagerCreate(nullptr, kIOHIDOptionsTypeNone);
	
	NSArray* masks = [NSArray arrayWithObjects:
					  [NSDictionary dictionaryWithObjectsAndKeys:
					   [NSNumber numberWithInteger:kHIDPage_GenericDesktop], CFSTR(kIOHIDDeviceUsagePageKey),
					   [NSNumber numberWithInteger:kHIDUsage_GD_Joystick], CFSTR(kIOHIDDeviceUsageKey),
					   nil],
					  [NSDictionary dictionaryWithObjectsAndKeys:
					   [NSNumber numberWithInteger:kHIDPage_GenericDesktop], CFSTR(kIOHIDDeviceUsagePageKey),
					   [NSNumber numberWithInteger:kHIDUsage_GD_GamePad], CFSTR(kIOHIDDeviceUsageKey),
					   nil],
					  [NSDictionary dictionaryWithObjectsAndKeys:
					   [NSNumber numberWithInteger:kHIDPage_GenericDesktop], CFSTR(kIOHIDDeviceUsagePageKey),
					   [NSNumber numberWithInteger:kHIDUsage_GD_MultiAxisController], CFSTR(kIOHIDDeviceUsageKey),
					   nil],
					  nil];
	IOHIDManagerSetDeviceMatchingMultiple(s_HIDManager, (CFArrayRef)masks);
	
	IOHIDManagerRegisterDeviceMatchingCallback(s_HIDManager, OnDeviceMatched, nullptr);
	IOHIDManagerRegisterDeviceRemovalCallback(s_HIDManager, OnDeviceRemoved, nullptr);
	
	IOHIDManagerScheduleWithRunLoop(s_HIDManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	
	IOHIDManagerOpen(s_HIDManager, kIOHIDOptionsTypeNone);
	
	IOHIDManagerRegisterInputValueCallback(s_HIDManager, OnValueChanged, nullptr);
}

void Controller_Shutdown()
{
	IOHIDManagerUnscheduleFromRunLoop(s_HIDManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	CFRelease(s_HIDManager);
}

void Controller_Update()
{
}

int Bacon_SetControllerConnectedEventHandler(Bacon_ControllerConnectedEventHandler handler)
{
	s_ConnectedHandler = handler;
	return Bacon_Error_None;
}

int Bacon_SetControllerButtonEventHandler(Bacon_ControllerButtonEventHandler handler)
{
	s_ButtonHandler = handler;
	return Bacon_Error_None;
}

int Bacon_SetControllerAxisEventHandler(Bacon_ControllerAxisEventHandler handler)
{
	s_AxisHandler = handler;
	return Bacon_Error_None;
}

static CFStringRef GetPropertyKey(int property)
{
	switch (property)
	{
		case Bacon_Controller_Property_VendorId: return CFSTR(kIOHIDVendorIDKey);
		case Bacon_Controller_Property_ProductId: return CFSTR(kIOHIDProductIDKey);
		case Bacon_Controller_Property_Name: return CFSTR(kIOHIDProductKey);
		default: return nullptr;
	};
}

static int GetControllerProperty(int controller, int property, CFTypeRef& outValue)
{
	IOHIDDeviceRef device = s_Controllers[controller].m_Device;
	if (!device)
		return Bacon_Error_InvalidHandle;
	
	CFStringRef propertyKey = GetPropertyKey(property);
	if (!propertyKey)
		return Bacon_Error_InvalidArgument;
	
	outValue = IOHIDDeviceGetProperty(device, propertyKey);
	return Bacon_Error_None;
}

int Bacon_GetControllerPropertyInt(int controller, int property, int* outValue)
{
	if (!outValue)
		return Bacon_Error_InvalidArgument;
	
	if (controller < 0 || controller > MaxControllerCount)
		return Bacon_Error_InvalidHandle;
	
	switch (property)
	{
		case Bacon_Controller_Property_SupportedAxesMask:
			*outValue = s_Controllers[controller].m_SupportedAxesMask;
			return Bacon_Error_None;
		case Bacon_Controller_Property_SupportedButtonsMask:
			*outValue = s_Controllers[controller].m_SupportedButtonsMask;
			return Bacon_Error_None;
		case Bacon_Controller_Property_Profile:
			return Bacon_Controller_Profile_Generic;
	}

	CFTypeRef type;
	if (int error = GetControllerProperty(controller, property, type))
		return error;
	
	if (!type || CFGetTypeID(type) != CFNumberGetTypeID())
		return Bacon_Error_InvalidArgument;

	if (!CFNumberGetValue((CFNumberRef)type, kCFNumberSInt32Type, outValue))
		return Bacon_Error_InvalidArgument;

	return Bacon_Error_None;
}

int Bacon_GetControllerPropertyString(int controller, int property, char* outBuffer, int* inOutBufferSize)
{
	if (!outBuffer)
		return Bacon_Error_InvalidArgument;
	
	if (controller < 0 || controller > MaxControllerCount)
		return Bacon_Error_InvalidHandle;
	
	CFTypeRef type;
	if (int error = GetControllerProperty(controller, property, type))
		return error;
	
	if (!type || CFGetTypeID(type) != CFStringGetTypeID())
		return Bacon_Error_InvalidArgument;
	
	CFStringRef str = (CFStringRef)type;
	CFIndex usedBufLen;
	if (!CFStringGetBytes(str, CFRangeMake(0, CFStringGetLength(str)), kCFStringEncodingUTF8, '?', false, (UInt8*)outBuffer, *inOutBufferSize - 1, &usedBufLen))
		return Bacon_Error_Unknown;
	
	outBuffer[usedBufLen] = '\0';
	*inOutBufferSize = (int)CFStringGetLength(str) + 1;
	return Bacon_Error_None;
}
