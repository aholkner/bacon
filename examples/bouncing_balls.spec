# -*- mode: python -*-
a = Analysis(['bouncing_balls.py'],
             pathex=['bouncing_balls'],
             hiddenimports=[],
             hookspath=None,
             runtime_hooks=None)
pyz = PYZ(a.pure)
res_tree = Tree('res', prefix='res')
exe = EXE(pyz,
          a.scripts,
          a.binaries,
          a.zipfiles,
          a.datas,
          res_tree,
          name='bouncing_balls.exe',
          debug=False,
          strip=None,
          upx=True,
          console=False)
coll = COLLECT(exe,
               a.binaries,
               a.zipfiles,
               a.datas,
               #res_tree,
               strip=None,
               upx=True,
               name='dist/bouncing_balls')
app = BUNDLE(coll,
             name='bouncing_balls.app',
             icon=None)