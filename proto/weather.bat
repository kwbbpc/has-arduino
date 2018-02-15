protoc -oweather.pb weather.proto
python D:\src\has\lib\proto\nanopb-0.3.9\generator\nanopb_generator.py weather.pb
del C:\Users\kybro\Documents\Arduino\libraries\weather\weather.pb.c
del C:\Users\kybro\Documents\Arduino\libraries\weather\weather.pb.h
del C:\Users\kybro\Documents\Arduino\libraries\weather\weather.pb
del C:\Users\kybro\Documents\Arduino\libraries\weather\weather.proto
copy weather.* C:\Users\kybro\Documents\Arduino\libraries\weather

protoc -I="." --java_out=D:\src\has\server\src\main\java "weather.proto"