from ctypes import *
import math
import os
import sys

import logging
logger = logging.getLogger('bacon')

_mock_native = False
if 'BACON_MOCK_NATIVE' in os.environ and os.environ['BACON_MOCK_NATIVE']:
    _mock_native = True
    
def enum(cls):
    names = {}
    for key in dir(cls):
        if key[0] != '_':
            names[getattr(cls, key)] = key
    cls.__names = names
    
    @classmethod
    def tostring(cls, value):
        try:
            return cls.__names[value]
        except KeyError:
            return str(value)
    cls.tostring = tostring

    @classmethod
    def parse(cls, name):
        try:
            return getattr(cls, name)
        except KeyError:
            return int(name)
    cls.parse = parse
    return cls

def flags(cls):
    names = {}
    for key in dir(cls):
        if key[0] != '_':
            names[getattr(cls, key)] = key
    cls.__names = names

    @classmethod
    def tostring(cls, value):
        if not value:
            return '0'
        items = []
        bit = 1
        while value:
            if value & 1:
                try:
                    items.append(cls.__names[bit])
                except KeyError:
                    items.append('(1 << %d)' % math.log(bit, 2))
            bit <<= 1
            value >>= 1
            value &= 0x7fffffff
        return ' | '.join(items)
    cls.tostring = tostring

    @classmethod
    def parse(cls, names):
        try:
            v = 0
            for name in names.split('|'):
                v |= getattr(cls, name.strip())
            return v
        except KeyError:
            return int(names)
    cls.parse = parse

    return cls    

@enum
class ErrorCodes(object):
    none = 0
    unknown = 1
    invalid_argument = 2
    invalid_handle = 3
    stack_underflow = 4
    unsupported_format = 5
    shader_compile_error = 6
    shader_link_error = 7
    not_rendering = 8
    invalid_font_size = 9
    not_looping = 10
    running = 11
    rendering_to_self = 12
    io_error = 13

@enum
class LogLevels(object):
    trace = 0
    info = 1
    warning = 2
    error = 3
    fatal = 4
    disable = 5

@enum
class Commands(object):
    push_transform = 0
    pop_transform = 1
    translate = 2
    scale = 3
    rotate = 4
    set_transform = 5
    push_color = 6
    pop_color = 7
    set_color = 8
    multiply_color = 9
    set_blending = 10
    draw_image = 11
    draw_image_region = 12
    draw_image_quad = 13
    draw_line = 14
    draw_rect = 15
    fill_rect = 16
    set_shader_uniform_floats = 17
    set_shader_uniform_ints = 18
    set_shared_shader_uniform_floats = 19
    set_shared_shader_uniform_ints = 20
    set_shader = 21
    clear = 22
    set_frame_buffer = 23
    set_viewport = 24

'''Blend values that can be passed to set_blending'''
@enum
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

ImageFlags_atlas_shift = 8

@flags
class ImageFlags(object):
    premultiply_alpha = 1 << 0
    discard_bitmap = 1 << 1
    sample_nearest = 1 << 2
    wrap = 1 << 3

    atlas_mask = 0xff << 8

@flags
class FontFlags(object):
    light_hinting = 1 << 0

@enum
class ShaderUniformType(object):
    none            = 0
    int_            = 0x1404
    float_          = 0x1406
    vec2            = 0x8B50
    vec3            = 0x8B51
    vec4            = 0x8B52
    ivec2           = 0x8B53
    ivec3           = 0x8B54
    ivec4           = 0x8B55
    bool_           = 0x8B56
    bvec2           = 0x8B57
    bvec3           = 0x8B58
    bvec4           = 0x8B59
    mat2            = 0x8B5A
    mat3            = 0x8B5B
    mat4            = 0x8B5C
    sampler2D       = 0x8B5E
    samplerCube     = 0x8B60
    sampler2DRect   = 0x8B63
    samplerExternal = 0x8D66

@flags
class SoundFlags(object):
    stream = 1 << 0

    format_wav = 1 << 16
    format_ogg = 2 << 16

@flags
class VoiceFlags(object):
    loop = 1 << 0

@enum
class ControllerProfiles(object):
    generic = 0
    standard = 1
    extended = 2

@enum
class ControllerProperties(object):
    supported_axes_mask = 0
    supported_buttons_mask = 1
    vendor_id = 2
    product_id = 3
    name = 4
    profile = 5

@flags
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
    button1 = 1 << 15
    button2 = 1 << 16
    button3 = 1 << 17
    button4 = 1 << 18
    button5 = 1 << 19
    button6 = 1 << 20
    button7 = 1 << 21
    button8 = 1 << 22
    button9 = 1 << 23
    button10 = 1 << 24
    button11 = 1 << 25
    button12 = 1 << 26
    button13 = 1 << 27
    button14 = 1 << 28
    button15 = 1 << 29
    button16 = 1 << 30
    button17 = 1 << 31

@flags
class ControllerAxes(object):
    left_thumb_x = 1 << 0
    left_thumb_y = 1 << 1
    right_thumb_x = 1 << 2
    right_thumb_y = 1 << 3
    left_trigger = 1 << 4
    right_trigger = 1 << 5
    axis1 = 1 << 8
    axis2 = 1 << 9
    axis3 = 1 << 10
    axis4 = 1 << 11
    axis5 = 1 << 12
    axis6 = 1 << 13
    axis7 = 1 << 14
    axis8 = 1 << 15
    
@enum
class Keys(object):
    none            = 0
    space           = ord(' ')
    a               = ord('a')
    b               = ord('b')
    c               = ord('c')
    d               = ord('d')
    e               = ord('e')
    f               = ord('f')
    g               = ord('g')
    h               = ord('h')
    i               = ord('i')
    j               = ord('j')
    k               = ord('k')
    l               = ord('l')
    m               = ord('m')
    n               = ord('n')
    o               = ord('o')
    p               = ord('p')
    q               = ord('q')
    r               = ord('r')
    s               = ord('s')
    t               = ord('t')
    u               = ord('u')
    v               = ord('v')
    w               = ord('w')
    x               = ord('x')
    y               = ord('y')
    z               = ord('z')
    comma           = ord(',')
    period          = ord('.')
    slash           = ord('/')
    quote           = ord("'")
    semicolon       = ord(';')
    backtick        = ord('`')
    left_paren      = ord('(')
    right_paren     = ord(')')
    left_brace      = ord('{')
    right_brace     = ord('}')
    left_bracket    = ord('[')
    right_bracket   = ord(']')
    backslash       = ord('\\')
    minus           = ord('-')
    plus            = ord('+')
    underscore      = ord('_')
    equals          = ord('=')
    question        = ord('?')
    tilde           = ord('~')
    digit0          = ord('0')
    digit1          = ord('1')
    digit2          = ord('2')
    digit3          = ord('3')
    digit4          = ord('4')
    digit5          = ord('5')
    digit6          = ord('6')
    digit7          = ord('7')
    digit8          = ord('8')
    digit9          = ord('9')
    left            = 0x100
    right           = 0x100 + 1
    up              = 0x100 + 2
    down            = 0x100 + 3
    enter           = 0x100 + 4
    ctrl            = 0x100 + 5
    shift           = 0x100 + 6
    alt             = 0x100 + 7
    command         = 0x100 + 8
    tab             = 0x100 + 9
    insert          = 0x100 + 10
    delete          = 0x100 + 11
    backspace       = 0x100 + 12
    home            = 0x100 + 13
    end             = 0x100 + 14
    pageup          = 0x100 + 15
    pagedown        = 0x100 + 16
    escape          = 0x100 + 17
    f1              = 0x100 + 18
    f2              = 0x100 + 19
    f3              = 0x100 + 20
    f4              = 0x100 + 21
    f5              = 0x100 + 22
    f6              = 0x100 + 23
    f7              = 0x100 + 24
    f8              = 0x100 + 25
    f9              = 0x100 + 26
    f10             = 0x100 + 27
    f11             = 0x100 + 28
    f12             = 0x100 + 29
    numpad0         = 0x100 + 30
    numpad1         = 0x100 + 31
    numpad2         = 0x100 + 32
    numpad3         = 0x100 + 33
    numpad4         = 0x100 + 34
    numpad5         = 0x100 + 35
    numpad6         = 0x100 + 36
    numpad7         = 0x100 + 37
    numpad8         = 0x100 + 38
    numpad9         = 0x100 + 39
    numpad_div      = 0x100 + 40
    numpad_mul      = 0x100 + 41
    numpad_sub      = 0x100 + 42
    numpad_add      = 0x100 + 43
    numpad_enter    = 0x100 + 44
    numpad_period   = 0x100 + 45

@enum
class MouseButtons(object):
    left = 0
    middle = 1
    right = 2

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

class _DllPath(object):
    def __init__(self, module, files):
        self.module = module
        self.files = files

    def get_dir(self):
        # In pkg_resources, we may need to extract DLLs into temporary directory
        # via resource_filename
        try:
            # Import without using import keyword, to avoid analysis tools
            # picking up that we depend on pkg_resources
            _pkg_resources = __import__('pkg_resources', globals(), locals(), [], -1)
            resource_filename = _pkg_resources.resource_filename
            package = 'bacon.' + self.module
            dll_dir = os.path.dirname(resource_filename(package, self.files[0]))
            for file in self.files:
                if not os.path.exists(os.path.join(dll_dir, file)):
                    raise ImportError('pkg_resources returned incorrect directory')
                if os.path.dirname(resource_filename(package, file)) != dll_dir:
                    raise ValueError('Supporting DLLs extracted to inconsistent directory')
            return dll_dir
        except ImportError:
            pass

        # In PyInstaller --onefile mode, use sys._MEIPASS temporary
        # directory to find local files
        try:
            return sys._MEIPASS
        except AttributeError:
            pass
        
        # In frozen executable, assume DLLs are alongside executable
        try:
            if sys.frozen:
                return os.path.dirname(sys.executable)
        except AttributeError:
            pass

        # In development installation, or if pkg_resources not available, DLLs
        # are in their directory    
        return os.path.join(os.path.dirname(__file__), self.module)

    def get_lib(self):
        dll_dir = self.get_dir()
        if sys.platform == 'win32':
            # Dependent DLLs loaded by Bacon.dll also need to be loaded from this path, use
            # SetDllDirectory to affect the library search path; requires XP SP 1 or Vista.
            windll.kernel32.SetDllDirectoryA(dll_dir.encode('utf-8'))
            dll = self.files[0]
        else:
            dll = os.path.join(dll_dir, self.files[0])
        try:
            return cdll.LoadLibrary(dll)
        except:
            logger.exception('dll_dir is %s' % dll_dir)
            raise ImportError('Failed to load DLL %s' % dll)

if sys.platform == 'win32':
    if sizeof(c_void_p) == 4:
        _dll_path = _DllPath('windows32', [
            'Bacon.dll',
            'libEGL.dll',
            'libGLESv2.dll',
            'd3dcompiler_46.dll',
            'msvcp110.dll',
            'msvcr110.dll',
            'vccorlib110.dll',
        ])
    else:
        _dll_path = _DllPath('windows64', [
            'Bacon.dll',
            'libEGL.dll',
            'libGLESv2.dll',
            'd3dcompiler_46.dll',
            'msvcp110.dll',
            'msvcr110.dll',
            'vccorlib110.dll',
        ])
elif sys.platform == 'darwin':
    if sizeof(c_void_p) == 4:
        _dll_path = _DllPath('darwin32', [
            'Bacon.dylib',
        ])
    else:
        _dll_path = _DllPath('darwin64', [
            'Bacon64.dylib',
        ])
else:
    _dll_path = None

class MockCDLL(object):
    def __call__(self, *args, **kwargs):
        return self

    def __getattr__(self, name):
        return self

def mock_function_wrapper(fn, *args):
    return fn

def load(function_wrapper = None):    
    if 'BACON_MOCK_NATIVE' in os.environ and os.environ['BACON_MOCK_NATIVE']:
        _lib = MockCDLL()
        fn = mock_function_wrapper
        can_init = False
    elif not _dll_path:
        raise ImportError('Unsupported platform %s' % sys.platform)
    else:
        _lib = _dll_path.get_lib()
        fn = create_fn(function_wrapper)
        can_init = True

    # Function types
    LogCallback = CFUNCTYPE(None, c_int, c_char_p)
    TickCallback = CFUNCTYPE(None)
    WindowResizeEventHandler = CFUNCTYPE(None, c_int, c_int)
    KeyEventHandler = CFUNCTYPE(None, c_int, c_int)
    MouseButtonEventHandler = CFUNCTYPE(None, c_int, c_int)
    MouseScrollEventHandler = CFUNCTYPE(None, c_float, c_float)
    ControllerConnectedEventHandler = CFUNCTYPE(None, c_int, c_int)
    ControllerButtonEventHandler = CFUNCTYPE(None, c_int, c_uint, c_int)
    ControllerAxisEventHandler = CFUNCTYPE(None, c_int, c_int, c_float)
    VoiceCallback = CFUNCTYPE(None)
    EnumShaderUniformsCallback = CFUNCTYPE(None, c_int, c_int, c_char_p, c_int, c_int, c_void_p)

    # Functions
    Init = fn(_lib.Bacon_Init)
    Run = fn(_lib.Bacon_Run)
    Stop = fn(_lib.Bacon_Stop)
    GetVersion = fn(_lib.Bacon_GetVersion, POINTER(c_int), POINTER(c_int), POINTER(c_int))
    Shutdown = fn(_lib.Bacon_Shutdown)
    SetLogCallback = fn(_lib.Bacon_SetLogCallback, LogCallback)
    SetLogLevel = fn(_lib.Bacon_SetLogLevel, c_int)
    SetTickCallback = fn(_lib.Bacon_SetTickCallback, TickCallback)

    SetWindowResizeEventHandler = fn(_lib.Bacon_SetWindowResizeEventHandler, WindowResizeEventHandler)
    GetWindowSize = fn(_lib.Bacon_GetWindowSize, POINTER(c_int), POINTER(c_int))
    SetWindowSize = fn(_lib.Bacon_SetWindowSize, c_int, c_int)
    SetWindowTitle = fn(_lib.Bacon_SetWindowTitle, c_char_p)
    SetWindowResizable = fn(_lib.Bacon_SetWindowResizable, c_int)
    GetWindowContentScale = fn(_lib.Bacon_GetWindowContentScale, POINTER(c_float))
    SetWindowContentScale = fn(_lib.Bacon_SetWindowContentScale, c_float)
    SetWindowFullscreen = fn(_lib.Bacon_SetWindowFullscreen, c_int)

    CreateShader = fn(_lib.Bacon_CreateShader, POINTER(c_int), c_char_p, c_char_p)
    EnumShaderUniforms = fn(_lib.Bacon_EnumShaderUniforms, c_int, EnumShaderUniformsCallback, c_void_p)
    SetShaderUniform = fn(_lib.Bacon_SetShaderUniform, c_int, c_int, c_void_p, c_int)
    CreateSharedShaderUniform = fn(_lib.Bacon_CreateSharedShaderUniform, POINTER(c_int), c_char_p, c_int, c_int)
    SetSharedShaderUniform = fn(_lib.Bacon_SetSharedShaderUniform, c_int, c_void_p, c_int)

    CreateImage = fn(_lib.Bacon_CreateImage, POINTER(c_int), c_int, c_int, c_int)
    LoadImage = fn(_lib.Bacon_LoadImage, POINTER(c_int), c_char_p, c_int)
    GetImageRegion = fn(_lib.Bacon_GetImageRegion, POINTER(c_int), c_int, c_int, c_int, c_int, c_int)

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
    SetFrameBuffer = fn(_lib.Bacon_SetFrameBuffer, c_int, c_float)
    SetViewport = fn(_lib.Bacon_SetViewport, c_int, c_int, c_int, c_int)
    SetShader = fn(_lib.Bacon_SetShader, c_int)
    SetBlending = fn(_lib.Bacon_SetBlending, c_int, c_int)
    DrawImage = fn(_lib.Bacon_DrawImage, c_int, c_float, c_float, c_float, c_float)
    DrawImageRegion = fn(_lib.Bacon_DrawImageRegion, c_int, c_float, c_float, c_float, c_float, c_float, c_float, c_float, c_float)
    DrawLine = fn(_lib.Bacon_DrawLine, c_float, c_float, c_float, c_float)
    DrawRect = fn(_lib.Bacon_DrawRect, c_float, c_float, c_float, c_float)
    FillRect = fn(_lib.Bacon_FillRect, c_float, c_float, c_float, c_float)

    LoadFont = fn(_lib.Bacon_LoadFont, POINTER(c_int), c_char_p)
    UnloadFont = fn(_lib.Bacon_UnloadFont, c_int)
    GetDefaultFont = fn(_lib.Bacon_GetDefaultFont, POINTER(c_int))
    GetFontMetrics = fn(_lib.Bacon_GetFontMetrics, c_int, c_float, POINTER(c_int), POINTER(c_int))
    GetGlyph = fn(_lib.Bacon_GetGlyph, c_int, c_float, c_int, c_int, POINTER(c_int), POINTER(c_int), POINTER(c_int), POINTER(c_int))

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

    ExecuteCommands = fn(_lib.Bacon_ExecuteCommands, POINTER(c_int), c_int, POINTER(c_float), c_int)

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
