.. module:: bacon

Reference
---------

Audio
=====

.. autoclass:: Sound
    :members:
    
.. autoclass:: Voice
    :members:

Keyboard
========

.. class:: Keys

	See: :doc:`keys`

.. data:: keys
	
	Set of keyboard keys that are currently pressed.  Each element is a enumerator of :class:`Keys`.

Keyboard events:

* :func:`Game.on_key`

Game
====

.. autoclass:: Game
	:members:

.. autofunction:: run
.. autofunction:: quit

.. autodata:: timestep

Game Controllers
================

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

.. autoclass:: ControllerMapping
	:members:

Game controller events:

* :func:`Game.on_controller_connected`
* :func:`Game.on_controller_disconnected`
* :func:`Game.on_controller_button`
* :func:`Game.on_controller_axis`


Graphics
========

.. contents:: Contents
	:local:

Drawing
^^^^^^^

.. autofunction:: clear

.. autofunction:: draw_line
.. autofunction:: draw_rect
.. autofunction:: fill_rect

.. autofunction:: set_frame_buffer

.. autofunction:: set_blending

.. autoclass:: BlendFlags
    :members:
    :undoc-members:

Transform
^^^^^^^^^

.. autofunction:: translate
.. autofunction:: scale
.. autofunction:: rotate

.. autofunction:: push_transform
.. autofunction:: pop_transform
.. autofunction:: set_transform

Color
^^^^^

.. autofunction:: set_color
.. autofunction:: multiply_color
.. autofunction:: push_color
.. autofunction:: pop_color

Images
^^^^^^

.. autoclass:: Image
    :members:

.. autofunction:: draw_image

.. autofunction:: draw_image_region

Fonts
^^^^^

.. autoclass:: Font
    :members:

.. autofunction:: draw_string

Shaders
^^^^^^^

.. autofunction:: set_shader

.. autoclass:: Shader
    :members:

.. autoclass:: ShaderUniform
    :members:

.. autoclass:: ShaderUniformType
    :members:
    :undoc-members:


Mouse
=====

.. autoclass:: Mouse
	:members:

.. autoclass:: MouseButtons
	:members:
	:undoc-members:

.. autodata:: mouse

Mouse events:

* :func:`Game.on_mouse_button`
* :func:`Game.on_mouse_scroll`

Window
======

.. autoclass:: Window
	:members:
