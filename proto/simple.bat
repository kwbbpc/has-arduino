protoc -osimple.pb simple.proto
python D:\src\has\lib\proto\nanopb-0.3.9\generator\nanopb_generator.py simple.pb

del C:\Users\kybro\Documents\Arduino\libraries\simple\simple.pb.c
del C:\Users\kybro\Documents\Arduino\libraries\simple\simple.pb.h
del C:\Users\kybro\Documents\Arduino\libraries\simple\simple.pb
del C:\Users\kybro\Documents\Arduino\libraries\simple\simple.proto
copy simple.* C:\Users\kybro\Documents\Arduino\libraries\simple

protoc -I="." --java_out=D:\src\has\server\src\main\java "simple.proto"
