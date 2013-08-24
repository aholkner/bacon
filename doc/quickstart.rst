
Installation
------------

Bacon can be installed from the Python Package Index ::

	$ pip install -U bacon

See the `PIP documentation <http://www.pip-installer.org/en/latest/installing.html>`_ for more information on using PIP to install Python packages.

Bacon can work equally well without installation, by packaging its source anywhere in your ``PYTHONPATH``.  You can download the latest distribution from

* https://pypi.python.org/pypi/bacon


Minimum Requirements
^^^^^^^^^^^^^^^^^^^^

Bacon requires Python 2.7 or later, or Python 3.3 or later.  It works with both 32 and 64-bit versions of Python, on recent versions of OS X and Windows as detailed below:

OS X
%%%%

* Your game will require OS X 10.6 or later

* If your game requires an Xbox 360 controller, the user must install an `Xbox 360 Game Controller driver <http://tattiebogle.net/index.php/ProjectRoot/Xbox360Controller/OsxDriver>`_ , as the device does not register as a game controller by itself.

Windows
%%%%%%%

* Your game will require Windows Vista or later

.. note:: Bacon has not yet been tested on versions of Windows before Windows 7, and may require patches.

Example game
------------

Let's start with a simple example:

.. literalinclude:: ../examples/quickstart.py
   :linenos:

.. note:: This example is in the Bacon source distribution, in ``examples/quickstart.py``.

Reading from the end of the file backwards:

.. literalinclude:: ../examples/quickstart.py
   :lines: 13

This instructs Bacon to start running the game; i.e., to show the window, initialize the graphics driver, 
and start rendering frames and processing events.  All events are sent to the :class:`Game` class you
provide, including :func:`Game.on_tick`, which is called once per frame:

.. literalinclude:: ../examples/quickstart.py
   :pyobject: Game.on_tick

Other events, such as keyboard and mouse input, can be handled by overriding methods on :class:`Game`.

Typically all your animation, game logic and rendering is performed in ``on_tick``.