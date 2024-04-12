before build project need to install protobuf с++ compilier v3.15.2 from https://github.com/protocolbuffers/protobuf/releases/tag/v3.15.2 and run these commands 
```
git submodule update --init --recursive; 
git submodule update --remote
cd main; cd proto;
protoc --proto_path=./protobuf/ --cpp_out=./protobuf/ ./protobuf/*/*.proto;
cd ../; cd ../;
```