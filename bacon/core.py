from bacon import native
from ctypes import *
import collections
import logging
import os
import sys

# Bacon logger
logger = logging.getLogger('bacon')
logger.setLevel(logging.INFO)

# Default logging configuration.  Users can override this
# by setting the 'bacon' logger configuration before importing
# bacon.
if not logger.handlers:
    log_file = 'bacon.log'

    # Check for a console or null handler in the parent, if one hasn't been set up, set one
    # up for bacon.  (This makes behaviour on Windows and OS X the same; Windows sets up a
    # default console handler, OS X sets up nothing).
    _has_console_handler = False
    for handler in logger.parent.handlers:
        if isinstance(handler, logging.NullHandler) or isinstance(handler, logging.StreamHandler):
            _has_console_handler = True
    if not _has_console_handler:
        console_handler = logging.StreamHandler()
        console_handler.setLevel(logging.WARNING)
        console_handler.setFormatter(logging.Formatter('%(levelname)s: %(message)s'))
        logger.addHandler(console_handler)

    logger.warning('No logging configuration for "bacon" set; using default')

    try:
        file_handler = logging.FileHandler(log_file)
        file_handler.setLevel(logging.INFO)
        file_handler.setFormatter(logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s'))
        logger.addHandler(file_handler)
        logger.info('Writing log file at %s', os.path.abspath(log_file))
    except IOError:
        logger.warning('Unable to create log file at %s', log_file)


# Native logging callback translates into Python logger calls

_log_level_map = {
    native.LogLevels.trace: logging.DEBUG,
    native.LogLevels.info: logging.INFO,
    native.LogLevels.warning: logging.WARNING,
    native.LogLevels.error: logging.ERROR,
    native.LogLevels.fatal: logging.FATAL,
}

def _log_callback(level, message):
    try:
        level = _log_level_map[level]
    except KeyError:
        level = logging.ERROR
    logger.log(level, message.decode('utf-8'))


# Convert return codes into exceptions.

class BaconError(Exception):
    def __init__(self, error_code):
        self.error_code = error_code

    def __repr__(self):
        return '%s(%d)' % (self.__class__.__name__, self.error_code)

    _error_classes = {}

    @classmethod
    def _register_error_class(cls, error_code, error_class):
        cls._error_classes[error_code] = error_class

    @classmethod
    def _from_error_code(cls, error_code):
        try:
            error_class = cls._error_classes[error_code]
        except KeyError:
            error_class = BaconError
        return error_class(error_code)

def error_code(error_code):
    def wrap(cls):
        BaconError._register_error_class(error_code, cls)
        return cls
    return wrap

@error_code(native.ErrorCodes.unknown)
class UnknownError(BaconError):
    pass

@error_code(native.ErrorCodes.invalid_argument)
class InvalidArgumentError(BaconError):
    pass

@error_code(native.ErrorCodes.invalid_handle)
class InvalidHandleError(BaconError):
    pass

@error_code(native.ErrorCodes.stack_underflow)
class StackUnderflowError(BaconError):
    pass

@error_code(native.ErrorCodes.unsupported_format)
class UnsupportedFormatError(BaconError):
    pass

@error_code(native.ErrorCodes.shader_compile_error)
class ShaderCompileError(BaconError):
    pass

@error_code(native.ErrorCodes.shader_link_error)
class ShaderLinkError(BaconError):
    pass

@error_code(native.ErrorCodes.not_rendering)
class NotRenderingError(BaconError):
    pass

@error_code(native.ErrorCodes.invalid_font_size)
class InvalidFontSizeError(BaconError):
    pass

@error_code(native.ErrorCodes.not_looping)
class NotLoopingError(BaconError):
    pass

@error_code(native.ErrorCodes.running)
class RunningError(BaconError):
    pass

@error_code(native.ErrorCodes.rendering_to_self)
class RenderingToSelfError(BaconError):
    pass

@error_code(native.ErrorCodes.io_error)
def  _io_error(error_code):
    return IOError()

def _error_wrapper(fn):
    def f(*args):
        result = fn(*args)
        if result != native.ErrorCodes.none:
            raise BaconError._from_error_code(result)
    return f


# Initialize library now

lib = native.load(function_wrapper = _error_wrapper)

if not native._mock_native:
    _log_callback_handle = lib.LogCallback(_log_callback)
    lib.SetLogCallback(_log_callback_handle)
    lib.Init()

    from bacon import commands
    commands.init()

    # Expose library version
    major_version = c_int()
    minor_version = c_int()
    patch_version = c_int()
    lib.GetVersion(byref(major_version), byref(minor_version), byref(patch_version))
    major_version = major_version.value     #: Major version number of the Bacon dynamic library that was loaded, as an integer.
    minor_version = minor_version.value     #: Minor version number of the Bacon dynamic library that was loaded, as an integer.
    patch_version = patch_version.value     #: Patch version number of the Bacon dynamic library that was loaded, as an integer.
else:
    major_version, minor_version, patch_version = (0, 1, 0)


#: Version of the Bacon dynamic library that was loaded, in the form ``"major.minor.patch"``.
version = '%d.%d.%d' % (major_version, minor_version, patch_version)

