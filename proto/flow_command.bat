protoc -oflowStatus.pb flowStatus.proto
python D:\src\has\arduino\protobuf\nanopb-0.3.9\generator\nanopb_generator.py flowStatus.pb
del C:\Users\kybro\Documents\Arduino\libraries\flowStatus\flowStatus.pb.c
del C:\Users\kybro\Documents\Arduino\libraries\flowStatus\flowStatus.pb.h
del C:\Users\kybro\Documents\Arduino\libraries\flowStatus\flowStatus.pb
del C:\Users\kybro\Documents\Arduino\libraries\flowStatus\flowStatus.proto
copy flowStatus.* C:\Users\kybro\Documents\Arduino\libraries\flowStatus

protoc -I="." --java_out=D:\src\has\server\src\main\java "flowStatus.proto"

protoc -oflowCommand.pb flowCommand.proto
python D:\src\has\arduino\protobuf\nanopb-0.3.9\generator\nanopb_generator.py flowCommand.pb
del C:\Users\kybro\Documents\Arduino\libraries\flowCommand\flowCommand.pb.c
del C:\Users\kybro\Documents\Arduino\libraries\flowCommand\flowCommand.pb.h
del C:\Users\kybro\Documents\Arduino\libraries\flowCommand\flowCommand.pb
del C:\Users\kybro\Documents\Arduino\libraries\flowCommand\flowCommand.proto
copy flowCommand.* C:\Users\kybro\Documents\Arduino\libraries\flowCommand

protoc -I="." --java_out=D:\src\has\server\src\main\java "flowCommand.proto"