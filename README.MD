# MpRpc - 轻量级RPC框架

## 项目介绍

MpRpc 是一个基于 Protobuf 和 Muduo 网络库实现的轻量级 RPC(远程过程调用)框架，支持远程服务注册、发现和调用。该框架使用 ZooKeeper 作为服务发现中心，实现了服务端和客户端之间的解耦。

## 技术栈

- **C++11**：使用现代 C++ 特性
- **Protobuf**：用于数据序列化和接口定义
- **Muduo**：高性能网络库
- **ZooKeeper**：分布式服务发现与注册中心
- **CMake**：项目构建系统

## 功能特点

- 基于 Protobuf 的 IDL 接口定义
- 支持多种 RPC 调用方式（同步调用）
- 基于 ZooKeeper 的服务注册与发现
- 异步日志系统
- 支持分布式部署
- 高性能网络通信

## 系统架构

该框架主要包含以下几个核心组件：

1. **RpcProvider**：RPC 服务发布者，负责注册和暴露 RPC 服务
2. **MprpcChannel**：RPC 通道，负责客户端的远程调用
3. **ZooKeeperUtil**：服务发现模块，基于 ZooKeeper 实现
4. **Logger**：日志系统，支持异步写入
5. **MprpcController**：RPC 控制器，处理 RPC 调用过程中的错误信息

## 使用示例

### 1. 定义服务接口 (Protobuf)

```protobuf
syntax = "proto3";
package example;

message ResultCode {
    int32 errcode = 1;
    string errmsg = 2;
}

message GetFriendsListRequest {
    uint32 id = 1;
}

message GetFriendsListResponse {
    ResultCode result = 1;
    repeated string friends = 2;
}

service FriendServiceRpc {
    rpc GetFriendsList(GetFriendsListRequest) returns(GetFriendsListResponse);
}
```

### 2. 实现服务端 (RPC 服务提供者)

```cpp
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

class FriendService : public example::FriendServiceRpc {
public:
    std::vector<std::string> GetFriendsList(uint32_t id) {
        // 实际业务逻辑
        return {"zhangsan", "lisi", "wangwu"};
    }

    void GetFriendsList(::google::protobuf::RpcController* controller,
                      const example::GetFriendsListRequest* request,
                      example::GetFriendsListResponse* response,
                      ::google::protobuf::Closure* done) override {
        uint32_t id = request->id();
        std::vector<std::string> friends = GetFriendsList(id);
        
        example::ResultCode* code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        for (const auto& friend_name : friends) {
            response->add_friends(friend_name);
        }
        
        done->Run();
    }
};

int main(int argc, char **argv) {
    // 初始化框架
    MprpcApplication::Init(argc, argv);
    
    // 注册服务
    RpcProvider provider;
    provider.NotifyService(new FriendService());
    
    // 启动服务
    provider.Run();
    return 0;
}
```

### 3. 实现客户端 (RPC 服务调用者)

```cpp
#include "mprpcapplication.h"
#include "friend.pb.h"

int main(int argc, char **argv) {
    // 初始化框架
    MprpcApplication::Init(argc, argv);
    
    // 创建服务代理
    example::FriendServiceRpc_Stub stub(new MprpcChannel());
    
    // 构造请求参数
    example::GetFriendsListRequest request;
    request.set_id(1);
    
    // 定义响应对象和控制器
    example::GetFriendsListResponse response;
    MprpcController controller;
    
    // 调用远程方法
    stub.GetFriendsList(&controller, &request, &response, nullptr);
    
    // 处理响应
    if (controller.Failed()) {
        std::cout << "RPC调用失败：" << controller.ErrorText() << std::endl;
    } else {
        // 处理成功响应
        std::cout << "获得好友列表：" << std::endl;
        for (int i = 0; i < response.friends_size(); ++i) {
            std::cout << response.friends(i) << std::endl;
        }
    }
    
    return 0;
}
```

### 4. 配置文件示例

```
# RPC服务端配置
rpcserverip=127.0.0.1
rpcserverport=8000
zookeeperip=127.0.0.1
zookeeperport=2181
```

## 项目构建与运行

### 环境依赖

- Linux系统
- CMake 3.0+
- Protobuf 3.0+
- ZooKeeper 3.4+
- Muduo网络库

### 构建步骤

```bash
# 克隆项目
git clone https://github.com/yourusername/MpRpc.git
cd MpRpc

# 构建项目
chmod +x autobuild.sh
./autobuild.sh

# 运行示例服务提供者
cd bin
./provider -i test.conf

# 在另一个终端运行示例服务消费者
cd bin
./consumer -i test.conf
```

## 项目结构

```
MpRpc/
├── bin/                # 可执行文件目录
├── build/              # 构建目录
├── example/            # 示例代码
│   ├── callee/         # 服务提供者示例
│   └── caller/         # 服务调用者示例
├── lib/                # 库文件
│   └── include/        # 头文件
├── src/                # 源代码
│   ├── include/        # 头文件
│   └── ...             # 实现文件
├── autobuild.sh        # 自动构建脚本
├── CMakeLists.txt      # CMake构建文件
└── README.MD           # 本文件
```

## 设计考量

1. **跨平台性**：目前主要支持Linux平台
2. **可扩展性**：框架设计支持功能扩展
3. **性能**：基于高性能的Muduo网络库和Protobuf序列化
4. **易用性**：简单的API设计，容易集成到现有项目中

## 未来计划

- 添加负载均衡功能
- 支持异步调用
- 完善错误处理机制
- 增加超时控制
- 增加连接池

## 贡献

欢迎提交问题和功能需求！

## 许可证

MIT