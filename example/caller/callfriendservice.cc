#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"

int main(int argc, char **argv) {
    // 整个程序启动以后，想使用mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login
    example::FriendServiceRpc_Stub stub(new MprpcChannel());
    // rpc方法的请求参数
    example::GetFriendsListRequest request;
    request.set_id(1);
    // rpc方法的响应参数
    example::GetFriendsListResponse response;
    // 发起rpc方法的调用   同步的rpc调用过程   MprpcChannel::callmethod
    MprpcController controller;
    stub.GetFriendsList(&controller, &request, &response, nullptr); // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送
    
    // 一次rpc调用完成，读调用的结果
    if (controller.Failed()) {
        std::cout << "rpc call failed! error: " << controller.ErrorText() << std::endl;
    } else {
        if (0 == response.result().errcode()) {
            std::cout << "rpc call success!" << std::endl;
            std::cout << "friends list: " << std::endl;
            for (int i = 0; i < response.friends_size(); ++i) {
                std::cout << response.friends(i) << std::endl;
            }
        } else {
            std::cout << "rpc call failed! error: " << response.result().errmsg() << std::endl;
        }
    }
    
    return 0;
}