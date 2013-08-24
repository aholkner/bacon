setlocal
set NativeDir=%1
set TargetPath=%2
set OutDir=%3
set VCInstallDir=%4
set FrameworkSdkDir=%5

mkdir %NativeDir%
copy %TargetPath% %NativeDir%
copy %OutDir%\Bacon.pdb %NativeDir%
copy %OutDir%\libEGL.dll %NativeDir%
copy %OutDir%\libEGL.pdb %NativeDir%
copy %OutDir%\libGLESv2.dll %NativeDir%
copy %OutDir%\libGLESv2.pdb %NativeDir%
copy %VCInstallDir%redist\x86\Microsoft.VC110.CRT\*.dll %NativeDir%
copy %FrameworkSdkDir%Redist\D3D\x86\D3DCompiler_46.dll %NativeDir%