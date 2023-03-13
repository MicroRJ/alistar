@REM  Copyright(C) 2022-2023 Dayan Rodriguez, All rights reserved.
@CALl taskkill /F /IM main.exe
@SET myDebugOptions=/Od /MTd /D_DEBUG /D_DEVELOPER
@SET myCompilerOptions=/options:strict /nologo /TC /Z7 /WX /W4 /Fa
@SET myLinkerOptions=/INCREMENTAL:NO
@SET myInclude=/I. /Ivendor /Ivendor\civetweb\include /Ivendor\civetweb\src
@CALL cl %myCompilerOptions% %myDebugOptions% %myInclude% main.c /link /SUBSYSTEM:WINDOWS %myLinkerOptions%
