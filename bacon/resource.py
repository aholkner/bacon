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

def get_resource_path(filename):
    '''Get a path to the given filename to load as a resource.  All non-absolute filenames passed to
    :class:`Image`, :class:`Font`, :class:`Sound`, etc are transformed through this function.

    :param str filename: a relative path to a resource file
    :return str: an absolute path to the file
    '''
    return os.path.join(resource_dir, filename)