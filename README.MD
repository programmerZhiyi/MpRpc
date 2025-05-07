# MpRpc - 高性能分布式RPC框架

## 项目介绍

MpRpc 是一个基于 Protobuf 和 Muduo 网络库实现的轻量级 RPC(远程过程调用)框架，支持远程服务注册、发现和调用。该框架使用 ZooKeeper 作为服务发现中心，实现了服务端和客户端之间的解耦，并提供了高并发、高可用的RPC通信能力。

## 技术栈

- **C++11**：使用现代 C++ 特性，如智能指针、线程库和lambda表达式等
- **Protobuf**：用于数据序列化和接口定义，支持跨语言通信
- **Muduo**：高性能网络库，基于Reactor模式的事件驱动设计
- **ZooKeeper**：分布式服务发现与注册中心，提供高可用服务
- **glog**：Google开源的日志库，提供异步日志记录功能
- **CMake**：跨平台项目构建系统

## 框架核心特性

- **IDL接口定义**：基于 Protobuf 的接口定义语言，实现服务接口与实现分离
- **同步调用**：支持RPC同步调用方式
- **服务注册与发现**：基于 ZooKeeper 实现的服务注册与发现机制
- **网络通信**：基于 Muduo 的高性能TCP长连接通信
- **日志系统**：集成 glog 的高性能异步日志系统
- **分布式部署**：支持分布式环境下的服务部署
- **线程安全**：在多线程环境下保证数据安全

## 系统架构

该框架主要包含以下几个核心组件：

### 服务端组件

1. **RpcProvider**：
   - 提供服务注册接口 `NotifyService()`
   - 基于 Muduo 提供高性能网络服务
   - 将服务注册到 ZooKeeper 节点
   - 处理 RPC 调用请求并分发到对应服务方法

### 客户端组件

2. **MprpcChannel**：
   - 实现 `google::protobuf::RpcChannel` 接口
   - 负责RPC请求的序列化和网络传输
   - 从 ZooKeeper 发现服务地址
   - 处理请求响应和错误情况

3. **MprpcController**：
   - 控制 RPC 调用过程
   - 提供错误信息处理

### 公共组件

4. **ZookeeperUtil**：
   - 封装 ZooKeeper C API
   - 提供服务注册和服务发现功能

5. **MprpcApplication**：
   - 框架初始化
   - 配置管理
   - 单例模式实现

6. **MprpcLogger**：
   - 基于 glog 的日志系统
   - 提供不同级别的日志记录接口

## 通信协议

MpRpc使用自定义的二进制协议进行通信：

```
+---------------+----------------+-------------+-------------+
| 头部长度(4字节) | 头部Protobuf序列化| 参数长度(4字节)| 参数序列化数据 |
+---------------+----------------+-------------+-------------+
```

- 头部包含：服务名、方法名、参数大小等元数据
- 参数数据是Protobuf序列化的二进制流

## 使用示例

### 1. 定义服务接口 (Protobuf)

```protobuf
syntax = "proto3";
package example;

message ResultCode {
    int32 errcode = 1;
    string errmsg = 2;
}

message LoginRequest {
    string name = 1;
    string pwd = 2;
}

message LoginResponse {
    ResultCode result = 1;
    bool success = 2;
}

service UserServiceRpc {
    rpc Login(LoginRequest) returns(LoginResponse);
}
```

### 2. 实现服务端 (RPC 服务提供者)

```cpp
class UserService : public example::UserServiceRpc {
public:
    bool Login(std::string name, std::string pwd) {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name: " << name << std::endl;
        std::cout << "pwd: " << pwd << std::endl;
        return true;
    }

    void Login(::google::protobuf::RpcController* controller,
               const example::LoginRequest* request,
               example::LoginResponse* response,
               ::google::protobuf::Closure* done) {
        // 获取请求参数
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 调用本地业务方法
        bool login_result = Login(name, pwd);

        // 设置响应
        example::ResultCode* code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        // 执行回调，由框架负责序列化和网络发送
        done->Run();
    }
};

int main(int argc, char **argv) {
    // 初始化框架
    MprpcApplication::Init(argc, argv);
    
    // 注册服务
    RpcProvider provider;
    provider.NotifyService(new UserService());
    
    // 启动服务
    provider.Run();
    return 0;
}
```

### 3. 实现客户端 (RPC 服务调用者)

```cpp
int main(int argc, char **argv) {
    // 初始化框架
    MprpcApplication::Init(argc, argv);
    
    // 创建服务代理
    example::UserServiceRpc_Stub stub(new MprpcChannel());
    
    // 构造请求参数
    example::LoginRequest request;
    request.set_name("zhangsan");
    request.set_pwd("123456");
    
    // 定义响应对象和控制器
    example::LoginResponse response;
    MprpcController controller;
    
    // 调用远程方法
    stub.Login(&controller, &request, &response, nullptr);
    
    // 处理响应
    if (controller.Failed()) {
        std::cout << "RPC调用失败：" << controller.ErrorText() << std::endl;
    } else {
        if (0 == response.result().errcode()) {
            std::cout << "登录成功：" << response.success() << std::endl;
        } else {
            std::cout << "登录失败：" << response.result().errmsg() << std::endl;
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

## 高性能设计

MpRpc框架采用了多项技术来确保高性能：

1. **基于Muduo的IO多路复用**：使用epoll实现高效的事件处理
2. **线程池**：使用Muduo内置的线程池处理并发请求
3. **零拷贝优化**：直接使用Protobuf序列化/反序列化数据到Buffer
4. **连接复用**：TCP连接复用减少握手开销
5. **服务发现缓存**：客户端缓存服务地址信息减少查询开销

## 项目构建与运行

### 环境依赖

- Linux系统 (Ubuntu 18.04+/CentOS 7+)
- CMake 3.0+
- Protobuf 3.0+
- ZooKeeper 3.4+
- Muduo网络库
- glog日志库

### 安装依赖

```bash
# Ubuntu/Debian
sudo apt-get install -y g++ cmake libprotobuf-dev protobuf-compiler zookeeper libzookeeper-mt-dev libgoogle-glog-dev

# 安装muduo库(需要单独编译)
git clone https://github.com/chenshuo/muduo.git
cd muduo && cmake . && make install
```

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

## 主要实现细节

### 1. 服务注册过程

```cpp
// RpcProvider::NotifyService 负责服务注册
void RpcProvider::NotifyService(google::protobuf::Service *service) {
    // 获取服务描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务名称
    std::string service_name = pserviceDesc->name();
    // 获取服务方法数量
    int methodCnt = pserviceDesc->method_count();
    
    // 记录服务信息
    ServiceInfo service_info;
    service_info.m_service = service;
    
    // 记录服务方法
    for (int i = 0; i < methodCnt; ++i) {
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
    }
    
    // 注册服务
    m_serviceMap.insert({service_name, service_info});
}
```

### 2. ZooKeeper服务发现

```cpp
// 从ZooKeeper获取服务地址
std::string method_path = "/" + service_name + "/" + method_name;
std::string host_data = zkCli.GetData(method_path.c_str());

// 解析服务地址
int idx = host_data.find(":");
std::string ip = host_data.substr(0, idx);
uint16_t port = atoi(host_data.substr(idx + 1, host_data.size()).c_str());
```

### 3. RPC调用流程

1. 客户端调用Stub方法
2. CallMethod序列化请求
3. 连接到RPC服务端
4. 发送序列化数据
5. 等待并接收响应
6. 反序列化响应数据
7. 返回结果给调用者

## 设计考量

1. **跨平台性**：主要支持Linux平台，可扩展到其他POSIX系统
2. **扩展性**：服务、方法、协议均可扩展
3. **性能优化**：基于高性能的Muduo网络库和Protobuf序列化
4. **易用性**：简洁API设计，与Protobuf无缝集成
5. **线程安全**：关键操作都进行了线程安全保护

## 未来计划

- **负载均衡**：增加多种负载均衡策略(轮询、一致性哈希)
- **异步调用**：支持异步RPC调用模式
- **超时控制**：完善超时和重试机制
- **熔断降级**：添加熔断和服务降级功能
- **连接池**：实现客户端连接池，提高性能
- **多语言客户端**：支持Java、Python等语言客户端
- **监控指标**：添加性能监控和度量指标

## 贡献

欢迎提交问题和功能需求！您可以通过以下方式参与项目：

1. Fork项目并提交Pull Request
2. 提交Issue报告Bug或提出新功能
3. 改进文档和示例

## 许可证

MIT