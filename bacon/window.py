from ctypes import *
import sys
import os

import bacon
from bacon.core import lib
from bacon import native

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

        if not native._mock_native:
            width = c_int()
            height = c_int()
            lib.GetWindowSize(byref(width), byref(height))
            self._width = width.value
            self._height = height.value
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

#: The singleton :class:`Window` instance.
window = Window()

def _window_resize_event_handler(width, height):
    window._width = width
    window._height = height
    bacon._current_game.on_resize(width, height)