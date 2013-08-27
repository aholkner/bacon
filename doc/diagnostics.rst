Diagnostics
-----------

Logging
=======

Bacon logs all messages through the Python ``logging`` module to the ``"bacon"`` logger.  If no logging configuration is setup, bacon will log warning and error messages to the console, and informational messages to a file in the current directory named ``bacon.log``.

You can change this behaviour by setting up logging handler for the ``"bacon"`` module before importing bacon.  The following example suppresses all logging, including the creation of the ``bacon.log`` file.

.. literalinclude:: ../examples/suppress_logging.py
	:emphasize-lines: 2,3