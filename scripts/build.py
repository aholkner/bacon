import os
import sys
base_dir = os.path.join(os.path.dirname(__file__), '..')
sys.path.insert(0, base_dir)

import setup
import shutil
import subprocess


# Dropbox details are not checked in, you must create
# them locally according to your Dropbox installation
try:
    from build_auth import dropbox_dir
except ImportError:
    raise ImportError('build_auth.py must be provided with dropbox_dir')

def build_osx():
    subprocess.call([
        'xcodebuild', 
        '-scheme', 'Bacon'
        ], 
        cwd=os.path.join(base_dir, 'native/Projects/Xcode'))
    subprocess.call([
        'xcodebuild', 
        '-scheme', 'Bacon64'
        ], 
        cwd=os.path.join(base_dir, 'native/Projects/Xcode'))

def build_windows():
    subprocess.call([
        'C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\MSBuild.exe',
        'Bacon.sln',
        '/p:Configuration=Release',
        '/p:Platform=Win32'
        ],
        cwd=os.path.join(base_dir, 'native/Projects/VisualStudio'))
    subprocess.call([
        'C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\MSBuild.exe',
        'Bacon.sln',
        '/p:Configuration=Release',
        '/p:Platform=x64'
        ],
        cwd=os.path.join(base_dir, 'native/Projects/VisualStudio'))

def copy_dir_files(src, dest):
    os.makedirs(dest)
    for file in os.listdir(src):
        if not os.path.exists(os.path.join(dest, file)):
            shutil.copy2(os.path.join(src, file), os.path.join(dest, file))

def publish_build_dirs(version, commit, dirs):
    print('Copying local build dirs to dropbox...')
    for dir in dirs:
        copy_dir_files(os.path.join(base_dir, dir), share_path + dir)

def download_build_dirs(version, commit, alt_dirs):
    print('Copying alternative platform dirs from dropbox...')
    for dir in alt_dirs:
        if not os.path.exists(dir):
            print('...not found, finished build')
            return False
        copy_dir_files(share_path + dir, os.path.join(base_dir, dir))
    return True

def has_build_dirs(version, commit, dirs):
    for file in dirs:
        if not os.path.exists(share_path + file):
            return False
    return True

def build():
    if sys.platform == 'win32':
        build_windows()
    elif sys.platform == 'darwin':
        build_osx()
    else:
        raise Exception('Unsupported platform %s' % sys.platform)

def publish():
    print('Publishing to PyPI')
    subprocess.call(['python', 'setup.py', 'sdist', '--formats=zip', 'upload'], cwd=base_dir)

def get_build_dirs():
    windows_dirs = ['bacon/windows32', 'bacon/windows64']
    darwin_dirs = ['bacon/darwin32', 'bacon/darwin64']
    if sys.platform == 'win32':
        dirs = windows_dirs
        alt_dirs = darwin_dirs
    elif sys.platform == 'darwin':
        dirs = darwin_dirs
        alt_dirs = windows_dirs
    else:
        raise Exception('Unsupported platform %s' % sys.platform)

    return dirs, alt_dirs

def get_master_commit():
    changes = subprocess.Popen(['git', 'diff', '--shortstat'], stderr=None, stdout=subprocess.PIPE).communicate()[0]
    if changes:
        raise Exception('Git repo is dirty')
    return subprocess.Popen(['git', 'rev-parse', 'HEAD'], stderr=None, stdout=subprocess.PIPE).communicate()[0].strip()

def get_version():
    return setup.version

def get_native_version():
    import re
    src = open(os.path.join(base_dir, 'native/Source/Bacon/Bacon.h'), 'r').read()
    major = re.match('.*#define BACON_VERSION_MAJOR ([0-9]+).*', src, re.DOTALL).groups(1)[0]
    minor = re.match('.*#define BACON_VERSION_MINOR ([0-9]+).*', src, re.DOTALL).groups(1)[0]
    patch = re.match('.*#define BACON_VERSION_PATCH ([0-9]+).*', src, re.DOTALL).groups(1)[0]
    return '%s.%s.%s' % (major, minor, patch)

def tag(version):
    subprocess.call(['git', 'tag', '-a', 'v%s' % version, 'Release %s' % version], shell=True)

if __name__ == '__main__':
    version = get_version()
    native_version = get_native_version()
    if version != native_version:
        raise Exception('Native version does not match setup.py (%s vs %s)' % (native_version, version))

    commit = get_master_commit()
    share_path = os.path.join(dropbox_dir, 'bacon-%s/%s/' % (version, commit))

    print('Version %s' % version)
    print('Commit %s' % commit)
    import time; time.sleep(1)

    dirs, alt_dirs = get_build_dirs()
    if not has_build_dirs(version, commit, dirs):
        build()
        publish_build_dirs(version, commit, dirs)

    if download_build_dirs(version, commit, alt_dirs):
        publish()
        tag(version)





