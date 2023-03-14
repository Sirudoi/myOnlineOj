#include <iostream>
#include "../common/httplib.h"

using namespace httplib;

int main()
{
    Server svr;

    // 获取题目列表
    svr.Get("/questions_list", [](const Request& req, Response& resp){
        resp.set_content("这是题目列表", "text/plain;charset=utf-8");
    });

    // 获取单个题目内容
    svr.Get(R"(/question/(\d+))", [](const Request& req, Response& resp){
        std::string question_num = req.matches[1]; // 正则匹配到的内容
        resp.set_content("题目" + question_num, "text/plain;charset=utf-8");
    });

    // 用户提交
    svr.Get(R"(/judge/(\d+))", [](const Request& req, Response& resp){
        std::string question_num = req.matches[1]; // 正则匹配到的内容
        resp.set_content("判题题目" + question_num, "text/plain;charset=utf-8");
    });

    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0", 8080);

    return 0;
}