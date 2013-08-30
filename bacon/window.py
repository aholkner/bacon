from ctypes import *
import sys
import os

import bacon
from bacon.core import lib
from bacon import native
from bacon import graphics

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
        self._target = None

        if not native._mock_native:
            width = c_int()
            height = c_int()
            lib.GetWindowSize(byref(width), byref(height))
            self._width = width.value
            self._height = height.value

            content_scale = c_float()
            lib.GetWindowContentScale(byref(content_scale))
            self._content_scale = content_scale.value

            self.title = os.path.basename(sys.argv[0])

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

    def _get_target(self):
        return self._target
    def _set_target(self, target):
        self._target = target
    target = property(_get_target, _set_target, doc='''Optional image to use as the default render target.  

        If set, all rendering will be to this image, which will appear scaled and letterboxed if necessary 
        in the center of the window.  :attr:`width`, :attr:`height` and :attr:`content_scale` will return 
        the dimensions of this target instead of the window dimensions.

        :type: :class:`Image`''')

    def _get_content_scale(self):
        return self._content_scale
    def _set_content_scale(self, content_scale):
        lib.SetWindowContentScale(content_scale)
        self._content_scale = content_scale
    content_scale = property(_get_content_scale, _set_content_scale, doc='''The scaling factor applied 
        to the window.  On Windows this is always 1.0.  On OS X with a retina display attached,
        ``content_scale`` will default to 2.0.  

        Fonts and offscreen render targets are created at this content scale by default, to match the
        pixel density.

        You can explicitly set ``content_scale`` to 1.0, disabling the high-resolution framebuffer.  You
        should do so before loading any assets.

        :type: float
        ''')
        
#: The singleton :class:`Window` instance.
window = Window()

def _window_resize_event_handler(width, height):
    window._width = width
    window._height = height
    bacon._current_game.on_resize(width, height)

_window_frame_target = None
def _begin_frame():
    global _window_frame_target
    _window_frame_target = window._target
    if _window_frame_target:
        graphics.push_target(_window_frame_target)

def _end_frame():
    global _window_frame_target
    if _window_frame_target:
        graphics.pop_target()
        graphics.clear(0, 0, 0, 1)
        graphics.set_color(1, 1, 1, 1)
        target_aspect = _window_frame_target._width / _window_frame_target._height
        window_aspect = window._width / window._height
        if target_aspect > window_aspect:
            width = window._width
            height = width / target_aspect
        else:
            height = window._height
            width = height * target_aspect
        x = int(window._width / 2 - width / 2)
        y = int(window._height / 2 - height / 2)
        graphics.draw_image(_window_frame_target, x, y, x + width, y + width)
        _window_frame_target = None