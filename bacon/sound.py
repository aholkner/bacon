from ctypes import *

from bacon.core import lib
from bacon import native
from bacon import resource

class Sound(object):
    '''Loads a sound from disk.  Supported formats are WAV (``.wav``) and Ogg Vorbis (``.ogg``).

    Sounds can be played immediately with the :func:`play` method; or used as the parameter to a new
    :class:`Voice`, if control over the playback is required.

    Sounds can be streamed by specifying ``stream=True``; this causes them to load faster but incur a small
    latency on playback.  Background music should always be streamed.  Sound effects should not be streamed.

    Sounds are kept in memory until explicitly unloaded, see :func:`unload`.

    :param file: path to the sound file to load.  The sound format is deduced from the file extension.
    :param stream: if ``True``, the sound is streamed from disk; otherwise it is fully cached in memory.
    '''
    def __init__(self, file, stream=False):
        flags = 0
        if stream:
            flags |= native.SoundFlags.stream

        if file.lower().endswith('.wav'):
            flags |= native.SoundFlags.format_wav
        elif file.lower().endswith('.ogg'):
            flags |= native.SoundFlags.format_ogg

        handle = c_int()
        lib.LoadSound(byref(handle), resource.get_resource_path(file).encode('utf-8'), flags)
        self._handle = handle.value

    def unload(self):
        '''Release all resources associated with the sound.'''
        lib.UnloadSound(self._handle)
        self._handle = -1

    def play(self, gain=None, pan=None, pitch=None):
        '''Play the sound as a `one-shot`.

        The sound will be played to completion.  If the sound is played more than once at a time, it will mix
        with all previous instances of itself.  If you need more control over the playback of sounds, see
        :class:`Voice`.

        :param gain: optional volume level to play the sound back at, between 0.0 and 1.0 (defaults to 1.0)
        :param pan: optional stereo pan, between -1.0 (left) and 1.0 (right)
        :param pitch: optional sampling rate modification, between 0.4 and 16.0, where 1.0 represents the original pitch
        '''
        if gain is None and pan is None and pitch is None:
            lib.PlaySound(self._handle)
        else:
            voice = Voice(self)
            if gain is not None:
                voice.gain = gain
            if pan is not None:
                voice.pan = pan
            if pitch is not None:
                voice.pitch = pitch
            voice.play()

class Voice(object):
    '''Handle to a single instance of a sound.  Voices can be used to:

    - Modify the parameters of a sound while it's playing (for example, its gain, pan, or pitch)
    - Pause and resume playback
    - Determine the current playback head position and seek to a different position
    - Provide a callback for when the sound finishes playing
    - Create and control a sound that loops automatically

    A voice is created to play back a single sound instance once (besides any looping), and cannot be reused to play 
    another sound, or even to play the same sound once playback of that sound completes.

    Do not use a ``Voice`` to play simple sound effects that do not require control during playing; see :func:`Sound.play`.

    Create a voice by first loading the sound, then creating a voice to play it::

        engine_sound = bacon.Sound('res/Engine.wav')
        engine_voice = bacon.Voice(engine_sound)
        engine_voice.play()

    Specify the ``loop`` parameter to play music that loops until stopped::

        music_sound = bacon.Sound('res/MyMusic.ogg', stream=True)
        music_voice = bacon.Voice(music_sound, loop=True)
        music_voice.play()

    Voices become invalid when their sound completes (after their callback returns, if one is set).  Continuing to use
    a voice after it has finished will result in an exception; you can check the state of a voice with :attr:`playing`.

    Paused voices remain in memory until explicitly destroyed with :func:`destroy`.

    :param sound: a :class:`Sound` to play
    :param loop: if ``True``, the voice will be set to looping playback
    '''
    _gain = 1.0
    _pitch = 1.0
    _pan = 0.0
    _callback = None

    def __init__(self, sound, loop=False):
        flags = 0
        if loop:
            flags |= native.VoiceFlags.loop

        handle = c_int()
        lib.CreateVoice(byref(handle), sound._handle, flags)
        self._handle = handle

    def destroy(self):
        '''Destroy a voice.  Required if a sound is stopped before completing to free associated resources.
        '''
        lib.DestroyVoice(self._handle)
        self._handle = -1

    def play(self):
        '''Being or resume playing the sound.'''
        lib.PlayVoice(self._handle)

    def stop(self):
        '''Pause playback of the sound.'''
        lib.StopVoice(self._handle)

    def _get_gain(self):
        return self._gain
    def _set_gain(self, gain):
        self._gain = gain
        lib.SetVoiceGain(self._handle, gain)
    gain = property(_get_gain, _set_gain, doc='''Get or set the gain (volume) of the sound, between 0.0 and 1.0.  Defaults to 1.0''')

    def _get_pitch(self):
        return self._pitch
    def _set_pitch(self, pitch):
        self._pitch = pitch
        lib.SetVoicePitch(self._handle, pitch)
    pitch = property(_get_pitch, _set_pitch, doc='''Get or set the pitch (sample rate) of the sound, between 0.4 and 16.0, with 1.0 being normal playback speed.''')

    def _get_pan(self):
        return self._pan
    def _set_pan(self, pan):
        self._pan = pan
        lib.SetVoicePan(self._handle, pan)
    pan = property(_get_pan, _set_pan, doc='''Get or set the stereo pan of the sound, between -1.0 and 1.0.''')

    def _is_playing(self):
        playing = c_int()
        lib.IsVoicePlaying(self._handle, byref(playing))
        return playing.value
    def _set_playing(self, playing):
        if playing:
            self.play()
        else:
            self.stop()
    playing = property(_is_playing, _set_playing, doc='''``True`` if the sound is currently playing, ``False`` if it has finished.''')

    def set_loop_points(self, start_sample=-1, end_sample=0):
        '''Set the loop points within the sound.

        The sound must have been created with ``loop=True``.  The default parameters cause the loop points to be set to
        the entire sound duration.

        :note: There is currently no API for converting sample numbers to times.
        :param start_sample: sample number to loop back to
        :param end_sample: sample number to loop at
        '''
        lib.SetVoiceLoopPoints(self._handle, start_sample, end_sample)

    def _get_callback(self):
        return self._callback
    def _set_callback(self, callback):
        self._callback = callback
        self._callback_handle = lib.VoiceCallback(callback)
        lib.SetVoiceCallback(self._handle, self._callback_handle)
    callback = property(_get_callback, _set_callback, doc='''Set a callback function to be called when the sound finishes.  The function takes no arguments.''')

    def _get_position(self):
        position = c_int()
        lib.GetVoicePosition(self._handle, position)
        return position.value
    def _set_position(self, position):
        lib.SetVoicePosition(self._handle, position)
    position = property(_get_position, _set_position, doc='''Get or set the current sample position within the sound.  

        :note: There is currently no API for converting sample numbers to times.
        ''')