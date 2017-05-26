.\..\..\tools\vswhere.exe -latest -property installationPath > %TEMP%/where_is_vs.txt
set /p VS_2017_DIR= < %TEMP%/where_is_vs.txt

@call "%VS_2017_DIR%\VC\Auxiliary\Build\vcvarsall.bat" amd64 & %*
