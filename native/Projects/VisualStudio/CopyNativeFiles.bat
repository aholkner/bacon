setlocal
set NativeDir=%1
set TargetPath=%2
set OutDir=%3
set VCInstallDirRedist=%4
set FrameworkSdkDirRedist=%5

mkdir %NativeDir%
copy %TargetPath% %NativeDir%
copy %OutDir%\Bacon.pdb %NativeDir%
copy %OutDir%\libEGL.dll %NativeDir%
copy %OutDir%\libEGL.pdb %NativeDir%
copy %OutDir%\libGLESv2.dll %NativeDir%
copy %OutDir%\libGLESv2.pdb %NativeDir%
copy %VCInstallDirRedist%\*.dll %NativeDir%
copy %FrameworkSdkDirRedist%\D3DCompiler_46.dll %NativeDir%