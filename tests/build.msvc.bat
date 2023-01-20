@REM # For debug builds I can't compile with /Za because windows complains about it...
@REM /MDd
@SET myDebugOptions=/Od /MTd -D_DEBUG -D_ITERATOR_DEBUG_LEVEL=2
@SET myCompilerOptions=/options:strict /nologo /TP /Z7 /WX /W4 /Fa /std:c++20
@SET myLinkerOptions=/INCREMENTAL:NO
@SET myInclude=/I. /I.. /I..\.. /I..\..\detroit /I..\..\dr /I..\..\vendor\stb /Ivendor /Ivendor\civetweb\include /Ivendor\civetweb\src /Ivendor\s2clientprotocol\include\s2clientprotocol /Ivendor\s2clientprotocol\include /Ivendor\protobuf\src /Ivendor\protobuf\third_party\abseil-cpp

@REM TODO(RJ):
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\sc2api.pb.cc
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\sc2api.pb.h
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\common.pb.cc
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\common.pb.h
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\data.pb.cc
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\data.pb.h
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\debug.pb.cc
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\debug.pb.h
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\error.pb.cc
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\error.pb.h
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\query.pb.cc
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\query.pb.h
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\raw.pb.cc
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\raw.pb.h
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\score.pb.cc
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\score.pb.h
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\spatial.pb.cc
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\spatial.pb.h
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\ui.pb.cc
@DEL  vendor\s2clientprotocol\include\s2clientprotocol\ui.pb.h


@CALL protoc vendor\s2clientprotocol\sc2api.proto --proto_path vendor --cpp_out vendor\s2clientprotocol\include
@CALL protoc vendor\s2clientprotocol\common.proto --proto_path vendor --cpp_out vendor\s2clientprotocol\include
@CALL protoc vendor\s2clientprotocol\data.proto --proto_path vendor --cpp_out vendor\s2clientprotocol\include
@CALL protoc vendor\s2clientprotocol\debug.proto --proto_path vendor --cpp_out vendor\s2clientprotocol\include
@CALL protoc vendor\s2clientprotocol\error.proto --proto_path vendor --cpp_out vendor\s2clientprotocol\include
@CALL protoc vendor\s2clientprotocol\query.proto --proto_path vendor --cpp_out vendor\s2clientprotocol\include
@CALL protoc vendor\s2clientprotocol\raw.proto --proto_path vendor --cpp_out vendor\s2clientprotocol\include
@CALL protoc vendor\s2clientprotocol\score.proto --proto_path vendor --cpp_out vendor\s2clientprotocol\include
@CALL protoc vendor\s2clientprotocol\spatial.proto --proto_path vendor --cpp_out vendor\s2clientprotocol\include
@CALL protoc vendor\s2clientprotocol\ui.proto --proto_path vendor --cpp_out vendor\s2clientprotocol\include

@SET mySourceFiles=sc2.c vendor\s2clientprotocol\include\s2clientprotocol\sc2api.pb.cc vendor\s2clientprotocol\include\s2clientprotocol\common.pb.cc vendor\s2clientprotocol\include\s2clientprotocol\data.pb.cc vendor\s2clientprotocol\include\s2clientprotocol\debug.pb.cc vendor\s2clientprotocol\include\s2clientprotocol\error.pb.cc vendor\s2clientprotocol\include\s2clientprotocol\query.pb.cc vendor\s2clientprotocol\include\s2clientprotocol\raw.pb.cc vendor\s2clientprotocol\include\s2clientprotocol\score.pb.cc vendor\s2clientprotocol\include\s2clientprotocol\spatial.pb.cc vendor\s2clientprotocol\include\s2clientprotocol\ui.pb.cc
@CALL cl %myCompilerOptions% -DDRL_256 %myDebugOptions% %myInclude% /w /EHsc %mySourceFiles% /link /SUBSYSTEM:WINDOWS %myLinkerOptions%
