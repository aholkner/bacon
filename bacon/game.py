from ctypes import *
import time

import bacon
from bacon.core import lib
from bacon import native
from bacon import commands
from bacon import controller
from bacon import keyboard
from bacon import graphics
from bacon import mouse_input
from bacon import shader
from bacon import window

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

if not native._mock_native:
    _time_uniform = shader.ShaderUniform('g_Time', shader.ShaderUniformType.float_)

#: Number of seconds since the last frame.  This is a convenience value for timing animations.
bacon.timestep = 0.0

def _first_tick_callback():
    global _tick_callback_handle
    global _last_frame_time
    global _start_time
    global timestep

    _start_time = time.time()
    _last_frame_time = _start_time
    bacon.timestep = 0.0

    _tick_callback_handle = lib.TickCallback(_tick_callback)
    lib.SetTickCallback(_tick_callback_handle)

    # Exceptions on startup (either during on_init or the first on_tick) stop the
    # game loop immediately, since they're likely to be showstoppers.
    try:
        bacon._current_game.on_init()

        _last_frame_time = time.time() - _start_time
        _tick_callback()
    except:
        _tick_callback_handle = lib.TickCallback(_error_tick_callback)
        lib.SetTickCallback(_tick_callback_handle)
        raise

def _error_tick_callback():
    lib.Stop()

def _tick_callback():
    global _last_frame_time
    
    now_time = time.time() - _start_time
    bacon.timestep = now_time - _last_frame_time
    _last_frame_time = now_time

    _time_uniform.value = now_time

    graphics._target_stack = [None]
    window._begin_frame()
    mouse_input.mouse._update_position()

    try:
        bacon._current_game.on_tick()
    except:
        _tick_callback_handle = lib.TickCallback(_error_tick_callback)
        lib.SetTickCallback(_tick_callback_handle)
        raise

    window._end_frame()
    commands.flush()

bacon._current_game = None

def run(game):
    '''Start running the game.  The window is created and shown at this point, and then
    the main event loop is entered.  'game.on_tick' and other event handlers are called
    repeatedly until the game exits.

    If a game is already running, this function replaces the :class:`Game` instance that
    receives events.
    '''
    if bacon._current_game:
        bacon._current_game = game
        return

    global _tick_callback_handle
    bacon._current_game = game

    # Window handler
    window_resize_callback_handle = lib.WindowResizeEventHandler(window._window_resize_event_handler)
    lib.SetWindowResizeEventHandler(window_resize_callback_handle)

    # Key handler
    key_callback_handle = lib.KeyEventHandler(keyboard._key_event_handler)
    lib.SetKeyEventHandler(key_callback_handle)

    # Mouse handlers
    mouse_button_callback_handle = lib.MouseButtonEventHandler(mouse_input._mouse_button_event_handler)
    lib.SetMouseButtonEventHandler(mouse_button_callback_handle)
    mouse_scroll_callback_handle = lib.MouseScrollEventHandler(mouse_input._mouse_scroll_event_handler)
    lib.SetMouseScrollEventHandler(mouse_scroll_callback_handle)

    # Controller handlers
    controller_connected_handle = lib.ControllerConnectedEventHandler(controller._controller_connected_event_handler)
    lib.SetControllerConnectedEventHandler(controller_connected_handle)
    controller_button_handle = lib.ControllerButtonEventHandler(controller._controller_button_event_handler)
    lib.SetControllerButtonEventHandler(controller_button_handle)
    controller_axis_handle = lib.ControllerAxisEventHandler(controller._controller_axis_event_handler)
    lib.SetControllerAxisEventHandler(controller_axis_handle)

    # Tick handler
    _tick_callback_handle = lib.TickCallback(_first_tick_callback)
    lib.SetTickCallback(_tick_callback_handle)

    lib.Run()
    bacon._current_game = None
    _tick_callback_handle = None

    lib.SetWindowResizeEventHandler(lib.WindowResizeEventHandler(0))
    lib.SetKeyEventHandler(lib.KeyEventHandler(0))
    lib.SetMouseButtonEventHandler(lib.MouseButtonEventHandler(0))
    lib.SetMouseScrollEventHandler(lib.MouseScrollEventHandler(0))
    lib.SetControllerConnectedEventHandler(lib.ControllerConnectedEventHandler(0))
    lib.SetControllerButtonEventHandler(lib.ControllerButtonEventHandler(0))
    lib.SetControllerAxisEventHandler(lib.ControllerAxisEventHandler(0))
    lib.SetTickCallback(lib.TickCallback(0))
    
def quit():
    '''Stop the game loop and exit the application before the next :func:`on_tick` is called.
    '''
    lib.Stop()
