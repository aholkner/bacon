from dropbox import client, rest
import git
import sys
import os
import subprocess

base_dir = os.path.join(os.path.dirname(__file__), '..')

# Dropbox auth details are not checked in, you must create
# them locally according to your Dropbox developer account.
try:
    from build_auth import app_key, app_secret
except ImportError:
    raise ImportError('build_auth.py must be provided with Dropbox app_key and app_secret strings')

class Dropbox(object):
    TOKEN_FILE = os.path.join(os.path.dirname(__file__), 'build_auth.token.txt')

    def __init__(self):
        self.api_client = None
        try:
            token = open(self.TOKEN_FILE).read()
            self.api_client = client.DropboxClient(token)
            print("[loaded access token]")
        except:
            self.login()

    def login(self):
        flow = client.DropboxOAuth2FlowNoRedirect(app_key, app_secret)
        authorize_url = flow.start()
        sys.stdout.write("1. Go to: " + authorize_url + "\n")
        sys.stdout.write("2. Click \"Allow\" (you might have to log in first).\n")
        sys.stdout.write("3. Copy the authorization code.\n")
        code = raw_input("Enter the authorization code here: ").strip()

        try:
            access_token, user_id = flow.finish(code)
        except rest.ErrorResponse:
            raise

        with open(self.TOKEN_FILE, 'w') as f:
            f.write(access_token)
        self.api_client = client.DropboxClient(access_token)

    def put(self, src_path, share_path):
        f = open(src_path, 'rb')
        self.api_client.put_file(share_path, f)
        f.close()

    def get(self, share_path, dest_path):
        f, metadata = self.api_client.get_file_and_metadata(share_path)
        to_file = open(dest_path, 'wb')
        to_file.write(f.read())
        to_file.close()

def build_osx():
    subprocess.call([
        'xcodebuild', 
        '-scheme', 'Bacon'
        ], 
        cwd=os.path.join(base_dir, 'native/Projects/Xcode'))

def build_windows():
    subprocess.call([
        'C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\MSBuild.exe',
        'Bacon.sln',
        '/p:Configuration=Debug',
        '/p:Platform=Win32'
        ],
        cwd=os.path.join(base_dir, 'native/Projects/VisualStudio'))

def share_build_files(commit, files, alt_files):
    package_dir = os.path.join(base_dir, 'bacon')
    print('Connecting to dropbox')
    dropbox = Dropbox()
    print('Copying local build files to dropbox...')
    for file in files:
        print(file)
        dropbox.put(package_dir, '/%s/%s' % (commit, file))

    print('Copying alternative platform files from dropbox...')
    try:
        for file in alt_files:
            dropbox.get('/%s/%s' (commit, file), package_dir)
    except rest.ErrorResponse:
        print('...not found, finished build')
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

def get_build_files():
    windows_files = [
            'Bacon.dll', 
            'libGLESv2.dll',
            'libEGL.dll',
            'd3dcompiler_46.dll'
        ]

    osx_files = ['Bacon.dylib']

    if sys.platform == 'windows':
        files = windows_files
        alt_files = osx_files
    elif sys.platform == 'darwin':
        files = osx_files
        alt_files = windows_files
    else:
        raise Exception('Unsupported platform %s' % sys.platform)

    return files, alt_filse

def get_master_commit():
    repo = git.Repo(base_dir)
    if repo.is_dirty:
        raise Exception('Git repo is dirty')
    return repo.commit('master').id

if __name__ == '__main__':
    commit = get_master_commit()

    print('Building %s', commit)
    build()

    files, alt_files = get_build_files()
    if share_build_files(commit, files, alt_files):
        publish()





