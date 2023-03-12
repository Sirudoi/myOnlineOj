#pragma once 

//用于日志打印
#include <iostream>
#include <string>
#include "../common/Util.hpp"

namespace ns_log
{
    // 日志等级
    enum 
    {
        WARN, // 需要马上解决的错误
        ERROR, // 有告警，不影响程序执行
        INFO, // 日志信息
        DEBUG // 用于debug
    };

    // 将日志信息用标准输出流返回
    std::ostream& Log(const std::string& level, const std::string& file_name, int line)
    {
        std::string log_msg;

        //添加日志等级
        log_msg += level;
        log_msg += " ";

        //添加报错文件
        log_msg += "[";
        log_msg += file_name;
        log_msg += "] ";

        //添加报错行
        log_msg += "[line:";
        log_msg += std::to_string(line);
        log_msg += "] ";

        //添加时间戳
        log_msg += "[timestamp:";
        log_msg += ns_util::TimeUtil::GetTimeStamp();
        log_msg += "]";

        //将日志信息重定向到stdout中，不添加\n，保证其在缓冲区不会刷新出来
        std::cout << log_msg;

        return std::cout;
    }

    // 开放式日志利用宏替换
    // __FILE__：宏定义，当前的源文件
    // __LINE__：宏定义，当前行号
    #define LOG(level) Log(#level, __FILE__, __LINE__)
}