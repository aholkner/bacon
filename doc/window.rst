.. module:: bacon

Game
^^^^

Starting and stopping
---------------------

.. autofunction:: run
.. autofunction:: quit

Customizing the display
-----------------------

Bacon games have a single window within which they do all their drawing.  This window exists even if the game
is fullscreen.  To customize the window, set properties on the :data:`bacon.window` object, which is the only instance
of the :class:`Window` class.

.. autoclass:: Window

Window size
===========

At the very least you should set the size of the window your game should run in before calling :func:`run`::

	bacon.window.width = 640
	bacon.window.height = 480

.. autoattribute:: Window.width

.. autoattribute:: Window.height

By default the window will not be resizable by the user (but your code can modfy the window size at any time).

.. autoattribute:: Window.resizable

Fullscreen
==========

Use the ``bacon.window.fullscreen`` property to switch in and out of fullscreen mode.

.. autoattribute:: Window.fullscreen

Title
=====

You should set the title of the window to the name of your game.

.. autoattribute:: Window.title
