from ctypes import *
import os
import sys

'''Blend values that can be passed to set_blending'''
class BlendFlags(object):
    zero = 0
    one = 1
    src_color = 2
    one_minus_src_color = 3
    dst_color = 4
    one_minus_dst_color = 5
    src_alpha = 6
    one_minus_src_alpha = 7
    dst_alpha = 8
    one_minus_dst_alpha = 9

class ImageFlags(object):
    premultiply_alpha = 1 << 0
    discard_bitmap = 1 << 1

class SoundFlags(object):
    stream = 1 << 0

    format_wav = 1 << 16
    format_ogg = 2 << 16

class VoiceFlags(object):
    loop = 1 << 0

class ControllerProfiles(object):
    generic = 0
    standard = 1
    extended = 2

class ControllerProperties(object):
    supported_axes_mask = 0
    supported_buttons_mask = 1
    vendor_id = 2
    product_id = 3
    name = 4
    profile = 5

class ControllerButtons(object):
    start = 1 << 0
    back = 1 << 1
    select = 1 << 2
    action_up = 1 << 3
    action_down = 1 << 4
    action_left = 1 << 5
    action_right = 1 << 6
    dpad_up = 1 << 7
    dpad_down = 1 << 8
    dpad_left = 1 << 9
    dpad_right = 1 << 10
    left_shoulder = 1 << 11
    right_shoulder = 1 << 12
    left_thumb = 1 << 13
    right_thumb = 1 << 14
    misc0 = 1 << 16

class ControllerAxes(object):
    left_thumb_x = 1 << 0
    left_thumb_y = 1 << 1
    right_thumb_x = 1 << 2
    right_thumb_y = 1 << 3
    left_trigger = 1 << 4
    right_trigger = 1 << 5
    misc0 = 1 << 8
    

def create_fn(function_wrapper):
    import ctypes
    if function_wrapper:
        def fn(f, *argtypes):
            f.restype = ctypes.c_int
            f.argtypes = argtypes
            return function_wrapper(f)
    else:
        def fn(f, *argtypes):
            f.restype = ctypes.c_int
            f.argtypes = argtypes
            return f    
    return fn

def load(function_wrapper = None):    
    fn = create_fn(function_wrapper)

    if sys.platform == 'win32':
        # Dependent DLLs loaded by Bacon.dll also need to be loaded from this path, use
        # SetDllDirectory to affect the library search path; requires XP SP 1 or Vista.
        windll.kernel32.SetDllDirectoryA(os.path.dirname(__file__).encode('utf-8'))
        _lib_path = 'Bacon.dll'
    elif sys.platform == 'darwin':
        _lib_path = os.path.join(os.path.dirname(__file__), 'Bacon.dylib')
    _lib = cdll.LoadLibrary(_lib_path)

    # Function types
    TickCallback = CFUNCTYPE(None)
    WindowResizeEventHandler = CFUNCTYPE(None, c_int, c_int)
    KeyEventHandler = CFUNCTYPE(None, c_int, c_int)
    MouseButtonEventHandler = CFUNCTYPE(None, c_int, c_int)
    MouseScrollEventHandler = CFUNCTYPE(None, c_float, c_float)
    ControllerConnectedEventHandler = CFUNCTYPE(None, c_int, c_int)
    ControllerButtonEventHandler = CFUNCTYPE(None, c_int, c_int, c_int)
    ControllerAxisEventHandler = CFUNCTYPE(None, c_int, c_int, c_float)
    VoiceCallback = CFUNCTYPE(None)

    # Functions
    Init = fn(_lib.Bacon_Init)
    Run = fn(_lib.Bacon_Run)
    GetVersion = fn(_lib.Bacon_GetVersion, POINTER(c_int), POINTER(c_int), POINTER(c_int))
    Shutdown = fn(_lib.Bacon_Shutdown)
    SetTickCallback = fn(_lib.Bacon_SetTickCallback, TickCallback)

    SetWindowResizeEventHandler = fn(_lib.Bacon_SetWindowResizeEventHandler, WindowResizeEventHandler)
    GetWindowSize = fn(_lib.Bacon_GetWindowSize, POINTER(c_int), POINTER(c_int))
    SetWindowSize = fn(_lib.Bacon_SetWindowSize, c_int, c_int)
    SetWindowTitle = fn(_lib.Bacon_SetWindowTitle, c_char_p)
    SetWindowResizable = fn(_lib.Bacon_SetWindowResizable, c_int)
    SetWindowFullscreen = fn(_lib.Bacon_SetWindowFullscreen, c_int)

    CreateShader = fn(_lib.Bacon_CreateShader, POINTER(c_int), c_char_p, c_char_p)
    CreateImage = fn(_lib.Bacon_CreateImage, POINTER(c_int), c_int, c_int)
    LoadImage = fn(_lib.Bacon_LoadImage, POINTER(c_int), c_char_p, c_int)
    UnloadImage = fn(_lib.Bacon_UnloadImage, c_int)
    GetImageSize = fn(_lib.Bacon_GetImageSize, c_int, POINTER(c_int))

    PushTransform = fn(_lib.Bacon_PushTransform)
    PopTransform = fn(_lib.Bacon_PopTransform)
    Translate = fn(_lib.Bacon_Translate, c_float, c_float)
    Scale = fn(_lib.Bacon_Scale, c_float, c_float)
    Rotate = fn(_lib.Bacon_Rotate, c_float)
    SetTransform = fn(_lib.Bacon_SetTransform, c_float * 16)

    PushColor = fn(_lib.Bacon_PushColor)
    PopColor = fn(_lib.Bacon_PopColor)
    SetColor = fn(_lib.Bacon_SetColor, c_float, c_float, c_float, c_float)
    MultiplyColor = fn(_lib.Bacon_MultiplyColor, c_float, c_float, c_float, c_float)

    Clear = fn(_lib.Bacon_Clear, c_float, c_float, c_float, c_float)
    SetFrameBuffer = fn(_lib.Bacon_SetFrameBuffer, c_int)
    SetViewport = fn(_lib.Bacon_SetViewport, c_int, c_int, c_int, c_int)
    SetShader = fn(_lib.Bacon_SetShader, c_int)
    SetBlending = fn(_lib.Bacon_SetBlending, c_int, c_int)
    DrawImage = fn(_lib.Bacon_DrawImage, c_int, c_float, c_float, c_float, c_float)
    DrawImageRegion = fn(_lib.Bacon_DrawImageRegion, c_int, c_float, c_float, c_float, c_float, c_float, c_float, c_float, c_float)
    DrawLine = fn(_lib.Bacon_DrawLine, c_float, c_float, c_float, c_float)

    LoadFont = fn(_lib.Bacon_LoadFont, POINTER(c_int), c_char_p)
    UnloadFont = fn(_lib.Bacon_UnloadFont, c_int)
    GetFontMetrics = fn(_lib.Bacon_GetFontMetrics, c_int, c_float, POINTER(c_float), POINTER(c_float))
    GetGlyph = fn(_lib.Bacon_GetGlyph, c_int, c_float, c_int, POINTER(c_int), POINTER(c_float), POINTER(c_float), POINTER(c_float))

    GetKeyState = fn(_lib.Bacon_GetKeyState, c_int, POINTER(c_int))
    SetKeyEventHandler = fn(_lib.Bacon_SetKeyEventHandler, KeyEventHandler)

    GetMousePosition = fn(_lib.Bacon_GetMousePosition, POINTER(c_float), POINTER(c_float))
    SetMouseButtonEventHandler = fn(_lib.Bacon_SetMouseButtonEventHandler, MouseButtonEventHandler)
    SetMouseScrollEventHandler = fn(_lib.Bacon_SetMouseScrollEventHandler, MouseScrollEventHandler)

    SetControllerConnectedEventHandler = fn(_lib.Bacon_SetControllerConnectedEventHandler, ControllerConnectedEventHandler)
    SetControllerButtonEventHandler = fn(_lib.Bacon_SetControllerButtonEventHandler, ControllerButtonEventHandler)
    SetControllerAxisEventHandler = fn(_lib.Bacon_SetControllerAxisEventHandler, ControllerAxisEventHandler)
    GetControllerPropertyInt = fn(_lib.Bacon_GetControllerPropertyInt, c_int, c_int, POINTER(c_int))
    GetControllerPropertyString = fn(_lib.Bacon_GetControllerPropertyString, c_int, c_int, POINTER(c_char), POINTER(c_int))

    LoadSound = fn(_lib.Bacon_LoadSound, POINTER(c_int), c_char_p, c_int)
    UnloadSound = fn(_lib.Bacon_UnloadSound, c_int)
    PlaySound = fn(_lib.Bacon_PlaySound, c_int)

    CreateVoice = fn(_lib.Bacon_CreateVoice, POINTER(c_int), c_int)
    DestroyVoice = fn(_lib.Bacon_DestroyVoice, c_int)
    PlayVoice = fn(_lib.Bacon_PlayVoice, c_int)
    StopVoice = fn(_lib.Bacon_StopVoice, c_int)
    SetVoiceGain = fn(_lib.Bacon_SetVoiceGain, c_int, c_float)
    SetVoicePitch = fn(_lib.Bacon_SetVoicePitch, c_int, c_float)
    SetVoicePan = fn(_lib.Bacon_SetVoicePan, c_int, c_float)
    SetVoiceLoopPoints = fn(_lib.Bacon_SetVoiceLoopPoints, c_int, c_int, c_int)
    SetVoiceCallback = fn(_lib.Bacon_SetVoiceCallback, c_int, VoiceCallback)
    IsVoicePlaying = fn(_lib.Bacon_IsVoicePlaying, c_int, POINTER(c_int))
    GetVoicePosition = fn(_lib.Bacon_GetVoicePosition, c_int, POINTER(c_int))
    SetVoicePosition = fn(_lib.Bacon_SetVoicePosition, c_int, c_int)

    class BaconLibrary(object):
        pass
    ns = BaconLibrary()
    for k, v in list(locals().items()):
        setattr(ns, k, v)

    return ns

__all__ = [
    ImageFlags,
    SoundFlags,
    VoiceFlags,
    ControllerProperties,
    ControllerAxes,
    ControllerButtons,
    load
]
