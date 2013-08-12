from distutils.core import setup

version = '0.1.5'

windows_dlls = [
    'bacon/Bacon.dll',
    'bacon/libEGL.dll',
    'bacon/libGLESv2.dll',
    'bacon/d3dcompiler_46.dll'
]

osx_dlls = [
    'bacon/Bacon.dylib'
]


setup(name='bacon',
      description='Bacon Game Engine',
      long_description=open('README', 'r').read(),
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
      packages=['bacon'],
      data_files=[
        ('bacon', windows_dlls + osx_dlls),
      ],
)
