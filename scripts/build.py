import os
import sys
base_dir = os.path.join(os.path.dirname(__file__), '..')
sys.path.insert(0, base_dir)

import setup
import shutil
import git
import subprocess


# Dropbox details are not checked in, you must create
# them locally according to your Dropbox installation
try:
    from build_auth import dropbox_dir
except ImportError:
    raise ImportError('build_auth.py must be provided with dropbox_dir')

class Dropbox(object):
    def __init__(self):
        pass

    def login(self):
        pss

    def put(self, src_path, share_path):
        shutil.copytree(src_path, os.path.join(dropbox_dir, share_path))
        
    def get(self, share_path, dest_path):
        shutil.copytree(os.path.join(dropbox_dir, share_path), dest_path)
        
dropbox = Dropbox()
    
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

def publish_build_files(version, commit, files):
    share_path = '/bacon-%s/%s/' % (version, commit)
    print('Copying local build files to dropbox...')
    for file in files:
        print(file)
        dropbox.put(os.path.join(base_dir, file), share_path + file)

def download_build_files(version, commit, alt_files):
    print('Copying alternative platform files from dropbox...')
    share_path = '/bacon-%s/%s/' % (version, commit)
    try:
        for file in alt_files:
            dropbox.get(share_path + file, os.path.join(base_dir, file))
    except rest.ErrorResponse:
        print('...not found, finished build')
        return False

    return True

def has_build_files(version, commit, files):
    share_path = '/bacon-%s/%s/' % (version, commit)
    try:
        for file in alt_files:
            dropbox.get(share_path + file, os.path.join(base_dir, file))
    except IOError:
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

def get_build_files():
    windows_files = ['bacon/windows32', 'bacon/windows64']
    darwin_files = ['bacon/darwin32', 'bacon/darwin64']
    if sys.platform == 'win32':
        files = windows_files
        alt_files = darwin_files
    elif sys.platform == 'darwin':
        files = darwin_files
        alt_files = windows_files
    else:
        raise Exception('Unsupported platform %s' % sys.platform)

    return files, alt_files

def get_master_commit():
    repo = git.Repo(base_dir)
    if repo.is_dirty:
        raise Exception('Git repo is dirty')
    return repo.commit('master').id

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

    print('Version %s' % version)
    print('Commit %s' % commit)
    import time; time.sleep(1)

    files, alt_files = get_build_files()
    if not has_build_files(version, commit, files):
        build()
        publish_build_files(version, commit, files)

    if download_build_files(version, commit, alt_files):
        publish()
        tag(version)





