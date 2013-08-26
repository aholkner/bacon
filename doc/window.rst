.. currentmodule:: bacon


Windowing
---------

Bacon games have a single window within which they do all their drawing.  This window exists even if the game
is fullscreen.  To customize the window, set properties on the :data:`bacon.window` object, which is the only instance
of the :class:`Window` class.

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

Title
=====

You should set the title of the window to the name of your game.

.. note:: TODO title example
