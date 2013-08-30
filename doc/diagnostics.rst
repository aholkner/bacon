Diagnostics
-----------

Logging
=======

Bacon logs all messages through the Python ``logging`` module to the ``"bacon"`` logger.  If no logging configuration is setup, bacon will log warning and error messages to the console, and informational messages to a file in the current directory named ``bacon.log``.

You can change this behaviour by setting up logging handler for the ``"bacon"`` module before importing bacon.  The following example suppresses all logging, including the creation of the ``bacon.log`` file.

.. literalinclude:: ../examples/suppress_logging.py
	:emphasize-lines: 2,3

Debug Overlay
=============

Press ``Ctrl`` and ``~`` (tilde or backtick) within a Bacon application to toggle the debug overlay.  This shows vital statistics about the performance of your game, and can be used to check for memory leaks and performance bottlenecks.  The statistics shown are described below.

FPS
	Instantaneous frames per second (FPS).  Bacon limits the FPS to the monitor vsync and so typically is never higher than 60.
MSPF
	Milliseconds per frame.  Calculated as 1000 / FPS, this is a more useful measure for benchmarking than FPS as it has a linear scale.
Texture Memory
	Total number of bytes allocated to resident textures.
Images
	Total number of image handles.  Note that individual font glyphs are also images and are cached by Bacon.  Image regions also count towards this number.  The overhead per image is low and so a high number of images is not necessarily a point of concern.
Textures
	Total number of textures resident.  Textures are the GPU storage for images, and Bacon automatically packs multiple images into a single texture.
Shaders
	Total number of shaders resident.
Targets/Frame
	Number of times the active target was changed during the previous frame.  This can potentially be an expensive operation.
Draws/Frame
	Number of draw calls submitted to the graphics driver during the previous frame.  Bacon automatically batches multiple primitives (e.g., images) into a smaller number of draw calls, but is forced to flush a
	draw call when the render state changes (e.g., by changing blend modes, shader, or texture).
Primitives/Frame
	Number of primitives rendered during the previous frame.  An image counts as two primitives, as it is composed of two triangles.
Sounds
	Number of sounds currently loaded
Voices
	Number of voices currently active