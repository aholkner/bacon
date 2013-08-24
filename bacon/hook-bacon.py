from PyInstaller.compat import is_win, is_darwin
from hookutils import collect_data_files

import sys
import ctypes
print 'hello'

if is_win:
    if ctypes.sizeof(ctypes.c_void_p) == 4:
        from . import windows32
        datas = collect_data_files(windows32)
    else:
        from . import windows64
        datas = collect_data_files(windows64)
elif is_darwin:
    if ctypes.sizeof(ctypes.c_void_p) == 4:
        from . import darwin32
        datas = collect_data_files(darwin32)
    else:
        from . import darwin64
        datas = collect_data_files(darwin64)
