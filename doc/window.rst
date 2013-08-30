.. currentmodule:: bacon


Windowing
---------

Bacon games have a single window within which they do all their drawing.  This window exists even if the game
is fullscreen.  To customize the window, set properties on the :data:`bacon.window` object, which is the only instance
of the :class:`Window` class.

Title
=====

You should set the title of the window to the name of your game.

.. note:: TODO title example

Window size
===========

At the very least you should set the size of the window your game should run in before calling :func:`run`::

	bacon.window.width = 640
	bacon.window.height = 480

By default the window will not be resizable by the user (but your code can modfy the window size at any time).  To allow the user to resize the window, set :attr:`Window.resizable` to ``True``.

Fullscreen
==========

Use the :attr:`Window.fullscreen` property to switch in and out of fullscreen mode::

.. note:: TODO fullscreen example

Target
======

Usually games are designed for a specific target resolution, and it is difficult to adapt to a larger or smaller window.  Making the window non-resizable will certainly fix the window size to the desired dimensions, however this doesn't help if you want to also support fullscreen, since different computers will have different screen resolutions.

The :attr:`window.target` property can be used in this situation.  Set to an image of the desired dimensions, this image will be used as the default render target, and will be drawn into the window with scaling and letterboxing as appropriate.

The following example is a game designed for exactly 512x512 resolution, but the window is both resizable and
able to be set to fullscreen.  Note the ``atlas=0`` argument passed to the offscreen image constructor: this is necessary as the target image cannot be used if it shares a texture atlas with the images and fonts you will be drawing.

.. literalinclude:: ../examples/window_target.py
	:emphasize-lines: 4

If your game is designed with a "pixel-art" style, consider creating the target with ``sample_nearest=True`` and ``content_scale=1`` arguments in the :class:`Image` constructor to preserve sharp edges between pixels.