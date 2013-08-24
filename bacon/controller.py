from ctypes import *

import bacon
from bacon.core import lib
from bacon import native

ControllerProfiles = native.ControllerProfiles
ControllerButtons = native.ControllerButtons
ControllerAxes = native.ControllerAxes

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
        bacon._current_game.on_controller_connected(controller)
    else:
        del _controllers[controller_index]
        bacon._current_game.on_controller_disconnected(controller)
        
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

    bacon._current_game.on_controller_button(controller, button, pressed)

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
        bacon._current_game.on_controller_axis(controller, axis, value)