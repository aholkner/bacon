CopyNativeFiles.bat %*

setlocal
set NativeDir=%1
set TargetPath=%2
set OutDir=%3
set VCInstallDir=%4
set FrameworkSdkDir=%5

copy %OutDir%\Bacon.pdb %NativeDir%
copy %OutDir%\libEGL.pdb %NativeDir%
copy %OutDir%\libGLESv2.pdb %NativeDir%
