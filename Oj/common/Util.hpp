#pragma once

// 工具类
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

namespace ns_util
{
    // 临时文件路径
    const std::string g_tmp_path = "./tmp/";

    class PathUtil
    {
    public:
        // 构建后缀：.cpp源文件/.exe可执行文件/.compli_err错误输出文件 / ...
        static std::string AddSuffix(const std::string &file_name, const std::string &suffix)
        {
            std::string file_path = g_tmp_path;
            file_path += file_name;
            file_path += suffix;

            return file_path;
        }

        // 构建编译源文件路径，xx.cc
        static std::string BuildSrcPath(const std::string &file_name)
        {
            return AddSuffix(file_name, ".cpp");
        }

        // 构建编译目标文件路径，xx.exe
        static std::string BuildExePath(const std::string &file_name)
        {
            return AddSuffix(file_name, ".exe");
        }

        // 构建编译错误时，错误信息输出的路径，xx.compli_err
        static std::string BuildCompliErrPath(const std::string &file_name)
        {
            return AddSuffix(file_name, ".compli_err");
        }

        // 构建用户提交代码的stdin路径，xx.stdin
        static std::string BuildStdinPath(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdin");
        }

        // 构建用户提交代码的stdout路径，xx.stdout
        static std::string BuildStdoutPath(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdout");
        }

        // 构建用户提交代码的stderr路径，xx.stderr
        static std::string BuildStderrPath(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stderr");
        }
    };

    class FileUtil
    {
    public:
        static bool IsFileExit(const std::string &file_path)
        {
            struct stat s;
            if (stat(file_path.c_str(), &s) == 0)
            {
                return true;
            }

            return false;
        }

        // 生成唯一文件名
        static std::string UniqueName()
        {
            // TODO

            return "";
        }

        // 读取指定文件，返回其内容
        static std::string ReadFile(const std::string &file_path)
        {
            // TODO
        }

        // 将指定内容写入文件
        static int WriteFile(const std::string &file_path, const std::string &code)
        {
            // TODO
        }

        // 根据传入信号返回信号描述
        static std::string DescripeStatus(int status)
        {
            std::string desc;
            // TODO
            switch (status)
            {
            case -1:
                desc = "未知错误";
                break;
            case -2:
                desc = "编译错误";
                break;
            case -3:
                desc = "代码为空";
                break;
            case 0:
                desc = "运行成功";
                break;
            case 6:
                desc = "运行超过限制时间，请检查是否存在死循环";
                break;
            case 24:
                desc = "内存申请超过限制空间";
                break;
            default:
                desc = "未知信号错误,status=";
                desc += std::to_string(status);
                break;
            }

            return desc;
        }
    };

    class TimeUtil
    {
    public:
        static std::string GetTimeStamp()
        {
            struct timeval st;
            gettimeofday(&st, nullptr);

            // 返回1970至今的second
            return std::to_string(st.tv_sec);
        }
    };

    // TODUBUG:
    class DeleteUtil
    {
    public:
        static bool DeleteFile(const std::string &file_path)
        {
            if (FileUtil::IsFileExit(file_path))
            {
                unlink(file_path.c_str());

                return true;
            }
            else
                return false;
        }
    };
}