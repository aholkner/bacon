.. module:: bacon

Events
------

You handle events by overriding methods in the :class:`Game` class.  Typically your game will be structured with a instance of a
subclass of :class:`Game`, which overrides all the event methods you wish to handle:

.. autoclass:: Game

Bacon also stores the state of all input devices; so for example, you can query whether a particular key is pressed or not at any
point in your program, without having to handle the event.

Frame Tick
==========

Every frame Bacon sends the :func:`Game.on_tick` event.  You must override this method, as it is the only way to update and render
the game.

.. automethod:: Game.on_tick

Keyboard
========

The :func:`Game.on_key` function is called when a key is pressed or released:

.. literalinclude:: ../examples/key_event.py

.. automethod:: Game.on_key

Alternatively, you can check the :data:`keys` set at any time to determine if a key is currently pressed.  The following example clears
the screen to green as long as the spacebar is pressed:

.. literalinclude:: ../examples/keys.py

.. autodata:: keys

See the :doc:`keys` for a complete list of supported key codes.

Mouse
=====

The :func:`Game.on_mouse_button` function is called when a mouse button is pressed or released:

.. literalinclude:: ../examples/mouse_button_event.py

.. automethod:: Game.on_mouse_button

The :func:`Game.on_mouse_scroll` function is called whenever the mouse wheel is scrolled:

.. literalinclude:: ../examples/mouse_scroll_event.py

.. automethod:: Game.on_mouse_scroll

The current position of the mouse, and the state of its buttons, can be queried from the :data:`bacon.mouse`
object:

.. literalinclude:: ../examples/mouse.py

.. autoclass:: Mouse
	:members:

Game Controllers
================

To support the use of game controllers, first listen for the :func:`Game.on_controller_connected` event.  Check the passed 
in :class:`Controller` instance for the set of buttons and axes it supports.  Then either listen for 
:func:`Game.on_controller_button` and :func:`Game.on_controller_axis` events, or check the state of the controller's
buttons and axes on the controller instance itself.

.. literalinclude:: ../examples/controller.py

.. automethod:: Game.on_controller_connected
.. automethod:: Game.on_controller_disconnected
.. automethod:: Game.on_controller_button
.. automethod:: Game.on_controller_axis

.. autoclass:: Controller
	:members:

.. autoclass:: ControllerButtons
	:members:
	:undoc-members:

.. autoclass:: ControllerAxes
	:members:
	:undoc-members:

.. autoclass:: ControllerProfiles
	:members:
	:undoc-members: