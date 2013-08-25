Distribution
------------

You can use `PyInstaller <http://www.pyinstaller.org/>`_ to create standalone ``.exe`` (Windows) and ``.app`` (OS X) applications.  These are a great way to distribute your game to fans who may not have the technical expertise (or inclination) to install Python and other dependencies.

.. note:: Use the development version of PyInstaller, the current release (2.0) is too old.  PyInstaller does not currently support installation via PIP or ``setup.py``, you'll need to just extract it and run it from its directory.

PyInstaller must be told how to package Bacon; this is done with the following ``bacon-hooks.py`` file (also included in the distributed Bacon package):

.. literalinclude:: ../bacon/hook-bacon.py

The following is an example ``.spec`` file that PyInstaller can use to build the ``bouncing_balls`` example.  You can modify it to use your game's name and resources.  You must also change ``hookspath`` to point to the directory containing the above ``hook-bacon.py``.

.. literalinclude:: ../examples/bouncing_balls.spec

To build run Pyinstaller with this spec file::

	$ python /path/to/pyinstaller/PyInstaller.py bouncing_balls.spec --window

The resulting file will be built in a new ``dist`` directory.