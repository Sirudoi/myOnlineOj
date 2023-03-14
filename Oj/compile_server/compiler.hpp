#pragma once

// 编译类
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../common/Util.hpp"
#include "../common/log.hpp"

namespace ns_complier
{
    using namespace ns_log;
    using namespace ns_util;

    class Compiler
    {
    public:
        //编译功能，将传入的文件进行编译
        static bool CompileService(const std::string& file_name)
        {
            pid_t ret = fork();
            //子进程
            if (ret == 0)
            {
                std::string src_path = PathUtil::BuildSrcPath(file_name);      // 构建xx.cpp路径
                std::string exe_path = PathUtil::BuildExePath(file_name);      // 构建xx.exe路径
                std::string stderr_path = PathUtil::BuildCompliErrPath(file_name); // 构建xx.stderr路径

                // 1.创建重定向标准错误流文件描述符
                int new_stderr = open(stderr_path.c_str(), O_CREAT | O_WRONLY, 0644);
                if (new_stderr < 0)
                {
                    //打开文件失败
                    LOG(WARN) << "没有成功创建compli_err文件" << std::endl;
                    exit(0);
                }
                else
                {
                    //重定向标准错误流到new_stderr
                    dup2(new_stderr, 2);
                }

                // 2.子进程进行编译工作
                // g++ -o target src -std=c++11
                // 构建target和src，利用进程替换执行编译行为
                execlp("g++", "g++", "-o", exe_path.c_str() , src_path.c_str(), "-std=c++11", nullptr);

                exit(1);
            }

            //父进程
            else if (ret > 0)
            {
                waitpid(ret, nullptr, 0);
                //判断是否编译成功，即查看是否生成可执行文件
                if (FileUtil::IsFileExit(PathUtil::BuildExePath(file_name)))
                {
                    LOG(INFO) << " [" << PathUtil::BuildSrcPath(file_name) << "编译成功]" << std::endl;

                    return true;
                }
                else
                {
                    LOG(ERROR) << " [编译失败]" << std::endl;

                    return false;
                }
            }

            //创建失败
            else
            {
                LOG(WARN) << " [子进程创建失败]" << std::endl;
                exit(2);
            }
        }
    };
}