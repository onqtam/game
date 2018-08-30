@REM .\..\..\tools\vswhere.exe -latest -property installationPath > %TEMP%/where_is_vs.txt
@REM set /p VS_2017_DIR= < %TEMP%/where_is_vs.txt
@REM @call "%VS_2017_DIR%\VC\Auxiliary\Build\vcvarsall.bat" amd64 & %*

@REM these were extracted by running "vcvarsall.bat x64" (x64/amd64 mean the same) and then inspecting the output from SET

@SET PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.15.26726\bin\HostX64\x64;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\VC\VCPackages;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\TestWindow;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\bin\Roslyn;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Team Tools\Performance Tools;C:\Program Files (x86)\Microsoft Visual Studio\Shared\Common\VSPerfCollectionTools\;C:\Program Files (x86)\Microsoft SDKs\Windows\v10.0A\bin\NETFX 4.6.1 Tools\;C:\Program Files (x86)\Windows Kits\10\bin\x64;C:\Program Files (x86)\Windows Kits\10\bin\10.0.16299.0\x64;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\\MSBuild\15.0\bin;C:\Windows\Microsoft.NET\Framework64\v4.0.30319;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\;%PATH%

@SET LIB=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.15.26726\ATLMFC\lib\x64;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.15.26726\lib\x64;C:\Program Files (x86)\Windows Kits\NETFXSDK\4.6.1\lib\um\x64;C:\Program Files (x86)\Windows Kits\10\lib\10.0.16299.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\lib\10.0.16299.0\um\x64;

@SET LIBPATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.15.26726\ATLMFC\lib\x64;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.15.26726\lib\x64;C:\Program Files (x86)\Windows Kits\10\UnionMetadata\10.0.16299.0\;C:\Program Files (x86)\Windows Kits\10\References\10.0.16299.0\;C:\Windows\Microsoft.NET\Framework64\v4.0.30319;

@SET INCLUDE=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.15.26726\ATLMFC\include;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.15.26726\include;C:\Program Files (x86)\Windows Kits\NETFXSDK\4.6.1\include\um;C:\Program Files (x86)\Windows Kits\10\include\10.0.16299.0\ucrt;C:\Program Files (x86)\Windows Kits\10\include\10.0.16299.0\shared;C:\Program Files (x86)\Windows Kits\10\include\10.0.16299.0\um;C:\Program Files (x86)\Windows Kits\10\include\10.0.16299.0\winrt;

@%*
