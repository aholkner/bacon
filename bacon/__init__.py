from bacon import native
from bacon.readonly_collections import ReadOnlyDict
from ctypes import *
import os
import logging
import time

logger = logging.getLogger(__name__)

_mock_native = native._mock_native

# Convert return codes into exceptions.
class BaconError(Exception):
    def __init__(self, error_code):
        self.error_code = error_code

    def __repr__(self):
        return '%s(%d)' % (self.__class__.__name__, self.error_code)

    _error_classes = {}

    @classmethod
    def _register_error_class(cls, error_code, error_class):
        cls._error_classes[error_code] = error_class

    @classmethod
    def _from_error_code(cls, error_code):
        try:
            error_class = cls._error_classes[error_code]
        except KeyError:
            error_class = BaconError
        return error_class(error_code)

def error_code(error_code):
    def wrap(cls):
        BaconError._register_error_class(error_code, cls)
        return cls
    return wrap

@error_code(native.ErrorCodes.unknown)
class UnknownError(BaconError):
    pass

@error_code(native.ErrorCodes.invalid_argument)
class InvalidArgumentError(BaconError):
    pass

@error_code(native.ErrorCodes.invalid_handle)
class InvalidHandleError(BaconError):
    pass

@error_code(native.ErrorCodes.stack_underflow)
class StackUnderflowError(BaconError):
    pass

@error_code(native.ErrorCodes.unsupported_format)
class UnsupportedFormatError(BaconError):
    pass

@error_code(native.ErrorCodes.shader_compile_error)
class ShaderCompileError(BaconError):
    pass

@error_code(native.ErrorCodes.shader_link_error)
class ShaderLinkError(BaconError):
    pass

@error_code(native.ErrorCodes.not_rendering)
class NotRenderingError(BaconError):
    pass

@error_code(native.ErrorCodes.invalid_font_size)
class InvalidFontSizeError(BaconError):
    pass

@error_code(native.ErrorCodes.not_looping)
class NotLoopingError(BaconError):
    pass

def _error_wrapper(fn):
    def f(*args):
        result = fn(*args)
        if result != native.ErrorCodes.none:
            raise BaconError._from_error_code(result)
    return f

lib = native.load(function_wrapper = _error_wrapper)

_log_level_map = {
    native.LogLevels.trace: logging.DEBUG,
    native.LogLevels.info: logging.INFO,
    native.LogLevels.warning: logging.WARNING,
    native.LogLevels.error: logging.ERROR,
    native.LogLevels.fatal: logging.FATAL,
}

def _log_callback(level, message):
    try:
        level = _log_level_map[level]
    except KeyError:
        level = logging.ERROR
    logger.log(level, message.decode('utf-8'))

# Initialize library now
if not _mock_native:
    _log_callback_handle = lib.LogCallback(_log_callback)
    lib.SetLogCallback(_log_callback_handle)
    lib.Init()

    # Expose library version
    major_version = c_int()
    minor_version = c_int()
    patch_version = c_int()
    lib.GetVersion(byref(major_version), byref(minor_version), byref(patch_version))
    major_version = major_version.value     #: Major version number of the Bacon dynamic library that was loaded, as an integer.
    minor_version = minor_version.value     #: Minor version number of the Bacon dynamic library that was loaded, as an integer.
    patch_version = patch_version.value     #: Patch version number of the Bacon dynamic library that was loaded, as an integer.
else:
    major_version, minor_version, patch_version = (0, 1, 0)

#: Version of the Bacon dynamic library that was loaded, in the form ``"major.minor.patch"``.
version = '%d.%d.%d' % (major_version, minor_version, patch_version)

BlendFlags = native.BlendFlags
ShaderUniformType = native.ShaderUniformType
ControllerProfiles = native.ControllerProfiles
ControllerButtons = native.ControllerButtons
ControllerAxes = native.ControllerAxes
Keys = native.Keys
MouseButtons = native.MouseButtons

class Game(object):
    '''Base class for all Bacon games.  An instance of this class is passed to :func:`run`.  Override methods on
    this class to handle game events such as :func:`on_tick`.  A complete example of a game::

        class MyGame(bacon.Game):
            def on_tick(self):
                # Update and draw game here.
                pass

        # Start the game
        bacon.run(MyGame())

    '''

    def on_init(self):
        '''Called once when the game starts.  You can use this to do any initialization that
        requires the graphics device to have been initialized; for example, rendering to a texture.'''
        pass
    
    def on_tick(self):
        '''Called once per frame to update and render the game.  You may only call
        drawing functions within the scope of this method.'''
        clear(1, 0, 1, 1)

    def on_key(self, key, pressed):
        '''Called when a key on the keyboard is pressed or released.

        :param key: key code, one of :class:`Keys` enumeration
        :param pressed: ``True`` if the key was pressed, otherwise ``False``
        '''
        pass

    def on_mouse_button(self, button, pressed):
        '''Called when a mouse button is pressed or released.

        :param button: button index, of :class:`MouseButton` enumeration
        :param pressed: ``True`` if the button was pressed, otherwise ``False``
        '''
        pass

    def on_mouse_scroll(self, dx, dy):
        '''Called when the mouse scroll wheel is scrolled.  Most mice have a scroll wheel that moves in
        the ``y`` axis only; Apple trackpads and mice support scrolling in ``x`` as well.

        :note: units are aribitrary and not currently consistent across platforms

        :param dx: relative scroll amount along the ``x`` axis
        :param dy: relative scroll amount along the ``y`` axis
        '''
        pass

    def on_resize(self, width, height):
        '''Called when size of the window changes.

        :param width: width of the drawable area of the window, in pixels
        :param height: height of the drawable area of the window, in pixels
        '''
        pass

    def on_controller_connected(self, controller):
        '''Called when a game controller is connected.

        :param controller: the :class:`Controller` that is now available for polling and events
        '''
        pass

    def on_controller_disconnected(self, controller):
        '''Called when a game controller is disconnected.  You should use the `controller` parameter only
        to identify a previously used controller; its properties and values will no longer be available.

        :param controller: the :class:`Controller` that was disconnected
        '''
        pass

    def on_controller_button(self, controller, button, pressed):
        '''Called when a button on a game controller is pressed or released.

        :param controller: the :class:`Controller` containing the button
        :param button: button index, of :class:`ControllerButtons` enumeration
        :param pressed: ``True`` if the button was pressed, otherwise ``False``
        '''
        pass

    def on_controller_axis(self, controller, axis, value):
        '''Called when an axis on a game controller is moved.

        :param controller: the :class:`Controller` containing the axis
        :param button: axis index, of :class:`ControllerAxes` enumeration
        :param value: absolute position of the axis, between ``-1.0`` and ``1.0``
        '''
        pass

class Shader(object):
    '''A GPU shader object that can be passed to :func:`set_shader`.

    The default shader is as follows, and demonstrates the available vertex attributes and
    uniforms::

        default_shader = Shader(vertex_source=
                                """
                                precision highp float;
                                attribute vec3 a_Position;
                                attribute vec2 a_TexCoord0;
                                attribute vec4 a_Color;
                                
                                varying vec2 v_TexCoord0;
                                varying vec4 v_Color;
                                
                                uniform mat4 g_Projection;
                                
                                void main()
                                {
                                    gl_Position = g_Projection * vec4(a_Position, 1.0);
                                    v_TexCoord0 = a_TexCoord0;
                                    v_Color = a_Color;
                                }
                                """,
                                fragment_source=
                                """
                                precision highp float;
                                uniform sampler2D g_Texture0;
                                varying vec2 v_TexCoord0;
                                varying vec4 v_Color;
                                
                                void main()
                                {
                                    gl_FragColor = v_Color * texture2D(g_Texture0, v_TexCoord0);
                                }
                                """)

    The shading language is OpenGL-ES SL 2.  The shader will be translated automatically into
    HLSL on Windows, and into GLSL on other desktop platforms.

    :param vertex_source: string of source code for the vertex shader
    :param fragment_source: string of source code for the fragment shader
    '''
    def __init__(self, vertex_source, fragment_source):
        self._vertex_source = vertex_source
        self._fragment_source = fragment_source

        handle = c_int()
        lib.CreateShader(byref(handle), vertex_source.encode('utf-8'), fragment_source.encode('utf-8'))
        self._handle = handle.value

        self._uniforms = {}

        enum_uniform_callback = lib.EnumShaderUniformsCallback(self._on_enum_uniform)
        lib.EnumShaderUniforms(handle, enum_uniform_callback, None)

        self._uniforms = ReadOnlyDict(self.uniforms)

    def _on_enum_uniform(self, shader, uniform, name, type, array_count, arg):
        name = name.decode('utf-8')
        self._uniforms[name] = ShaderUniform(self, uniform, name, type, array_count)

    @property
    def uniforms(self):
        '''Map of shader uniforms available on this shader.

        :type: read-only dictionary of string to :class:`ShaderUniform`
        '''
        return self._uniforms

    @property
    def vertex_source(self):
        '''Get the vertex shader source

        :type: ``str``
        '''
        return self._vertex_source

    @property
    def fragment_source(self):
        '''Get the fragment shader source

        :type: ``str``
        '''
        return self._fragment_source

class _ShaderUniformNativeType(object):
    def __init__(self, ctype, converter=None):
        self.ctype = ctype
        if converter:
            self.converter = converter
        elif hasattr(ctype, '_length_'):
            self.converter = lambda x: ctype(*x)
        else:
            self.converter = ctype


_shader_uniform_native_types = {
    native.ShaderUniformType.float_:    _ShaderUniformNativeType(c_float),
    native.ShaderUniformType.vec2:      _ShaderUniformNativeType(c_float * 2),
    native.ShaderUniformType.vec3:      _ShaderUniformNativeType(c_float * 3),
    native.ShaderUniformType.vec4:      _ShaderUniformNativeType(c_float * 4),
    native.ShaderUniformType.int_:      _ShaderUniformNativeType(c_int),
    native.ShaderUniformType.ivec2:     _ShaderUniformNativeType(c_int * 2),
    native.ShaderUniformType.ivec3:     _ShaderUniformNativeType(c_int * 3),
    native.ShaderUniformType.ivec4:     _ShaderUniformNativeType(c_int * 4),
    native.ShaderUniformType.bool_:     _ShaderUniformNativeType(c_int),
    native.ShaderUniformType.bvec2:     _ShaderUniformNativeType(c_int * 2),
    native.ShaderUniformType.bvec3:     _ShaderUniformNativeType(c_int * 3),
    native.ShaderUniformType.bvec4:     _ShaderUniformNativeType(c_int * 4),
    native.ShaderUniformType.mat2:      _ShaderUniformNativeType(c_float * 4),
    native.ShaderUniformType.mat3:      _ShaderUniformNativeType(c_float * 9),
    native.ShaderUniformType.mat4:      _ShaderUniformNativeType(c_float * 16),
    native.ShaderUniformType.sampler2D: _ShaderUniformNativeType(c_int, lambda image : c_int(image._handle))
}

class ShaderUniform(object):
    '''A uniform variable assocated with a single shader.

    :see: :attr:`Shader.uniforms`
    '''
    def __init__(self, shader, uniform, name, type, array_count):
        self._shader_handle = shader._handle
        self._uniform_handle = uniform
        self._name = name
        self._type = type
        self._array_count = array_count
        self._value = None

        try:
            native_type = _shader_uniform_native_types[type]
        except KeyError:
            raise ValueError('Unsupported shader uniform type %s' % native.ShaderUniformType.tostring(type))

        if array_count > 1:
            ctype = native_type.ctype
            converter = native_type.converter
            self._converter = lambda v: (ctype * array_count)(*(converter(x) for x in v))
        else:
            self._converter = native_type.converter
            
    def __repr__(self):
        return 'ShaderUniform(%d, %s, %s, %d)' % (self._shader_handle, self.name, native.ShaderUniformType.tostring(self.type), self.array_count)

    def _get_value(self):
        return self._value
    def _set_value(self, value):
        self._value = value
        native_value = self._converter(value)
        lib.SetShaderUniform(self._shader_handle, self._uniform_handle, byref(native_value), sizeof(native_value))
    value = property(_get_value, _set_value, doc='''Current value of the uniform as seen by the shader.

        The type of the value depends on the type of the uniform:

        * ``float``, ``int``, ``bool``: their equivalent Python types
        * ``vec[2-4]``, ``ivec[2-4]``, ``bvec[2-4]``: a sequence of equivalent Python types (e.g., a sequence of 3 floats for ``vec3``)
        * ``mat2``, ``mat3``, ``mat4``: a sequence of 4, 9 or 16 floats, respectively
        * ``sampler2D``: an :class:`Image`

        For uniform arrays, the value is a sequence of the above types.  For example, a uniform of type ``vec2[3]`` can be assigned::

            value = ((0, 1), (2, 3), (4, 5))

        ''')

    @property
    def name(self):
        '''Name of the uniform, as it appears in the shader.

        :type: ``str``
        '''
        return self._name

    @property
    def type(self):
        '''Type of the uniform, or, if the uniform is an array, the element type of the array.

        :type: an enumeration of :class:`ShaderUniformType`
        '''
        return self._type

    @property
    def array_count(self):
        '''The size of the array, or 1 if this uniform is not an array.

        :type: ``int``
        '''
        return self._array_count

class Image(object):
    '''An image that can be passed to :func:`draw_image` and other rendering functions.

    There are two forms to the `Image` constructor.  The first loads an image from a file::

        Image(file, premultiply_alpha=True, discard_bitmap=True)

    The other creates an empty image which can then be rendered to using :func:`set_frame_buffer`::

        Image(width, height)

    There may be GPU limits on the maximum size of an image that can be uploaded to the GPU (typically 2048x2048 is
    a safe upper bound for current generation mobile devices; desktops may support up to 8192x8192).  There is no
    diagnostic if the device limit is exceeded, the image will not render.

    Images are retained by the renderer until :func:`unload` is explicitly called.

    :param file: path to an image file to load.  Supported formats include PNG, JPEG, BMP, TIF, etc.
    :param premultiply_alpha: if ``True`` (the default), the color channels are multiplied by the alpha channel when 
        image is loaded.  This allows the image to be alpha blended with bilinear interpolation between texels correctly.
        This paramater should be set to ``False`` if the original image data is required and won't be blended (for example, if 
        it will be used as a mask).
    :param discard_bitmap: if ``True`` (the default), the bitmap data backing the image will be discarded after
        a GPU texture has been created (which happens automatically the first time the image is rendered).  This saves
        memory.  The parameter should be set to ``False`` if the source data will be required for reasons besides rendering
        (there is currently no API for using an image this way).
    :param width: width of the image to create, in texels
    :param height: height of the image to creat, in texels
    '''

    def __init__(self, file=None, premultiply_alpha=True, discard_bitmap=True, width=None, height=None, handle=None):
        if file:
            # Load image from file
            if handle:
                raise ValueError('`handle` is not a not valid argument if `file` is given')

            flags = 0
            if premultiply_alpha:
                flags |= native.ImageFlags.premultiply_alpha
            if discard_bitmap:
                flags |= native.ImageFlags.discard_bitmap

            handle = c_int()
            lib.LoadImage(byref(handle), file.encode('utf-8'), flags)
            handle = handle.value
            
            if not width or not height:
                width = c_int()
                height = c_int()
                lib.GetImageSize(handle, byref(width), byref(height))
                width = width.value
                height = height.value    

        elif width and height and not handle:
            # Create empty image of given dimensions
            handle = c_int()
            lib.CreateImage(byref(handle), width, height)
            handle = handle.value

        if not handle:
            raise ValueError('invalid arguments to Image, must specify either `file` or `width` and `height`')

        self._handle = handle
        self._width = width
        self._height = height

    def unload(self):
        '''Releases renderer resources associated with this image.'''
        lib.UnloadImage(self._handle)
        self._handle = -1

    @property
    def width(self):
        ''' The width of the image, in texels (read-only).'''
        return self._width

    @property
    def height(self):
        ''' The height of the image, in texels (read-only).'''
        return self._height

class FontMetrics(object):
    '''Aggregates pixel metrics for a font loaded at a particular size.  See :attr:`Font.metrics`

    :param ascent: ascent of the font, in pixels
    :param ascent: descent of the font, in pixels
    '''

    def __init__(self, ascent, descent):
        self._ascent = ascent
        self._descent = descent

    @property
    def descent(self):
        '''Descent of the font below the baseline, in pixels; typically positive'''
        return self._descent

    @property
    def ascent(self):
        '''Ascent of the font above the baseline, in pixels; typically negative'''
        return self._ascent

class Glyph(object):
    '''Image and metrics for rendering a character in a particular font.

    :param char: the character (a string) this glyph will render
    :param image: the :class:`Image` of the glyph
    :param offset_x: offset in pixels to draw the image relative to the baseline origin
    :param offset_y: offset in pixels to draw the image relative to the baseline origin
    :param advance: horizontal advance, in pixels
    '''
    def __init__(self, char, image, offset_x, offset_y, advance):
        self._char = char
        self._image = image
        self._offset_x = offset_x
        self._offset_y = offset_y
        self._advance = advance

    @property
    def char(self):
        '''The character (a string) this glyph will render'''
        return self._char

    @property
    def image(self):
        '''The :class:`Image` of the glyph'''
        return self._image

    @property
    def offset_x(self):
        '''Offset in pixels to draw the image relative to the baseline origin'''
        return self._offset_x

    @property
    def offset_y(self):
        '''Offset in pixels to draw the image relative to the baseline origin'''
        return self._offset_y

    @property
    def advance(self):
        '''Horizontal advance, in pixels'''
        return self._advance

class _FontFile(object):
    _font_files = {}

    def __init__(self, file):
        handle = c_int()
        lib.LoadFont(byref(handle), file.encode('utf-8'))
        self._handle = handle.value

    def unload(self):
        lib.UnloadFont(self._handle)
        self._handle = -1

    def get_metrics(self, size):
        ascent = c_float()
        descent = c_float()
        lib.GetFontMetrics(self._handle, size, byref(ascent), byref(descent))
        return FontMetrics(-round(ascent.value), -round(descent.value))

    def get_glyph(self, size, char):
        image_handle = c_int()
        offset_x = c_float()
        offset_y = c_float()
        advance = c_float()
        lib.GetGlyph(self._handle, size, ord(char), 
            byref(image_handle), byref(offset_x), byref(offset_y), byref(advance))

        if image_handle.value:
            width = c_int()
            height = c_int()
            lib.GetImageSize(image_handle, byref(width), byref(height))
            image = Image(width = width.value, height = height.value, handle = image_handle.value)
        else:
            image = None

        return Glyph(char, image, round(offset_x.value), round(offset_y.value), round(advance.value))

    @classmethod
    def get_font_file(cls, file):
        try:
            return cls._font_files[file]
        except KeyError:
            ff = _FontFile(file)
            cls._font_files[file] = ff
            return ff

class Font(object):
    '''A font that can be used for rendering text.  Fonts are loaded from TrueType (``.ttf``) files at a
    particular point size::

        font = Font('res/DejaVuSans.ttf', 24.0)

    Note that the point size of a font has a complex relationship to its height in pixels, affected by the
    design of the font, grid-fitting, and device DPI (always 96 in Bacon).  Once a font is loaded, its
    `metrics` can be queried to get the font's design measurements in pixels.

    Fonts are never unloaded.

    :param file: path to a font file to load.  Supported formats include TrueType, OpenType, PostScript, etc.
    :param size: the point size to load the font at
    '''
    def __init__(self, file, size):
        if type(file) is _FontFile:
            self._font_file = file
        else:
            self._font_file = _FontFile.get_font_file(file)
        self._size = size
        self._glyphs = { }

        self._metrics = self._font_file.get_metrics(size)

    @property
    def metrics(self):
        '''Retrieves the pixel-space design metrics of the font.

        :return: :class:`FontMetrics`
        '''
        return self._metrics

    @property
    def ascent(self):
        '''The ascent of the font above the baseline, in pixels; typically negative.'''
        return self._metrics.ascent

    @property
    def descent(self):
        '''The descent of the font below the baseline, in pixels; typically positive.'''
        return self._metrics.descent

    def get_glyph(self, char):
        '''Retrieves a :class:`Glyph` that renders the given character.

        :param char: the character (a string)
        '''
        try:
            return self._glyphs[char]
        except KeyError:
            glyph = self._font_file.get_glyph(self._size, char)
            self._glyphs[char] = glyph
            return glyph

    def get_glyphs(self, str):
        '''Retrieves a list of :class:`Glyph` for the given string.

        :param str: the string to render
        '''
        return [self.get_glyph(c) for c in str]

class GlyphLayout(object):
    '''Caches a layout of glyphs rendering a given string with bounding rectangle, layout metrics.

    :note: Incomplete, word-wrapping and alignment are not implemented.
    '''
    def __init__(self, glyphs, width=None):
        self.lines = [glyphs]
        # TODO word-wrapping

class Sound(object):
    '''Loads a sound from disk.  Supported formats are WAV (``.wav``) and Ogg Vorbis (``.ogg``).

    Sounds can be played immediately with the :func:`play` method; or used as the parameter to a new
    :class:`Voice`, if control over the playback is required.

    Sounds can be streamed by specifying ``stream=True``; this causes them to load faster but incur a small
    latency on playback.  Background music should always be streamed.  Sound effects should not be streamed.

    Sounds are kept in memory until explicitly unloaded, see :func:`unload`.

    :param file: path to the sound file to load.  The sound format is deduced from the file extension.
    :param stream: if ``True``, the sound is streamed from disk; otherwise it is fully cached in memory.
    '''
    def __init__(self, file, stream=False):
        flags = 0
        if stream:
            flags |= native.SoundFlags.stream

        if file.lower().endswith('.wav'):
            flags |= native.SoundFlags.format_wav
        elif file.lower().endswith('.ogg'):
            flags |= native.SoundFlags.format_ogg

        handle = c_int()
        lib.LoadSound(byref(handle), file.encode('utf-8'), flags)
        self._handle = handle.value

    def unload(self):
        '''Release all resources associated with the sound.'''
        lib.UnloadSound(self._handle)
        self._handle = -1

    def play(self, gain=None, pan=None, pitch=None):
        '''Play the sound as a `one-shot`.

        The sound will be played to completion.  If the sound is played more than once at a time, it will mix
        with all previous instances of itself.  If you need more control over the playback of sounds, see
        :class:`Voice`.

        :param gain: optional volume level to play the sound back at, between 0.0 and 1.0 (defaults to 1.0)
        :param pan: optional stereo pan, between -1.0 (left) and 1.0 (right)
        :param pitch: optional sampling rate modification, between 0.4 and 16.0, where 1.0 represents the original pitch
        '''
        if gain is None and pan is None and pitch is None:
            lib.PlaySound(self._handle)
        else:
            voice = Voice(self)
            if gain is not None:
                voice.gain = gain
            if pan is not None:
                voice.pan = pan
            if pitch is not None:
                voice.pitch = pitch
            voice.play()

class Voice(object):
    '''Handle to a single instance of a sound.  Voices can be used to:

    - Modify the parameters of a sound while it's playing (for example, its gain, pan, or pitch)
    - Pause and resume playback
    - Determine the current playback head position and seek to a different position
    - Provide a callback for when the sound finishes playing
    - Create and control a sound that loops automatically

    A voice is created to play back a single sound instance once (besides any looping), and cannot be reused to play 
    another sound, or even to play the same sound once playback of that sound completes.

    Do not use a ``Voice`` to play simple sound effects that do not require control during playing; see :func:`Sound.play`.

    Create a voice by first loading the sound, then creating a voice to play it::

        engine_sound = bacon.Sound('res/Engine.wav')
        engine_voice = bacon.Voice(engine_sound)
        engine_voice.play()

    Specify the ``loop`` parameter to play music that loops until stopped::

        music_sound = bacon.Sound('res/MyMusic.ogg', stream=True)
        music_voice = bacon.Voice(music_sound, loop=True)
        music_voice.play()

    Voices become invalid when their sound completes (after their callback returns, if one is set).  Continuing to use
    a voice after it has finished will result in an exception; you can check the state of a voice with :attr:`playing`.

    Paused voices remain in memory until explicitly destroyed with :func:`destroy`.

    :param sound: a :class:`Sound` to play
    :param loop: if ``True``, the voice will be set to looping playback
    '''
    _gain = 1.0
    _pitch = 1.0
    _pan = 0.0
    _callback = None

    def __init__(self, sound, loop=False):
        flags = 0
        if loop:
            flags |= native.VoiceFlags.loop

        handle = c_int()
        lib.CreateVoice(byref(handle), sound._handle, flags)
        self._handle = handle

    def destroy(self):
        '''Destroy a voice.  Required if a sound is stopped before completing to free associated resources.
        '''
        lib.DestroyVoice(self._handle)
        self._handle = -1

    def play(self):
        '''Being or resume playing the sound.'''
        lib.PlayVoice(self._handle)

    def stop(self):
        '''Pause playback of the sound.'''
        lib.StopVoice(self._handle)

    def _get_gain(self):
        return self._gain
    def _set_gain(self, gain):
        self._gain = gain
        lib.SetVoiceGain(self._handle, gain)
    gain = property(_get_gain, _set_gain, doc='''Get or set the gain (volume) of the sound, between 0.0 and 1.0.  Defaults to 1.0''')

    def _get_pitch(self):
        return self._pitch
    def _set_pitch(self, pitch):
        self._pitch = pitch
        lib.SetVoicePitch(self._handle, pitch)
    pitch = property(_get_pitch, _set_pitch, doc='''Get or set the pitch (sample rate) of the sound, between 0.4 and 16.0, with 1.0 being normal playback speed.''')

    def _get_pan(self):
        return self._pan
    def _set_pan(self, pan):
        self._pan = pan
        lib.SetVoicePan(self._handle, pan)
    pan = property(_get_pan, _set_pan, doc='''Get or set the stereo pan of the sound, between -1.0 and 1.0.''')

    def _is_playing(self):
        playing = c_int()
        lib.IsVoicePlaying(self._handle, byref(playing))
        return playing.value
    def _set_playing(self, playing):
        if playing:
            self.play()
        else:
            self.stop()
    playing = property(_is_playing, _set_playing, doc='''``True`` if the sound is currently playing, ``False`` if it has finished.''')

    def set_loop_points(self, start_sample=-1, end_sample=0):
        '''Set the loop points within the sound.

        The sound must have been created with ``loop=True``.  The default parameters cause the loop points to be set to
        the entire sound duration.

        :note: There is currently no API for converting sample numbers to times.
        :param start_sample: sample number to loop back to
        :param end_sample: sample number to loop at
        '''
        lib.SetVoiceLoopPoints(self._handle, start_sample, end_sample)

    def _get_callback(self):
        return self._callback
    def _set_callback(self, callback):
        self._callback = callback
        self._callback_handle = lib.VoiceCallback(callback)
        lib.SetVoiceCallback(self._handle, self._callback_handle)
    callback = property(_get_callback, _set_callback, doc='''Set a callback function to be called when the sound finishes.  The function takes no arguments.''')

    def _get_position(self):
        position = c_int()
        lib.GetVoicePosition(self._handle, position)
        return position.value
    def _set_position(self, position):
        lib.SetVoicePosition(self._handle, position)
    position = property(_get_position, _set_position, doc='''Get or set the current sample position within the sound.  

        :note: There is currently no API for converting sample numbers to times.
        ''')

#: Set of keyboard keys that are currently pressed.  Each element is a enumerator of :class:`Keys`.
keys = set()

def _key_event_handler(key, value):
    if value:
        keys.add(key)
    elif key in keys:
        keys.remove(key)

    _game.on_key(key, value)

class Mouse(object):
    '''Exposes functions for the system mouse or trackpad.

    Do not construct an instance of this class, use the singleton :data:`mouse`.
    '''

    def __init__(self):
        self.button_mask = 0
        #: Location of the mouse in the window along the X axis, in pixels.
        self.x = 0
        #: Location of the mouse in the window along the Y axis, from top to bottom, in pixels.
        self.y = 0

    def _update_position(self):
        x = c_float()
        y = c_float()
        lib.GetMousePosition(byref(x), byref(y))
        self.x = x.value
        self.y = y.value

    @property
    def left(self):
        '''``True`` if the left mouse button is currently pressed.'''
        return self.button_mask & (1 << 0)

    @property
    def middle(self):
        '''``True`` if the middle mouse button is currently pressed.'''
        return self.button_mask & (1 << 1)

    @property
    def right(self):
        '''``True`` if the right mouse button is currently pressed.'''
        return self.button_mask & (1 << 2)

#: State of the system mouse or trackpad; an instance of :class:`Mouse`.
#:
#: For example, to query the current mouse position within the window::
#:
#:      x, y = mouse.x, mouse.y
#:
#: To query if the left mouse button is currently pressed::
#:
#:      if mouse.left:
#:          pass
#:
mouse = Mouse()

def _mouse_button_event_handler(button, value):
    if value:
        mouse.button_mask |= (1 << button)
    else:
        mouse.button_mask &= ~(1 << button)

    _game.on_mouse_button(button, value)

def _mouse_scroll_event_handler(dx, dy):
    _game.on_mouse_scroll(dx, dy)

class Window(object):
    '''Properties of the game window.

    The window is constructed automatically when :func:`run` is called.  The :data:`window` singleton
    provides access to the members of this class both before and after ``run`` is called.

    For example, to set up some common window properties for a game::

        bacon.window.title = 'Destiny of Swords'
        bacon.window.width = 800
        bacon.window.height = 600

    All properties can be modified at runtime, for example to toggle in and out of fullscreen.
    '''
    def __init__(self):
        self._width = -1
        self._height = -1
        self._resizable = False
        self._fullscreen = False

        if not _mock_native:
            width = c_int()
            height = c_int()
            lib.GetWindowSize(byref(width), byref(height))
            self._width = width.value
            self._height = height.value
            self.title = 'Bacon'

    def _get_width(self):
        return self._width
    def _set_width(self, width):
        lib.SetWindowSize(width, self._height)
        self._width = width
    width = property(_get_width, _set_width, doc='''Get or set the width of the drawable part of the window, in pixels.''')

    def _get_height(self):
        return self._height
    def _set_height(self, height):
        lib.SetWindowSize(self._width, height)
        self._height = height
    height = property(_get_height, _set_height, doc='''Get or set the height of the drawable part of the window, in pixels.''')

    def _get_title(self):
        return self._title
    def _set_title(self, title):
        lib.SetWindowTitle(title.encode('utf-8'))
        self._title = title
    title = property(_get_title, _set_title, doc='''Get or set the title of the window (a string)''')

    def _is_resizable(self):
        return self._resizable
    def _set_resizable(self, resizable):
        lib.SetWindowResizable(resizable)
        self._resizable = resizable
    resizable = property(_is_resizable, _set_resizable, doc='''If ``True`` the window can be resized and maximized by the user.  See :func:`Game.on_resize`.''')

    def _is_fullscreen(self):
        return self._fullscreen
    def _set_fullscreen(self, fullscreen):
        lib.SetWindowFullscreen(fullscreen)
        self._fullscreen = fullscreen
    fullscreen = property(_is_fullscreen, _set_fullscreen, doc='''Set to ``True`` to make the game fullscreen, ``False`` to play in a window.''')

#: The singleton :class:`Window` instance.
window = Window()

def _window_resize_event_handler(width, height):
    window._width = width
    window._height = height
    _game.on_resize(width, height)

def _get_controller_property_string(controller_index, property):
    buffer = create_string_buffer(256)
    length = c_int(sizeof(buffer))
    try:
        lib.GetControllerPropertyString(controller_index, property, buffer, byref(length))
    except:
        return None
    return buffer.value.decode('utf-8')

def _get_controller_property_int(controller_index, property):
    value = c_int()
    try:
        lib.GetControllerPropertyInt(controller_index, property, byref(value))
    except:
        return None
    return value.value
    
class Controller(object):
    '''Represents the state of a connected game controller.  ``Controller`` instances are created automatically when
    a physical game controller device is detected, and destroyed when they are disconnected.  To obtain a reference
    to a ``Controller``, override :func:`Game.on_controller_connected`.
    '''

    def __init__(self, controller_index):
        self._controller_index = controller_index
        
        # Controller state
        self._buttons = 0
        self._axes = {}

        # Controller properties
        self._supported_axes_mask = _get_controller_property_int(controller_index, native.ControllerProperties.supported_axes_mask)
        self._supported_buttons_mask = _get_controller_property_int(controller_index, native.ControllerProperties.supported_buttons_mask)

        for bit in range(16):
            self._axes[1 << bit] = 0.0

        #: A numeric vendor ID that can be used to identify the type of device, when combined with the :attr:`product_id`.
        self.vendor_id = _get_controller_property_int(controller_index, native.ControllerProperties.vendor_id)

        #: A numeric product ID that can be used to identify the type of device, when combined with the :attr:`vendor_id`.
        self.product_id = _get_controller_property_int(controller_index, native.ControllerProperties.product_id)

        #: A human-readable name of the device that can be used to identify it to the user.
        self.name = _get_controller_property_string(controller_index, native.ControllerProperties.name)

        #: The supported profile of the device, a member of the enumeration :class:`GameControllerProfiles`.  Game controllers
        #: with the ``generic`` profile do not provide semantic meanings to any of the buttons or axes, and must usually be configured
        #: by the player unless the device type is known by the game.
        #:
        #: The ``standard`` profile describes a game controller with no analogue sticks or triggers.
        #:
        #: The ``extended`` profile describes a game controller with a layout compatible with an Xbox 360 controller.
        #:
        #: For a game controller to appear in the ``standard`` or ``extended`` profiles, it must have been discovered by a
        #: suitable SDK on the host platform: on Windows, this is XInput, which only supports the Xbox 360 controller.  On
        #: OS X and iOS, this is the GameController SDK, which supports a limited range of new devices.
        self.profile = _get_controller_property_int(controller_index, native.ControllerProperties.profile)

        self.mapping = ControllerMapping.get(self)
        if self.mapping:
            self.profile = self.mapping.profile

    @property
    def controller_index(self):
        '''The index of the controller, between 0 and 4 (read-only).  Typically this is assigned in the order the
        controllers are detected, however some controllers may have an intrinsic "player number" that is exposed
        through this number.  No two controllers will have the same controller index.
        '''
        return self._controller_index

    def has_axis(self, axis):
        '''Returns ``True`` if the controller has the requested axis, a value of enumeration :class:`ControllerAxes`.
        '''
        return (axis & self._supported_axes_mask) != 0

    def get_axis(self, axis):
        '''Get the absolute value of the requested axis.

        :param axis: An axis, one of the values in :class:`ControllerAxes`.
        :return: The absolute position of the axis, between -1.0 and 0.0.
        '''
        try:
            return self._axes[axis]
        except KeyError:
            return 0.0

    def has_button(self, button):
        '''Returns ``True`` if the controller has the requested button, a value of enumeration :class:`ControllerButtons`.
        '''
        return (button & self._supported_buttons_mask) != 0

    def get_button_state(self, button):
        '''Get the pressed state of the requested button.

        :param button: A button, one of the values in :class:`ControllerButtons`.
        :return: ``True``` if the button is currently pressed, otherwise ``False``.
        '''
        return (button & self._buttons) != 0

    @property
    def connected(self):
        '''``True`` if the controller is still connected; ``False`` if it has been disconnected.

        Once a controller has been disconnected, it is never reconnected (if the same physical device is reconnected, a new
        ``Controller`` instance is created).
        '''
        return self._controller_index is not None

    # Axes

    @property
    def left_thumb_x(self):
        '''The absolute X axis value of the left thumb-stick, or the main stick on a joystick.'''
        return self.get_axis(ControllerAxes.left_thumb_x)

    @property
    def left_thumb_y(self):
        '''The absolute Y axis value of the left thumb-stick, or the main stick on a joystick.'''
        return self.get_axis(ControllerAxes.left_thumb_y)

    @property
    def right_thumb_x(self):
        '''The absolute X axis value of the right thumb-stick, or the twist axis on a joystick.'''
        return self.get_axis(ControllerAxes.right_thumb_x)

    @property
    def right_thumb_y(self):
        '''The absolute Y axis value of the right thumb-stick, or the throttle control on a joystick.'''
        return self.get_axis(ControllerAxes.right_thumb_y)

    @property
    def left_trigger(self):
        '''The absolute left trigger value, between 0.0 and 1.0.  Available only on game controllers with the
        ``extended`` profile.
        '''
        return self.get_axis(ControllerAxes.left_trigger)

    @property
    def right_trigger(self):
        '''The absolute right trigger value, between 0.0 and 1.0.  Available only on game controllers with the
        ``extended`` profile.
        '''
        return self.get_axis(ControllerAxes.right_trigger)

    # Buttons

    @property
    def start(self):
        '''``True`` if the start button is pressed.  Available only on game controllers with the ``standard`` or 
        ``extended`` profiles.
        '''
        return self._buttons & ControllerButtons.start

    @property
    def back(self):
        '''``True`` if the back button is pressed.  Available only on the Xbox 360 controller on Windows.'''
        return self._buttons & ControllerButtons.back

    @property
    def select(self):
        '''``True`` if the select button is pressed.'''
        return self._buttons & ControllerButtons.select

    @property
    def action_up(self):
        '''``True`` if the up action button ("Y" on Xbox 360) is pressed.  Available only on game controllers with 
        the ``standard`` or ``extended`` profiles.'''
        return self._buttons & ControllerButtons.action_up

    @property
    def action_down(self):
        '''``True`` if the down action button ("A" on Xbox 360) is pressed.  Available only on game controllers with 
        the ``standard`` or ``extended`` profiles.'''
        return self._buttons & ControllerButtons.action_down

    @property
    def action_left(self):
        '''``True`` if the left action button ("X" on Xbox 360) is pressed.  Available only on game controllers with 
        the ``standard`` or ``extended`` profiles.'''
        return self._buttons & ControllerButtons.action_left

    @property
    def action_right(self):
        '''``True`` if the right action button ("B" on Xbox 360) is pressed.  Available only on game controllers with 
        the ``standard`` or ``extended`` profiles.'''
        return self._buttons & ControllerButtons.action_right

    @property
    def dpad_up(self):
        '''``True`` if the up directional pad button is pressed.  The d-pad also corresponds to the POV or hat control
        on a joystick.'''
        return self._buttons & ControllerButtons.dpad_up

    @property
    def dpad_down(self):
        '''``True`` if the down directional pad button is pressed.  The d-pad also corresponds to the POV or hat control
        on a joystick.'''
        return self._buttons & ControllerButtons.dpad_down

    @property
    def dpad_left(self):
        '''``True`` if the left directional pad button is pressed.  The d-pad also corresponds to the POV or hat control
        on a joystick.'''
        return self._buttons & ControllerButtons.dpad_left

    @property
    def dpad_right(self):
        '''``True`` if the right directional pad button is pressed.  The d-pad also corresponds to the POV or hat control
        on a joystick.'''
        return self._buttons & ControllerButtons.dpad_right

    @property
    def left_shoulder(self):
        '''``True`` if the left shoulder button is pressed.  Available only on game controllers with the ``standard`` or 
        ``extended`` profiles.
        '''
        return self._buttons & ControllerButtons.left_shoulder

    @property
    def right_shoulder(self):
        '''``True`` if the right shoulder button is pressed.  Available only on game controllers with the ``standard`` or 
        ``extended`` profiles.
        '''
        return self._buttons & ControllerButtons.right_shoulder

    @property
    def left_thumb(self):
        '''``True`` if the left thumb stick is depressed.  Available only on game controllers with the 
        ``extended`` profile.
        '''
        return self._buttons & ControllerButtons.left_thumb

    @property
    def right_thumb(self):
        '''``True`` if the right thumb stick is depressed.  Available only on game controllers with the 
        ``extended`` profile.
        '''
        return self._buttons & ControllerButtons.right_thumb

class ControllerMapping(object):
    '''Map buttons and axes on a device controller to game button and axes.  Typically used to map generic
    inputs to semantic inputs associated with a controller profile.

    :param buttons: dictionary mapping button to button, each key and value is either a string or enum from :class:`GameControllerButtons`
    :param axes: dictionary mapping axis to axis, each key and value is either a string or enum from :class:`GameControllerAxes`
    :param dead_zones: dictionary mapping axis to its dead zone.  Axis input is rescaled to zero out +/- the dead zone.
    :param profile: the mapped profile, a string or enum from :class:`GameControllerProfiles`
    '''

    # Registry of (vendor_id, product_id) tuples to ControllerMapping
    _registry = {}

    @classmethod
    def register(cls, vendor_id, product_id, mapping):
        '''Register a mapping for controllers with the given vendor and product IDs.  The mapping will
        replace any existing mapping for these IDs for controllers not yet connected.

        :param vendor_id: the vendor ID of the controller, as reported by :attr:`Controller.vendor_id`
        :param product_id: the vendor ID of the controller, as reported by :attr:`Controller.product_id`
        :param mapping: a :class:`ControllerMapping` to apply
        '''
        cls._registry[(vendor_id, product_id)] = mapping

    @classmethod
    def get(cls, controller):
        '''Find a mapping that can apply to the given controller.  Returns None if unsuccessful.

        :param controller: :class:`Controller` to look up
        :return: :class:`ControllerMapping`
        '''
        try:
            return cls._registry[(controller.vendor_id, controller.product_id)]
        except KeyError:
            return None

    def __init__(self, buttons={}, axes={}, dead_zones={}, profile=ControllerProfiles.generic):
        self.buttons = {}
        for k, v in buttons.items():
            k = ControllerButtons.parse(k)
            v = ControllerButtons.parse(v)
            self.buttons[k] = v
        self.axes = {}
        for k, v in axes.items():
            k = ControllerAxes.parse(k)
            v = ControllerAxes.parse(v)
            self.axes[k] = v
        self.dead_zones = {}
        for k, v in dead_zones.items():
            k = ControllerAxes.parse(k)
            self.dead_zones[k] = float(v)
        self.profile = ControllerProfiles.parse(profile)

# Import known controller mappings
import bacon.controllers

# Map controller index to controller
_controllers = {}

def _controller_connected_event_handler(controller_index, connected):
    # Invalidate any existing controller with the same index
    controller = None
    try:
        controller = _controllers[controller_index]
        controller._controller_index = None
    except KeyError:
        pass

    if connected:
        _controllers[controller_index] = controller = Controller(controller_index)
        _game.on_controller_connected(controller)
    else:
        del _controllers[controller_index]
        _game.on_controller_disconnected(controller)
        
def _controller_button_event_handler(controller_index, button, pressed):
    try:
        controller = _controllers[controller_index]
    except KeyError:
        return

    if controller.mapping:
        try:
            button = controller.mapping.buttons[button]
        except KeyError:
            pass

    if pressed:
        controller._buttons |= button
    else:
        controller._buttons &= ~button

    _game.on_controller_button(controller, button, pressed)

def _controller_axis_event_handler(controller_index, axis, value):    
    try:
        controller = _controllers[controller_index]
    except KeyError:
        return

    if controller.mapping:
        try:
            axis = controller.mapping.axes[axis]
        except KeyError:
            pass

        try:
            dead_zone = controller.mapping.dead_zones[axis]
            if value < -dead_zone:
                value = (value + dead_zone) / (1.0 - dead_zone)
            elif value > dead_zone:
                value = (value - dead_zone) / (1.0 - dead_zone)
            else:
                value = 0.0
            
        except KeyError:
            pass

    if controller._axes[axis] != value:
        controller._axes[axis] = value
        _game.on_controller_axis(controller, axis, value)

#: Number of seconds since the last frame.  This is a convenience value for timing animations.
timestep = 0.0

def _first_tick_callback():
    global _tick_callback_handle
    global _last_frame_time
    global timestep

    _last_frame_time = time.time()
    timestep = 0.0

    _tick_callback_handle = lib.TickCallback(_tick_callback)
    lib.SetTickCallback(_tick_callback_handle)

    _game.on_init()
    _tick_callback()

def _tick_callback():
    global _last_frame_time
    global timestep

    now_time = time.time()
    timestep = now_time - _last_frame_time
    _last_frame_time = now_time

    mouse._update_position()
    _game.on_tick()

_game = None

def run(game):
    '''Start running the game.  The window is created and shown at this point, and then
    the main event loop is entered.  'game.on_tick' and other event handlers are called
    repeatedly until the game exits.
    '''
    global _game
    global _tick_callback_handle
    _game = game

    # Window handler
    window_resize_callback_handle = lib.WindowResizeEventHandler(_window_resize_event_handler)
    lib.SetWindowResizeEventHandler(window_resize_callback_handle)

    # Key handler
    key_callback_handle = lib.KeyEventHandler(_key_event_handler)
    lib.SetKeyEventHandler(key_callback_handle)

    # Mouse handlers
    mouse_button_callback_handle = lib.MouseButtonEventHandler(_mouse_button_event_handler)
    lib.SetMouseButtonEventHandler(mouse_button_callback_handle)
    mouse_scroll_callback_handle = lib.MouseScrollEventHandler(_mouse_scroll_event_handler)
    lib.SetMouseScrollEventHandler(mouse_scroll_callback_handle)

    # Controller handlers
    controller_connected_handle = lib.ControllerConnectedEventHandler(_controller_connected_event_handler)
    lib.SetControllerConnectedEventHandler(controller_connected_handle)
    controller_button_handle = lib.ControllerButtonEventHandler(_controller_button_event_handler)
    lib.SetControllerButtonEventHandler(controller_button_handle)
    controller_axis_handle = lib.ControllerAxisEventHandler(_controller_axis_event_handler)
    lib.SetControllerAxisEventHandler(controller_axis_handle)

    # Tick handler
    _tick_callback_handle = lib.TickCallback(_first_tick_callback)
    lib.SetTickCallback(_tick_callback_handle)

    lib.Run()
    _game = None
    _tick_callback_handle = None

    lib.SetWindowResizeEventHandler(lib.WindowResizeEventHandler(0))
    lib.SetKeyEventHandler(lib.KeyEventHandler(0))
    lib.SetMouseButtonEventHandler(lib.MouseButtonEventHandler(0))
    lib.SetMouseScrollEventHandler(lib.MouseScrollEventHandler(0))
    lib.SetControllerConnectedEventHandler(lib.ControllerConnectedEventHandler(0))
    lib.SetControllerButtonEventHandler(lib.ControllerButtonEventHandler(0))
    lib.SetControllerAxisEventHandler(lib.ControllerAxisEventHandler(0))
    lib.SetTickCallback(lib.TickCallback(0))
    
# Graphics

if _mock_native:
    def push_transform():
        pass
else:
    push_transform = lib.PushTransform
push_transform.__doc__ = '''Save the current graphics transform by pushing it on the transform stack.  It can be restored by
calling :func:`pop_transform`.
'''

if _mock_native:
    def pop_transform():
        pass
else:
    pop_transform = lib.PopTransform
pop_transform.__doc__ = '''Restore a previously saved transform by popping it off the transform stack.
'''

if _mock_native:
    def translate(x, y):
        pass
else:
    translate = lib.Translate
translate.__doc__ = '''Translate the current graphics transform by ``(x, y)`` units.
'''

if _mock_native:
    def scale(sx, sy):
        pass
else:
    scale = lib.Scale
scale.__doc__ = '''Scale the current graphics transform by multiplying through ``(sx, sy)``.
'''

if _mock_native:
    def rotate(radians):
        pass
else:
    rotate = lib.Rotate
rotate.__doc__ = '''Rotate the current graphics transform by ``radians`` counter-clockwise.
'''

def set_transform(matrix):
    '''Replace the current graphics transform with the given 4x4 matrix.  For example, to replace
    the transform with a translation by ``(x, y)``::

        bacon.set_transform([1, 0, 0, x,
                             0, 1, 0, y,
                             0, 0, 1, 0,
                             0, 0, 0, 1])

    :param matrix: a 4x4 matrix in column major order, represented as a flat 16 element sequence.
    '''
    lib.SetTransform((c_float * 16)(*matrix))

if _mock_native:
    def push_color():
        pass
else:
    push_color = lib.PushColor
push_color.__doc__ = '''Save the current graphics color by pushing it on the color stack.  It can be restored with :func:`pop_color`.

The color stack is cleared at the beginning of each frame, and the default color reset to white.
'''

if _mock_native:
    def pop_color():
        pass
else:
    pop_color = lib.PopColor
pop_color.__doc__ = '''Restore a previously saved graphics color by popping it off the color stack.
'''

if _mock_native:
    def set_color(r, g, b, a):
        pass
else:
    set_color = lib.SetColor
set_color.__doc__ = '''Set the current graphics color to the given RGBA values.  Typically each component has a value between
0.0 and 1.0, however out-of-range values are permitted and may be used for special effects with an 
appropriate shader.

The color is reset to white at the beginning of each frame.
'''

if _mock_native:
    def multiply_color(r, g, b, a):
        pass
else:
    multiply_color = lib.MultiplyColor
multiply_color.__doc__ = '''multiply_color(r, g, b, a)

Multiplies the current graphics color component-wise by the given RGBA values.
'''

if _mock_native:
    def clear(r, g, b, a):
        pass
else:
    clear = lib.Clear
clear.__doc__ = '''Clear the current framebuffer to the given RGBA color.  Each color component must be in the range 0.0 to 1.0.
You should clear the default frame buffer at the beginning of each frame.  Failure to do so may cause visual
artifacts and/or poor performance on some platforms.
'''

def set_frame_buffer(image):
    '''Set the current frame buffer to the given image.  All subsequent drawing commands will be applied to the
    given image instead of the default frame buffer.  To resume rendering to the main frame buffer (the window),
    pass ``None`` as the image.

    The framebuffer is automatically reset to the default framebuffer at the beginning of each frame.

    :param image: the :class:`Image` to render to
    ''' 
    lib.SetFrameBuffer(image._handle if image else 0)

if _mock_native:
    def set_viewport(x, y, width, height):
        pass
else:
    set_viewport = lib.SetViewport
set_viewport.__doc__ = '''Set the current viewport in screen-space.  This affects both the GPU viewport, which provides a screen-space clip,
and the projection matrix, which is constructed from the viewport coordinates automatically.

Viewport coordinates are specified in pixel-space with (0, 0) at the upper-left corner.

The viewport is reset to the framebuffer dimensions at the beginning of each frame, and when switching framebuffers.
'''

def set_shader(shader):
    '''Set the current graphics shader.  All subsequent drawing commands will be rendered with this shader.
    To reset to the default shader, pass ``None`` as the argument.

    The shader is automatically reset to the default shader at the beginning of each frame.

    :param shader: a :class:`Shader` to render with
    '''
    lib.SetShader(shader._handle if shader else 0)

if _mock_native:
    def set_blending(src_blend, dest_blend):
        pass
else:
    set_blending = lib.SetBlending
set_blending.__doc__ = '''Set the current graphics blend mode.  All subsequent drawing commands will be rendered with this blend mode.

The default blend mode is ``(BlendFlags.one, BlendFlags.one_minus_src_alpha)``, which is appropriate for
premultiplied alpha.

:param src_blend: a value from the :class:`BlendFlags` enumeration, specifying the blend contribution from the source fragment
:param dest_blend: a value from the :class:`BlendFlags` enumeration, specifying the blend contribution from the destination fragment
'''

def draw_image(image, x1, y1, x2 = None, y2 = None):
    '''Draw an image.

    The image's top-left corner is drawn at ``(x1, y1)``, and its lower-left at ``(x2, y2)``.  If ``x2`` and ``y2`` are omitted, they
    are calculated to render the image at its native resoultion.

    Note that images can be flipped and scaled by providing alternative values for ``x2`` and ``y2``.

    :param image: an :class:`Image` to draw
    '''
    if x2 is None:
        x2 = x1 + image.width
    if y2 is None:
        y2 = y1 + image.height
    lib.DrawImage(image._handle, x1, y1, x2, y2)
def draw_image_region(image, x1, y1, x2, y2,
                      ix1, iy1, ix2, iy2):
    '''Draw a rectangular region of an image.

    The part of the image contained by the rectangle in texel-space by the coordinates ``(ix1, iy1)`` to ``(ix2, iy2)`` is
    drawn at coordinates ``(x1, y1)`` to ``(x2, y2)``.  All coordinates have the origin ``(0, 0)`` at the upper-left corner.

    For example, to draw the left half of a ``100x100`` image at coordinates ``(x, y)``::

        bacon.draw_image_region(image, x, y, x + 50, y + 100,
                                0, 0, 50, 100)

    :param image: an :class:`Image` to draw
    '''
    lib.DrawImage(image._handle, x1, y1, x2, y2, ix1, iy1, ix2, iy2)

if _mock_native:
    def draw_line(x1, y1, x2, y2):
        pass
else:
    draw_line = lib.DrawLine
draw_line.__doc__ = '''Draw a line from coordinates ``(x1, y1)`` to ``(x2, y2)``.

No texture is applied.
'''

def draw_string(font, text, x, y):
    '''Draw a string with the given font.

    :note: Text alignment and word-wrapping is not yet implemented.  The text is rendered with the left edge and
        baseline at ``(x, y)``.

    :param font: the :class:`Font` to render text with
    :param text: a string of text to render.
    '''
    glyphs = font.get_glyphs(text)
    glyph_layout = GlyphLayout(glyphs)
    draw_glyph_layout(glyph_layout, x, y)

def draw_glyph_layout(glyph_layout, x, y):
    '''Draw a prepared :class:`GlyphLayout` at the given coordinates.
    '''
    start_x = x
    for line in glyph_layout.lines:
        x = start_x
        for glyph in line:
            if glyph.image:
                draw_image(glyph.image, x + glyph.offset_x, y - glyph.offset_y)
            x += glyph.advance
