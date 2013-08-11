.. module:: bacon

Audio
-----

Sounds
======

Sounds can be loaded from WAV or Ogg Vorbis files with the :class:`Sound` class.  Once loaded,
you can play a sound as "one-shot" effect by calling :func:`Sound.play`:

.. literalinclude:: ../examples/sound.py
    :emphasize-lines: 3,11

.. autoclass:: Sound
    :members:

Music
=====

Playing looping music, or longer-lasting sound effects like engine sounds, is best done with 
a :class:`Voice`.  A ``Voice`` is a particular instance of a sound playing.  For example,
if you play the same ``ball.wav`` sound effect three times, you have used three voices, whether
or not the sounds are playing all at once or one after another.

The following example demonstrates a looping music track.  The track's pan varies based on
the position of the mouse in the window, and it can be paused and resumed with the spacebar:

.. literalinclude:: ../examples/music.py
    :emphasize-lines: 5,8,9,16,21


.. autoclass:: Voice
    :members:

