from ctypes import *

import bacon
from bacon.core import lib
from bacon import native

Keys = native.Keys

#: Set of keyboard keys that are currently pressed.  Each element is a enumerator of :class:`Keys`.
keys = set()

def _key_event_handler(key, value):
    if value:
        keys.add(key)
    elif key in keys:
        keys.remove(key)

    bacon._current_game.on_key(key, value)
