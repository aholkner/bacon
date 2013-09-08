import os
import sys

from bacon import native

#: Path to resources.  Set to the script directory by default during development, and the executable
#: directory for frozen applications.
resource_dir = os.path.abspath(os.path.dirname(sys.argv[0]))

if native._mock_native:
    resource_dir = ''

# Or use frozen executable path
try:
    if sys.frozen:
        resource_dir = os.path.dirname(sys.executable)
except AttributeError:
    pass

# In PyInstaller --onefile mode, use sys._MEIPASS temporary
# directory to find local files if they are not found in default resource 
# directory
_dll_dir = None
try:
    _dll_dir = sys._MEIPASS
except AttributeError:
    pass

def get_resource_path(filename):
    '''Get a path to the given filename to load as a resource.  All non-absolute filenames passed to
    :class:`Image`, :class:`Font`, :class:`Sound`, etc are transformed through this function.

    :param str filename: a relative path to a resource file
    :return str: an absolute path to the file
    '''
    path = os.path.join(resource_dir, filename)
    if _dll_dir and not os.path.exists(path):
        path = os.path.join(_dll_dir, filename)
    return path