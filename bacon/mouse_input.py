from ctypes import *

import bacon
from bacon.core import lib
from bacon import native

MouseButtons = native.MouseButtons

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

    bacon._current_game.on_mouse_button(button, value)

def _mouse_scroll_event_handler(dx, dy):
    bacon._current_game.on_mouse_scroll(dx, dy)