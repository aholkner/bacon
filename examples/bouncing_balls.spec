# -*- mode: python -*-
a = Analysis(['../../bacon/examples/bouncing_balls.py'],
             pathex=['/Users/alex/development/foreign/pyinstaller/bouncing_balls'],
             hiddenimports=[],
             hookspath=None,
             runtime_hooks=None)
pyz = PYZ(a.pure)
res_tree = Tree('../../bacon/examples/res', prefix='res')
exe = EXE(pyz,
          a.scripts,
          exclude_binaries=True,
          name='bouncing_balls',
          debug=False,
          strip=None,
          upx=True,
          console=False )
coll = COLLECT(exe,
               a.binaries,
               a.zipfiles,
               a.datas,
               [('Bacon.dylib', '../../bacon/bacon/Bacon.dylib', 'DATA')],
               res_tree,
               strip=None,
               upx=True,
               name='bouncing_balls')
app = BUNDLE(coll,
             name='bouncing_balls.app',
             icon=None)
