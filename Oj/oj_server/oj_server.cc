#include <iostream>
#include <signal.h>

#include "control.hpp"
#include "../common/httplib.h"

using namespace httplib;
using namespace ns_control;

static Control* g_ctl_ptr = nullptr;

void RecoverMachine(int signo)
{
    g_ctl_ptr->RecoverMachine();
}

int main()
{
    signal(SIGQUIT, RecoverMachine);

    Server svr;
    Control ctl;
    g_ctl_ptr = &ctl;

    // 获取题目列表
    svr.Get("/questions_list", [&ctl](const Request& req, Response& resp){
        std::string html;
        ctl.AllQuestions(&html);

        resp.set_content(html, "text/html;charset=utf-8");
        // resp.set_content("这是题目列表", "text/plain;charset=utf-8"); // debug
    });

    // 获取单个题目内容
    svr.Get(R"(/question/(\d+))", [&ctl](const Request& req, Response& resp){

        std::string q_num = req.matches[1]; // 正则匹配到的内容
        std::string html;
        ctl.NumberQuestion(q_num, &html);

        resp.set_content(html, "text/html;charset=utf-8");
        // resp.set_content("题目" + question_num, "text/plain;charset=utf-8"); // debug
    });

    // 用户提交
    svr.Post(R"(/judge/(\d+))", [&ctl](const Request& req, Response& resp){
        std::string question_num = req.matches[1]; // 正则匹配到的内容

        std::string out_str;
        ctl.JudgeAnswer(question_num, req.body, out_str);
        resp.set_content(out_str, "application/json;charset=utf-8");
    });

    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0", 8080);

    return 0;
}