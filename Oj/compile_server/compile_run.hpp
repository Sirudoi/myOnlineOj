#pragma once

#include "compiler.hpp"
#include "runner.hpp"
#include "../common/log.hpp"
#include "../common/Util.hpp"

#include <jsoncpp/json/json.h>

namespace ne_compile_run
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_runner;
    using namespace ns_complier;

    class CompileAndRun
    {
        // json中的key&value
        // **************************************************
        // 用户方：
        // code：用户提交的代码
        // input：用户提交的输入数据
        // **************************************************
        // 客户端：
        // status:用户程序运行的状态码
        // 
        // stdout：程序输出结果
        // stderr：程序错误输出结果
        // **************************************************
    public:
        static int StartCplAndRun(const std::string& in_str, std::string* out_str)
        {
            // 进行反序列化
            Json::Value in_json;
            Json::Reader reader;
            reader.parse(in_str, in_json); // 将传入的序列化字符串反序列化到in_json中

            // 获得用户代码和用户输入
            std::string code = in_json["code"].asString();
            std::string input = in_json["input"].asString();

            // 获取题目资源限制
            int cpu_limit = in_json["cpu_limit"].asInt();
            int mem_limit = in_json["men_limit"].asInt();

            // 生成唯一的文件名
            std::string file_name = FileUtil::UniqueName();

            // 将用户代码的编译运行情况通过out_json返回
            // out_json[status]:表示返回情况
            // out_json[reason]:返回值描述
            Json::Value out_json;
            
            int start_status = 0; // StartCplAndRun()返回值，和差错处理保持一直
            int run_status; // ExecuteService()返回值，goto里面不给定义变量

            if (code.size() == 0)
            {
                start_status = -3;
                goto ERROR_HANDING_END;
            }

            // 生成src.cpp，将代码写入到里面
            if ( !FileUtil::WriteFile(PathUtil::BuildSrcPath(file_name), code) )
            {
                start_status = -1;
                goto ERROR_HANDING_END;
            }

            // 编译服务
            if ( !Compiler::CompileService(file_name) )
            {
                start_status = -2;
                goto ERROR_HANDING_END;
            }

            // 运行服务
            run_status = Runner::ExecuteService(file_name, cpu_limit, mem_limit);
            if (run_status > 0)
            {
                // 运行报错
                start_status = run_status;
            }
            else if (run_status == 0)
            {
                // 正常执行
                start_status = run_status;
            }
            else
            {
                // 内部报错
                start_status = -1;
            }
            
        ERROR_HANDING_END:
            out_json["status"] = start_status; // 存储返回的status
            out_json["reason"] = FileUtil::DescripeStatus(start_status); // 根据status返回其描述
            // 执行成功，返回三个流
            if (start_status == 0)
            {
                out_json["stdin"] = FileUtil::ReadFile(PathUtil::BuildStdinPath(file_name));
                out_json["stdout"] = FileUtil::ReadFile(PathUtil::BuildStdoutPath(file_name));
                out_json["stderr"] = FileUtil::ReadFile(PathUtil::BuildStderrPath(file_name));
            }

            Json::StyledWriter write;
            *out_str = write.write(out_json);
        }
    };
}