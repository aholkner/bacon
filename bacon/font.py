from ctypes import *

from bacon.core import lib
from bacon import native
from bacon import resource
import bacon.image

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
    _default_font_file = None

    def __init__(self, file, handle=None):
        if not handle:
            handle = c_int()
            lib.LoadFont(byref(handle), resource.get_resource_path(file).encode('utf-8'))
            self._handle = handle.value
        else:
            self._handle = handle

    def unload(self):
        lib.UnloadFont(self._handle)
        self._handle = -1

    def get_metrics(self, size):
        ascent = c_int()
        descent = c_int()
        lib.GetFontMetrics(self._handle, size, byref(ascent), byref(descent))
        return FontMetrics(-ascent.value, -descent.value)

    def get_glyph(self, size, content_scale, char, flags):
        image_handle = c_int()
        offset_x = c_int()
        offset_y = c_int()
        advance = c_int()
        lib.GetGlyph(self._handle, size * content_scale, ord(char), flags,
            byref(image_handle), byref(offset_x), byref(offset_y), byref(advance))

        if image_handle.value:
            width = c_int()
            height = c_int()
            lib.GetImageSize(image_handle, byref(width), byref(height))
            image = bacon.image.Image(width = width.value / content_scale, 
                                      height = height.value / content_scale, 
                                      handle = image_handle.value)
        else:
            image = None

        return Glyph(char, 
                     image, 
                     round(offset_x.value) / content_scale, 
                     round(offset_y.value) / content_scale, 
                     round(advance.value) / content_scale)

    @classmethod
    def get_font_file(cls, file):
        try:
            return cls._font_files[file]
        except KeyError:
            ff = _FontFile(file)
            cls._font_files[file] = ff
            return ff

    @classmethod
    def get_default_font_file(cls):
        if not cls._default_font_file:
            handle = c_int()
            lib.GetDefaultFont(byref(handle))
            cls._default_font_file = _FontFile(file=None, handle=handle)
        return cls._default_font_file

class Font(object):
    '''A font that can be used for rendering text.  Fonts are loaded from TrueType (``.ttf``) files at a
    particular point size::

        font = Font('res/DejaVuSans.ttf', 24.0)

    Note that the point size of a font has a complex relationship to its height in pixels, affected by the
    design of the font, grid-fitting, and device DPI (always 96 in Bacon).  Once a font is loaded, its
    `metrics` can be queried to get the font's design measurements in pixels.

    Fonts are never unloaded.

    :param str file: path to a font file to load.  Supported formats include TrueType, OpenType, PostScript, etc.  If ``None``, a default font is used
    :param float size: the point size to load the font at
    :param bool light_hinting: applies minimal autohinting to the outline; suitable for fonts designed 
        for OS X
    :param float content_scale: optional scaling factor for backing textures of glyphs.  Defaults to
        to :attr:`Window.content_scale`.
    '''
    def __init__(self, file, size, light_hinting=False, content_scale=None):
        if type(file) is _FontFile:
            self._font_file = file
        elif file is None:
            self._font_file = _FontFile.get_default_font_file()
        else:
            self._font_file = _FontFile.get_font_file(file)
        
        self._size = size

        if content_scale is None:
            content_scale = bacon.window.content_scale
        self._content_scale = content_scale
        
        self._glyphs = { }
        self._flags = 0

        if light_hinting:
            self._flags |= native.FontFlags.light_hinting

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
            glyph = self._font_file.get_glyph(self._size, self._content_scale, char, self._flags)
            self._glyphs[char] = glyph
            return glyph

    def get_glyphs(self, str):
        '''Retrieves a list of :class:`Glyph` for the given string.

        :param str: the string to render
        '''
        return [self.get_glyph(c) for c in str]