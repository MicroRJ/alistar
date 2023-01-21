@REM # For debug builds I can't compile with /Za because windows complains about it...
@REM /MDd
@SET myDebugOptions=/Od /MTd
@SET myCompilerOptions=/options:strict /nologo /TP /Z7 /WX /W4 /Fa /std:c++20
@SET myLinkerOptions=/INCREMENTAL:NO

@SET myInclude=/I. /I.. /I..\detroit /I..\dr /I..\vendor\stb /Ivendor /Ivendor\civetweb\include /Ivendor\civetweb\src
@SET mySourceFiles=main.c
@CALL cl %myCompilerOptions% %myDebugOptions% %myInclude% main.c /link /SUBSYSTEM:WINDOWS %myLinkerOptions%
@CALL cl %myCompilerOptions% %myDebugOptions% %myInclude% simple.c /link /SUBSYSTEM:CONSOLE %myLinkerOptions%
