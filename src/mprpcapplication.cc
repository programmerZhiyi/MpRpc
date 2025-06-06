#include "mprpcapplication.h"

MprpcConfig MprpcApplication::m_config; // 全局配置对象
std::mutex MprpcApplication::m_mutex; // 线程安全锁
MprpcApplication *MprpcApplication::m_application = nullptr; // 全局唯一单例访问对象

void ShowArgsHelp() {
    std::cout << "format: command -i <configfile>" << std::endl;
}

void MprpcApplication::Init(int argc, char **argv) {
    if (argc < 2) {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    while ((c = getopt(argc, argv, "i:")) != -1) {
        switch (c) {
            case 'i':
                config_file = optarg;
                break;
            case '?':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            case ':':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    // 开始加载配置文件了 rpcserver_ip rpcserver_port   zookeeper_ip zookeeper_port
    m_config.LoadConfigFile(config_file.c_str());

    // std::cout << "rpcserverip:" << m_config.Load("rpcserverip") << std::endl;
    // std::cout << "rpcserverport:" << m_config.Load("rpcserverport") << std::endl;
    // std::cout << "zookeeperip:" << m_config.Load("zookeeperip") << std::endl;
    // std::cout << "zookeeperport:" << m_config.Load("zookeeperport") << std::endl;
}

MprpcApplication& MprpcApplication::GetInstance() {
    std::lock_guard<std::mutex> lock(m_mutex); // 加锁，保证线程安全
    if (m_application == nullptr) {
        m_application = new MprpcApplication(); // 创建单例对象
        atexit(deleteInstance); // 注册退出函数
    }
    return *m_application;
}

// 程序退出时自动调用的函数，用于销毁单例对象
void MprpcApplication::deleteInstance() {
    if (m_application) {
        delete m_application;
    }
}

MprpcConfig& MprpcApplication::GetConfig() {
    return m_config;
}