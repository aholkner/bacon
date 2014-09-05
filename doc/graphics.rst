.. currentmodule:: bacon

Graphics
--------

Bacon provides functions for loading and drawing images and fonts.  It presents a simple 2D API for this, but under the hood is using current generation GPU APIs, which allows you to use sophisticated effects.

While images and fonts can be loaded and queried at any time, the drawing functions can only be called within the scope of your game's :func:`Game.on_tick` method.  

The coordinate system is top-down, with (0, 0) in the top-left corner.  

At the beginning of each frame (before doing any other rendering in ``on_tick``), you should clear the screen:

.. note:: TODO clear example

Basic shapes
============

Rectangles can drawn or filled in the current color with :func:`draw_rect` and :func:`fill_rect`:

.. literalinclude:: ../examples/rect.py
    :emphasize-lines: 7,9

Triangles can be drawn or filled in the current color with :func:`draw_triangle` and :func:`fill_triangle`:

.. literalinclude:: ../examples/triangles.py
    :emphasize-lines: 8,10

Lines can similarly be drawn with :func:`draw_line`.

Images
======

Images can be loaded from most standard file types, for example PNG, BMP, TIFF, JPEG, etc.  First create an :class:`Image`, then draw it with :func:`draw_image` or :func:`draw_image_region`:

.. literalinclude:: ../examples/image.py
    :emphasize-lines: 3,4,9

Fonts
=====

To render text, first load a TrueType or OpenType font at a particular point size by constructing a :class:`Font`, then render it with :func:`draw_string`:

.. literalinclude:: ../examples/font.py
    :emphasize-lines: 3,4,9


Transform stack
===============

All drawing commands are transformed by the top matrix in a *transform stack* before being submitted for rendering.  By transforming
the top matrix of the transform stack, you affect the position of all subsequent drawing commands.  This can be convenient for rendering
groups of objects with a parent transform, and is necessary to perform rotations, scales and skews on drawing.

This example shows how the :func:`rotate` function can be used to rotate an image, using :func:`translate` to set move
the rotation pivot to the center of the image:

.. literalinclude:: ../examples/transform_rotate.py
    :emphasize-lines: 9,10

The above functions operate only on the top matrix of the transform stack.  You can save and restore the current transform with 
:func:`push_transform` and :func:`pop_transform`.  The following example shows how the rotation transform does not
affect the text rendered, only the image.

.. literalinclude:: ../examples/transform_push.py
    :emphasize-lines: 11,15

Color stack
===========

All drawing commands are tinted by the top color in a *color stack* when rendered.  By default the color is white (``(1, 1, 1, 1)``),
so no tinting is applied.  This example shows the image tinted red by modifying the color:

.. literalinclude:: ../examples/color.py
    :emphasize-lines: 8

By changing the alpha (the fourth color component), you can make images and text translucent.

Just like the transform stack, the color stack allows you to save and restore the current color, using :func:`push_color` and :func:`pop_color`.


Rendering to an image
=====================

By default all rendering is done to the main window.  You can render to an image instead using :func:`push_target`.  This is useful for compositing elements in a single layer for blending, and for certain
special effects.  Use :func:`pop_target` to resume rendering to the previous target (e.g. the window).

.. literalinclude:: ../examples/target.py
    :emphasize-lines: 25, 33

Shaders
=======

Shaders are small programs that run on the GPU to modify the drawing commands.  The shader is written in OpenGL-ES Shading
Language 2.0, and is supplied as two strings: one for the vertex program and one for the fragment program.  The following
example demonstrates a shader that adjusts the brightness and contrast of the image depending on the mouse position within
the window:

.. literalinclude:: ../examples/shader.py

Bacon requires that certain shader uniforms and attributes are named and typed appropriately for it to be able to submit
drawing commands:

* ``attribute vec3 a_Position``: transformed unprojected vertex position (i.e., in screen space with the transform stack applied)
* ``attribute vec2 a_TexCoord0``: texture coordinate for the image drawn with :func:`draw_image`
* ``attribute vec4 a_Color``: vertex color, as provided by :func:`set_color`

* ``uniform mat4 g_Projection``: projection matrix, typically mapping screen space to NDC
* ``uniform sampler2D g_Texture0``: texture for the image drawn with :func:`draw_image`
* ``uniform float g_Time``: number of seconds since the game started

These uniforms may not be set directly, however others you define in the shader can be manipulated through the :attr:`Shader.uniforms`
map as shown in the example above.  Note that the naming convention of shader uniforms is important: uniforms with names that begin with
``g_`` share their value across all shaders (for example, ``g_Projection`` and ``g_Texture0`` above).  Other uniforms have values that
must be set per-shader.


Blend modes
===========

The current blend state dictates how new elements are composited into the target.  The default blend mode is suitable
for compositing images and glyphs with premultiplied alpha; changing the blend mode with :func:`set_blending` can be used for special effects.  In the
following example, the same text is rendered over itself in different colors with an additive blend; where the colors 
intersect, the color adds to white:

.. literalinclude:: ../examples/blending.py
    :emphasize-lines: 8

