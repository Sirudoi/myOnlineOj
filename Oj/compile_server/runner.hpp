#pragma once

// 运行类
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>

#include "../common/log.hpp"

namespace ns_runner
{
    using namespace ns_log;
    using namespace ns_util;

    class Runner
    {
    public:
        //设置进程占用资源
        static void SetProcessResource(int cpu_resource, int mem_resource)
        {
            // 限制CPU占用时间
            struct rlimit cpu_limit;
            cpu_limit.rlim_cur = cpu_resource; //软资源，外部传入
            cpu_limit.rlim_max = RLIM_INFINITY; //硬资源，设置成无限
            setrlimit(RLIMIT_CPU, &cpu_limit);

            // 限制可开辟内存大小
            struct rlimit men_limit;
            //mem_resource单位是字节，内部转化一下为kb
            cpu_limit.rlim_cur = mem_resource * 1024; //转化一下单位
            cpu_limit.rlim_max = RLIM_INFINITY; //硬资源，设置成无限
            setrlimit(RLIMIT_AS, &cpu_limit);
        }

        // *********************************************
        // 1. runner 只负责把代码运行，不关系结果
        // 2. runner 需要限制用户代码的资源
        // 3. cpu_limit 限制占用CPU时间，单位s
        // 4. mem_limit 限制可开辟内存大小，单位mb
        // *********************************************
        // 返回值是对应的信号
        // 返回值 > 0 子进程退出异常，收到了信号
        // 返回值 == 0 子进程正常执行
        // 返回值 < 0 父进程内部出错
        // *********************************************
        static int ExecuteService(const std::string& file_name, int cpu_limit, int men_limit)
        {
            std::string exe_path = PathUtil::BuildExePath(file_name);
            std::string stdin_path = PathUtil::BuildStdinPath(file_name);
            std::string stdout_path = PathUtil::BuildStdoutPath(file_name);
            std::string stderr_path = PathUtil::BuildStderrPath(file_name);

            // {
            //     // FOR TEST
            //     DeleteUtil::DeleteFile(stdin_path);
            //     DeleteUtil::DeleteFile(stdout_path);
            //     DeleteUtil::DeleteFile(stderr_path);
            // }

            // 在父进程创建文件，方便错误处理
            int _stdin = open(stdin_path.c_str(), O_CREAT | O_RDONLY, 0644);
            int _stdout = open(stdout_path.c_str(), O_CREAT | O_WRONLY, 0644);
            int _stderr = open(stderr_path.c_str(), O_CREAT | O_WRONLY, 0644);

            if (!(_stdin || _stdout || _stderr))
            {
                LOG(ERROR) << " [用户程序标准流文件创建失败]" << std::endl;
                exit(0);
            }

            pid_t ret = fork();
            //子进程
            if (ret == 0)
            {
                //重定向
                dup2(_stdin, 0);
                dup2(_stdout, 1);
                dup2(_stderr, 2);

                //设置资源
                //cpu：单位秒，内存：单位kb
                SetProcessResource(cpu_limit, men_limit);

                //执行用户程序
                execl(exe_path.c_str(), exe_path.c_str(), nullptr);

                return 0;
            }
            //父进程
            else if (ret > 0)
            {
                close(_stdin);
                close(_stdout);
                close(_stderr);

                int status = 0;
                waitpid(ret, &status, 0);
                LOG(INFO) << " [子进程运行完毕 status:" << status  << "]" << std::endl;

                LOG(INFO) << " [清理临时文件]" << std::endl;
                //判断信号内容
                return status & 0x7f;
            }
            else
            {
                LOG(WARN) << " [子进程创建失败]" << std::endl;
                return -1;
            }
        }
    };
}
