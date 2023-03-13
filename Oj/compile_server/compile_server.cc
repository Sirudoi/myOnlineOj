#include "compile_run.hpp"

int main()
{
    {
        // DEBUG：
        // 构建用户传入
        Json::Value client;
        client["input"] = "";
        client["code"] = R"(
        #include<iostream>
        int main()
        {
            while (1)
            {
            }

            return 0;
        })";
        client["cpu_limit"] = 1;
        client["mem_limit"] = 1024 * 50;

        // 序列化
        std::string in_str;
        Json::StyledWriter writer;
        in_str = writer.write(client);
        std::cout << in_str;

        // 编译运行
        std::string out_str;
        ns_compile_run::CompileAndRun::StartCplAndRun(in_str, &out_str);
        std::cout << out_str << std::endl;
    }


    return 0;
}