import distutils
import distutils.sysconfig
from distutils.core import setup
import os

version = '0.2.1'

script_dir = os.path.dirname(__file__)
lib_dir = distutils.sysconfig.get_python_lib()
data_files = []

def add_native_module(module, files):
  module_dir = os.path.join(script_dir, module)
  files = [os.path.join(module_dir, file) for file in files]
  data_files.append((os.path.join(lib_dir, module), files))

add_native_module('bacon/windows32', [
  'Bacon.dll',
  'libEGL.dll',
  'libGLESv2.dll',
  'd3dcompiler_46.dll',
  'msvcp110.dll',
  'msvcr110.dll',
  'vccorlib110.dll'
])
add_native_module('bacon/windows64', [
  'Bacon.dll',
  'libEGL.dll',
  'libGLESv2.dll',
  'd3dcompiler_46.dll',
  'msvcp110.dll',
  'msvcr110.dll',
  'vccorlib110.dll'
])
add_native_module('bacon/darwin32', [
  'Bacon.dylib'
])
add_native_module('bacon/darwin64', [
  'Bacon64.dylib'
])

if __name__ == '__main__':
    setup(name='bacon',
          description='Bacon Game Engine',
          long_description=open(os.path.join(script_dir, 'README'), 'r').read(),
          license='MIT',
          classifiers=[
            'Development Status :: 2 - Pre-Alpha',
            'Environment :: MacOS X',
            'Environment :: Win32 (MS Windows)',
            'Intended Audience :: Developers',
            'License :: OSI Approved :: MIT License',
            'Operating System :: MacOS :: MacOS X',
            'Operating System :: Microsoft :: Windows',
            'Programming Language :: C++',
            'Programming Language :: Python',
            'Topic :: Games/Entertainment',
            'Topic :: Software Development :: Libraries :: Python Modules',
          ],

          version=version,
          author='Alex Holkner',
          author_email='alex.holkner@gmail.com',
          url='https://github.com/aholkner/bacon',
          packages=[
            'bacon', 
            'bacon.windows32',
            'bacon.windows64',
            'bacon.darwin32',
            'bacon.darwin64'],
          data_files=data_files,
    )
