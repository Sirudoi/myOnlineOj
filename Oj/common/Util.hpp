#pragma once

// 工具类
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <boost/algorithm/string.hpp>

namespace ns_util
{
    // 临时文件路径
    const std::string g_tmp_path = "./tmp/";

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

        // 获得微秒级时间戳
        static std::string GetTimeMicro()
        {
            struct timeval st;
            gettimeofday(&st, nullptr);

            return std::to_string(st.tv_sec) + "_" + std::to_string(st.tv_usec);;
        }
    };

    class PathUtil
    {
    public:
        // 构建后缀：.cpp源文件/.exe可执行文件/.compli_err错误输出文件 / ...
        static inline std::string AddSuffix(const std::string &file_name, const std::string &suffix)
        {
            std::string file_path = g_tmp_path;
            file_path += file_name;
            file_path += suffix;

            return file_path;
        }

        // 构建编译源文件路径，xx.cc
        static inline std::string BuildSrcPath(const std::string &file_name)
        {
            return AddSuffix(file_name, ".cpp");
        }

        // 构建编译目标文件路径，xx.exe
        static inline std::string BuildExePath(const std::string &file_name)
        {
            return AddSuffix(file_name, ".exe");
        }

        // 构建编译错误时，错误信息输出的路径，xx.compli_err
        static inline std::string BuildCompliErrPath(const std::string &file_name)
        {
            return AddSuffix(file_name, ".compli_err");
        }

        // 构建用户提交代码的stdin路径，xx.stdin
        static inline std::string BuildStdinPath(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdin");
        }

        // 构建用户提交代码的stdout路径，xx.stdout
        static inline std::string BuildStdoutPath(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdout");
        }

        // 构建用户提交代码的stderr路径，xx.stderr
        static inline std::string BuildStderrPath(const std::string &file_name)
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

        // 生成唯一文件名，获取秒+微妙时间戳
        static std::string UniqueName()
        {
            // TODO
            return TimeUtil::GetTimeMicro();
        }

        // 读取指定文件，返回其内容
        static bool ReadFile(const std::string &file_path, std::string& content, bool flag = false)
        {
            std::ifstream ifs(file_path.c_str(), std::ifstream::in);
            content.clear();

            if (!ifs.is_open())
            {
                return false;
            }

            std::string line;
            // getline不保留换行符
            // 换行符需要内部控制，因此传入一个flag，如果外部需要保留换行符，传入true
            while (getline(ifs, line))
            {
                content += line;
                if (flag) content += '\n';
            }

            ifs.close();
            return true;
        }

        // 将指定内容写入文件
        static bool WriteFile(const std::string &file_path, const std::string &code)
        {
            std::ofstream ofs(file_path.c_str(), std::iostream::out);
            if (!ofs.is_open())
            {
                return false;
            }
            ofs.write(code.c_str(), code.size());
            ofs.close();

            return true;
        }

        // 根据传入信号返回信号描述
        static inline std::string DescripeStatus(int status)
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
                desc = "内存申请达到上限";
                break;
            case 11:
                desc = "Segmentation fault";
                break;
            case 24:
                desc = "运行超过限制时间，请检查是否存在死循环";
                break;
            default:
                desc = "未知信号错误,status=";
                desc += std::to_string(status);
                break;
            }

            return desc;
        }

        static inline void DeleteFile(const std::string& file_name)
        {
            std::string src_path = PathUtil::BuildSrcPath(file_name);
            std::string exe_path = PathUtil::BuildExePath(file_name);
            std::string in_path = PathUtil::BuildStdinPath(file_name);
            std::string out_path = PathUtil::BuildStdoutPath(file_name);
            std::string err_path = PathUtil::BuildStderrPath(file_name);
            std::string cmplerr_path = PathUtil::BuildCompliErrPath(file_name);


            if (FileUtil::IsFileExit(src_path)) unlink(src_path.c_str());
            if (FileUtil::IsFileExit(exe_path)) unlink(exe_path.c_str());
            if (FileUtil::IsFileExit(in_path)) unlink(in_path.c_str());
            if (FileUtil::IsFileExit(out_path)) unlink(out_path.c_str());
            if (FileUtil::IsFileExit(err_path)) unlink(err_path.c_str());
            if (FileUtil::IsFileExit(cmplerr_path)) unlink(cmplerr_path.c_str());
        }
    };

    class StringUtil
    {
    public:
        // line：需要切分的字符串 
        // tokens：输出型参数，按照op切分后返回结果
        // op：按照op字符做字符串切分
        static bool CutString(const std::string& line, std::vector<std::string>& tokens, const std::string& op)
        {
            // 切分后返回的数组，需要切分字符串，切分字符，批处理空切分是否打开
            boost::split(tokens, line, boost::is_any_of(op), boost::algorithm::token_compress_on);

            return true;
        }
    };
}