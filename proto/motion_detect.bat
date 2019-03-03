protoc -omotion_detect.pb motion_detect.proto
python D:\src\has\lib\proto\nanopb-0.3.9\generator\nanopb_generator.py motion_detect.pb
del C:\Users\kybro\Documents\Arduino\libraries\motion_detect\motion_detect.pb.c
del C:\Users\kybro\Documents\Arduino\libraries\motion_detect\motion_detect.pb.h
del C:\Users\kybro\Documents\Arduino\libraries\motion_detect\motion_detect.pb
del C:\Users\kybro\Documents\Arduino\libraries\motion_detect\motion_detect.proto
copy motion_detect.* C:\Users\kybro\Documents\Arduino\libraries\motion_detect

protoc -I="." --java_out=D:\src\has\server\src\main\java "motion_detect.proto"