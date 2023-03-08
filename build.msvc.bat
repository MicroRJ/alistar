@REM  Copyright(C) 2022-2023 Dayan Rodriguez, All rights reserved.
@CALl taskkill /F /IM main.exe
@SET myDebugOptions=/Od /MTd
@SET myCompilerOptions=/options:strict /nologo /TC /Z7 /WX /W4 /Fa
@SET myLinkerOptions=/INCREMENTAL:NO

@SET myInclude=/I. /I.. /I..\zen /I..\dr /I..\vendor\stb /Ivendor /Ivendor\civetweb\include /Ivendor\civetweb\src
@CALL cl %myCompilerOptions% %myDebugOptions% %myInclude% main.c /link /SUBSYSTEM:WINDOWS %myLinkerOptions%
@REM @CALL cl %myCompilerOptions% %myDebugOptions% %myInclude% simple.c /link /SUBSYSTEM:CONSOLE %myLinkerOptions%
