#include "logger.h"
#include <time.h>
#include <iostream>

// 获取单例对象
Logger& Logger::GetInstance(){
    static Logger logger;
    return logger;
}

Logger::Logger() {
    // 启动专门的写日志线程
    std::thread writeLogTask([&]() {
        for (;;) {
            // 获取当前的日期，然后取日志信息，写入相应的日志文件当中 a+
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);

            char filename[128];
            sprintf(filename, "%d-%d-%d-log.txt", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);

            FILE *fp = fopen(filename, "a+");
            if (fp == nullptr) {
                printf("open file %s failed!\n", filename);
                exit(EXIT_FAILURE);
            }

            std::string msg = m_lckQue.Pop();
            fputs(msg.c_str(), fp);
            fclose(fp);
        }
    });
    // 设置分离线程，守护线程
    writeLogTask.detach();
}

// 设置日志级别
void Logger::SetLogLevel(LogLevel level) {
    m_logLevel = level;
}

// 写日志，把日志信息写入lockqueue缓冲区当中
void Logger::Log(std::string msg) {
    m_lckQue.Push(msg);
}