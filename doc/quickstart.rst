An example game
---------------

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

.. note:: TODO build up this example incrementally to have more game in it.