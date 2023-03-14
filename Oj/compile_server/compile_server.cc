#include "compile_run.hpp"
#include "../common/httplib.h"

using namespace httplib;
using namespace ns_compile_run;

int main()
{
    // DEBUG编译运行:
    {
        // // 构建用户传入
        // Json::Value client;
        // client["input"] = "";
        // client["code"] = R"(
        // #include<iostream>
        // int main()
        // {
        //     while (1)
        //     {
        //     }

        //     return 0;
        // })";
        // client["cpu_limit"] = 1;
        // client["mem_limit"] = 1024 * 50;

        // // 序列化
        // std::string in_str;
        // Json::StyledWriter writer;
        // in_str = writer.write(client);
        // std::cout << in_str;

        // // 编译运行
        // std::string out_str;
        // ns_compile_run::CompileAndRun::StartCplAndRun(in_str, &out_str);
        // std::cout << out_str << std::endl;
    }

    // DEBUGhttplib
    {
        // Server svr;

        // //为某一路径注册路由方法
        // svr.Get("/hello", [](const Request& req, Response& resp){
        //     // 最后一个参数是body类型
        //     resp.set_content("上机！", "text/plain;charset=utf-8");
        // });

        // //启动http
        // svr.listen("0.0.0.0", 8080);
    }

    Server svr;

    svr.Post("/compile_and_run", [](const Request& req, Response& resp){
        std::string in_str = req.body;
        // std::cout << in_str << std::endl;
        std::string out_str;
        if (!in_str.empty())
        {
            CompileAndRun::StartCplAndRun(in_str, &out_str);
            resp.set_content(out_str, "application/json;charset=utf-8");
        }
    });

    svr.listen("0.0.0.0", 8080);
    // {
    //     //FOR DEBUG
    //     std::string in_str = R"({
    //     "code" : "
    //     #include <iostream>
    //     int main()
    //     {
    //         std::cout << \"hello compile_and_run\" << std::endl;
    //         return 0;
    //     }",
    //     "input" : "",
    //     "cpu_limit" : "2",
    //     "mem_limit" : "50000"})";

    //     std::string out_str;
    //     CompileAndRun::StartCplAndRun(in_str, &out_str);
    //     std::cout << out_str;
    // }
    return 0;
}