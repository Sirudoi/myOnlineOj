## 项目名称

负载均衡式的在线OJ平台

### 项目演示

![image-20230320204600907](https://gitee.com/sirudoi/my-item-oj/raw/master/images/image-20230320204600907.png)

很简单的首页，基本功能我都没有实现(哭)

![image-20230320204629759](https://gitee.com/sirudoi/my-item-oj/raw/master/images/image-20230320204629759.png)

这里是题库，题库我暂时只录了几道题

![image-20230320204724394](https://gitee.com/sirudoi/my-item-oj/raw/master/images/image-20230320204724394.png)	

在线编译，会向用户返回编译结果

![image-20230320204834330](https://gitee.com/sirudoi/my-item-oj/raw/master/images/image-20230320204834330.png)

负载功能

![image-20230320205028157](https://gitee.com/sirudoi/my-item-oj/raw/master/images/image-20230320205028157.png)

### 项目宏观原理

![image-20230320205621003](https://gitee.com/sirudoi/my-item-oj/raw/master/images/image-20230320205621003.png)

### 编译运行模块

#### compiler模块

![image-20230320215929880](https://gitee.com/sirudoi/my-item-oj/raw/master/images/image-20230320215929880.png)	

- `oj_server`交付给编译模块时，是`json`的序列化内容，因此要读取`json`里面的代码模块，并且生成一个`.cpp`后缀的临时文件，进行编译
- 同样的，需要在相同路径下生成一个可执行文件
- 编译错误，则需要返回标准错误流的结果，因此需要将标准错误流的文件重定向到一个新文件中，方便将错误结果返回给用户
- `compiler`只负责编译，运行交给`rnner`模块

```c++
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
            // 构建xx.cpp路径
            std::string src_path = PathUtil::BuildSrcPath(file_name);
            // 构建xx.exe路径
            std::string exe_path = PathUtil::BuildExePath(file_name);
            // 构建xx.stderr路径
            std::string stderr_path = PathUtil::BuildCompliErrPath(file_name); 
            
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
            execlp("g++", "g++", "-o", exe_path.c_str() , src_path.c_str(), "-std=c++11", "-D", "INCLUDE_HEAD", nullptr);

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
```

#### runner模块

- 编译模块生成可执行文件后，将这个可执行文件交给runner模块去执行
- runner只负责运行代码，不关心执行结果
- runner模块同样需要重定向`stdout`，`stdin`，`stderr`
- 这里的`stderr`是用户代码运行报错的标准错误流，之前`compiler`模块的`stderr`是用户代码编译报错时输出的内容
- 为了防止用户输入恶意代码，因此需要限制用户提交程序的CUP和内存资源

```c++
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
```

#### 编译运行模块

- 这个模块需要获取`oj_server`传入的序列化网络数据反序列化，并且拿到里面的用户代码
- 这里内部需要做差错处理
- 用户代码编译错误，则不执行程序，返回的是编译错误信息
- 用户代码编译通过，在父进程接受子进程（用户代码）的信号，根据信号进行差错处理
- 如果用户程序返回的信号 = 0，则说明执行成功
- 如果用户程序返回的信号 > 0，则说明用户程序有运行时报错，需要给用户返回报错信息，因此需要根据信号的值，来分别返回不同的情况

```c++
static int StartCplAndRun(const std::string &in_str, std::string &out_str)
{
    // 进行反序列化
    Json::Value in_json;
    Json::Reader reader;
    reader.parse(in_str, in_json); // 将传入的序列化字符串反序列化到in_json中

    // 获得用户代码和用户输入
    std::string code = in_json["code"].asString();
    std::string input = in_json["input"].asString();

    // 获取题目资源限制
    int cpu_limit = std::stoi(in_json["cpu_limit"].asString());
    int mem_limit = std::stoi(in_json["mem_limit"].asString());
    LOG(INFO) << " [获取用户输入成功]" << std::endl;

    // 生成唯一的文件名
    std::string file_name = FileUtil::UniqueName();

    // 将用户代码的编译运行情况通过out_json返回
    // out_json[status]:表示返回情况
    // out_json[reason]:返回值描述
    Json::Value out_json;

    int start_status = 0; // StartCplAndRun()返回值，和差错处理保持一直
    int run_status = 0;   // ExecuteService()返回值，goto里面不给定义变量

    if (code.size() == 0)
    {
        start_status = -3;
        goto ERROR_HANDING_END;
    }

    // 生成src.cpp，将代码写入到里面
    if (!FileUtil::WriteFile(PathUtil::BuildSrcPath(file_name), code))
    {
        start_status = -1;
        goto ERROR_HANDING_END;
    }

    // 编译服务
    if (!Compiler::CompileService(file_name))
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
    out_json["status"] = start_status;                           // 存储返回的status
    out_json["reason"] = FileUtil::DescripeStatus(start_status); // 根据status返回其描述
    // 执行成功，返回两个流，stdin不用管，程序输入是上面input获取的
    if (start_status == 0)
    {
        std::string out;
        std::string err;
        FileUtil::ReadFile(PathUtil::BuildStdoutPath(file_name), out, true);
        FileUtil::ReadFile(PathUtil::BuildStderrPath(file_name), err, true);

        out_json["stdout"] = out;
        out_json["stderr"] = err;
    }
    else if (start_status == -2)
    {
        std::string cmpl_err;
        FileUtil::ReadFile(PathUtil::BuildCompliErrPath(file_name), cmpl_err, true);

        out_json["compile_err"] = cmpl_err;
    }

    Json::StyledWriter write;
    out_str = write.write(out_json);

    // 删除临时文件
    FileUtil::DeleteFile(file_name);

    return 0;
}
```

根据信号返回错误信息，写成case，方便后续添加更多的错误内容

```c++
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
```

### 基于`MVC`的`oj_server`

#### 用户请求的路由功能

- 需要有获取题目列表的功能
- 需要有获取单个题目，并且编写的功能
- 判断题目是否正确的功能
- 这里有一个`RecoverMachine`是用来发送信号恢复服务的，后续介绍

```c++
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
```

#### model模块

- model模块负责和数据交互，题库的修改
- 首先要加载所有的题目列表，包括题号，标题，难度还有资源限制等等
- 通过读取一个初始化文件来获取所有题目列表
- 根据题目列表，读取每道题的题面，给用户预设的代码，还有测试用例代码，构建题号和详细内容的索引
- 获得所有题目和一个题目直接调用相关接口

```c++
const std::string g_base_path = "./questions/questions.base";
const std::string g_ques_path = "./questions/";

struct Question
{
    std::string number;     // 题目编号
    std::string title;      // 题目标题
    std::string difficulty; // 题目难度
    std::string desc;       // 题目描述
    int cpu_limit;          // CPU限制，单位s
    int mem_limit;          // 内存限制，单位kb

    std::string head; // 用户预设代码，给用户coding
    std::string tail; // 测试用例代码，和head拼接
};

class Model
{
private:
    // 题号和题目的映射
    std::unordered_map<std::string, Question> _ques_map;

public:
    Model()
    {
        assert(LoadQuestionConfig());
    }
    ~Model() {}

    // 加载文件配置，获取题目编号
    bool LoadQuestionConfig()
    {
        std::ifstream ifs(g_base_path);

        if (!ifs.is_open())
        {
            LOG(WARN) << " [加载questions.base失败,请检查相关文件]" << std::endl;
        }

        std::string line;
        while (getline(ifs, line))
        {
            Question q;

            // 读取questions.base中的题目信息
            std::vector<std::string> tokens;
            StringUtil::CutString(line, tokens, " "); // 按照空格切分

            if (tokens.size() != 5)
            {
                LOG(ERROR) << " [" << line << " 格式错误，读取失败]" << std::endl;
                continue;
            }

            // 获取题目基本信息
            // 顺序为：编号，题目，难度，cpu限制，内存限制
            q.number = tokens[0];
            q.title = tokens[1];
            q.difficulty = tokens[2];
            q.cpu_limit = std::stoi(tokens[3]);
            q.mem_limit = std::stoi(tokens[4]);

            // 读取用户预设代码，测试代码，题目描述
            std::string head_path = g_ques_path + q.number + "/head.cpp";
            std::string tail_path = g_ques_path + q.number + "/tail.cpp";
            std::string desc_path = g_ques_path + q.number + "/description.txt";

            FileUtil::ReadFile(head_path, q.head, true);
            FileUtil::ReadFile(tail_path, q.tail, true);
            FileUtil::ReadFile(desc_path, q.desc, true);

            // 插入到map中
            _ques_map.insert(std::make_pair(q.number, q));
        }

        LOG(INFO) << " [加载题库完成]" << std::endl;
        ifs.close();

        return true;
    }

    // 获取所有题目
    bool GetAllQuestions(std::vector<Question> &question_list)
    {
        if (_ques_map.size() == 0)
        {
            LOG(ERROR) << " [获取所有题目失败,题目列表为空]" << std::endl;
            return false;
        }

        for (const auto &it : _ques_map)
        {
            question_list.emplace_back(it.second);
        }
        return true;
    }

    // 获取单个题目
    bool GetOneQuestion(const std::string &number, Question &question)
    {
        if (_ques_map.size() == 0)
        {
            LOG(ERROR) << " [获取" << number << "题失败,题目列表为空]" << std::endl;
            return false;
        }

        // 根据题号获取题目
        auto iter = _ques_map.find(number);
        if (iter != _ques_map.end())
        {
            question = iter->second;
            return true;
        }
        else
        {
            LOG(ERROR) << " [获取" << number << "题失败,请检查题目列表]" << std::endl;
            return false;
        }
    }
};
```

#### view模块

- 引入`ctemplate`库对前端做渲染
- 把题目，题面，题号等等内容动态渲染到前端中
- 渲染介绍如下

![image-20230320224154783](https://gitee.com/sirudoi/my-item-oj/raw/master/images/image-20230320224154783.png)	

```c++
const std::string template_path = "./template/";

class View
{
public:
    View() {}
    ~View() {}

public:
    void ExpandALLHtml(const std::vector<Question>& ques_list, std::string* html)
    {
        // 形成数据字典
        ctemplate::TemplateDictionary root("all");
        for (const auto& q : ques_list)
        {
            ctemplate::TemplateDictionary* sub = root.AddSectionDictionary("question_list");
            sub->SetValue("number", q.number);
            sub->SetValue("title", q.title);
            sub->SetValue("difficulty", q.difficulty);
        }

        // 获取被渲染的html
        std::string all_path = template_path + "all.html";
        ctemplate::Template *tpl = ctemplate::Template::GetTemplate(all_path, ctemplate::DO_NOT_STRIP);

        // 完成渲染
        tpl->Expand(html, &root);
    }

    void ExpandOneHtml(const Question& q, std::string* html)
    {
        // 形成数据字典
        ctemplate::TemplateDictionary root("one");
        root.SetValue("number", q.number);
        root.SetValue("title", q.title);
        root.SetValue("difficulty", q.difficulty);
        root.SetValue("desc", q.desc);
        root.SetValue("head", q.head);

        // 获取被渲染的html
        std::string one_path = template_path + "one.html";
        ctemplate::Template *tpl = ctemplate::Template::GetTemplate(one_path, ctemplate::DO_NOT_STRIP);

        // 完成渲染
        tpl->Expand(html, &root);
    }
};
```

#### control模块

- 控制器需要反序列化用户json后，拼接用户代码和测试用例代码，将拼接好的代码交给编译服务执行
- 控制器需要有返回题目列表，返回单题目列表，执行测试用例的功能
- 判题功能涉及到负载均衡式选择一台机器，让其执行编译运行任务
- 控制器同样需要控制考虑compiler服务机器挂掉，需要马上选择下一台

```c++
// 控制器
class Control
{
private:
    Model _model;
    View _view;
    Balance _balance;

public:
    Control() {}
    ~Control() {}

public:
    // 获取所有题目，返回所有题目的网页
    bool AllQuestions(std::string *html)
    {
        std::vector<Question> ques_list;
        if (_model.GetAllQuestions(ques_list))
        {
            // 按题号排序
            std::sort(ques_list.begin(), ques_list.end(), [](const Question &q1, const Question &q2)
                      { return std::stoi(q1.number) < std::stoi(q2.number); });

            // 获取全部题目，构建题目列表网页
            _view.ExpandALLHtml(ques_list, html);

            return true;
        }
        else
        {
            return false;
        }
    }

    // 根据题号，返回单个题目的网页
    bool NumberQuestion(const std::string &number, std::string *html)
    {
        Question q;
        if (_model.GetOneQuestion(number, q))
        {
            // 获取单个题目，构建单个题目网页
            _view.ExpandOneHtml(q, html);

            return true;
        }
        else
        {
            return false;
        }
    }

    // 判题
    void JudgeAnswer(const std::string &number, const std::string &in_str, std::string &out_str)
    {
        // 1.反序列化用户json
        Json::Reader reader;
        Json::Value in_value;
        reader.parse(in_str, in_value);

        // 2.获取题目细节
        Question q;
        _model.GetOneQuestion(number, q);

        // 3.拼接用户完整代码，构建传输给compiler机器的json
        Json::Value cpl_value;
        cpl_value["input"] = in_value["input"];
        cpl_value["code"] = in_value["code"].asString() + "\n" + q.tail;
        cpl_value["cpu_limit"] = q.cpu_limit;
        cpl_value["mem_limit"] = q.mem_limit;

        Json::FastWriter writer;
        std::string in_json = writer.write(cpl_value);

        // 4.根据负载情况选择机器
        while (true)
        {
            int id{};
            Machine *ma;

            if (!_balance.IntelligentSelect(id, &ma))
            {
                LOG(WARN) << " [暂无主机可用,请检查服务器情况]" << std::endl;
                break;
            }

            // 5.向编译模块发起http请求，获得结果
            httplib::Client client(ma->_ip, ma->_port); // 构建客户端
            ma->IncreaseLoad();                         // 要发起请求，添加负载
            LOG(INFO) << " [选择主机成功，"
                      << "主机id:" << id << " 详情:" << ma->_ip << ":" << ma->_port
                      << " 当前主机负载:" << ma->_load << "]" << std::endl;
            _balance.PrintMachine();

            // 第一个参数请求：服务
            // 第二个参数：body
            // 第三个参数：请求类型
            auto res = client.Post("/compile_and_run", in_json, "application/json;charset=utf-8");
            if (res)
            {
                if (res->status == 200) // 响应码为200，即请求成功
                {
                    // 把响应的内容拿给out_str
                    out_str = res->body;
                    ma->DecreaseLoad(); // 编译完成，减少负载
                    break;
                }
                // 相应不正确，减少负载
                ma->DecreaseLoad();
            }
            else
            {
                // 请求失败
                LOG(ERROR) << " [请求主机失败，"
                           << "主机id:" << id << " 主机IP:" << ma->_ip << "可能已离线]" << std::endl;
                // 请求失败可能机器有问题，直接离线该机器
                _balance.OfflineMachine(id);
            }
        }
    }

    void RecoverMachine()
    {
        _balance.OnlineMachine();
    }
};
```

- 其中balance提供了对服务机器的管理方法
- 如果某台机器挂了，balance需要把这台机器下线处理，防止下次又请求到这台机器
- balance同样要提供负载均衡的功能，能让control选择负载最低的机器，把编译任务交给这台机器
- 这里采用的是轮询式的负载均衡算法，轮询遍历所有机器，选择一个负载最低的机器，把编译任务交给他
- 同样的，balance需要保证所有机器都下线了，能提供重新上限的服务

```c++
// 主机
class Machine
{
public:
    std::string _ip;  // ip
    int _port;        // 端口
    uint64_t _load;   // 负载情况
    std::mutex *_mtx; //
public:
    Machine() : _ip(""), _port(0), _load(0), _mtx(nullptr)
    {
    }
    ~Machine() {}

    void IncreaseLoad()
    {
        // std::unique_lock<std::mutex> lk;
        _mtx->lock();
        _load++;
        _mtx->unlock();
    }

    void DecreaseLoad()
    {
        // std::unique_lock<std::mutex> lk;
        _mtx->lock();
        _load--;
        _mtx->unlock();
    }
};


// 配置文件路径
const std::string g_conf_path = "./config/machine.conf";
// 负载均衡
class Balance
{
private:
    std::vector<Machine> _machine; // 所有主机
    std::vector<int> _online;      // 在线主机id
    std::vector<int> _offline;     // 离线主机id
    std::mutex _mtx;               // 保证选择主机时的可靠
public:
    Balance()
    {
        assert(LoadMachine(g_conf_path));
        LOG(INFO) << " [加载主机成功]" << std::endl;
    }
    ~Balance() {}

    // 加载主机
    bool LoadMachine(const std::string &conf_path)
    {
        std::ifstream ifs(conf_path);
        if (!ifs.is_open())
        {
            LOG(WARN) << " [主机文件加载失败，请检查配置]" << std::endl;
            return false;
        }

        std::string line;
        while (getline(ifs, line))
        {
            std::vector<std::string> tokens;
            StringUtil::CutString(line, tokens, " ");
            if (tokens.size() != 2)
            {
                LOG(ERROR) << " [切分 " << line << " 出现错误]" << std::endl;
                continue;
            }

            Machine m;
            m._ip = tokens[0];
            m._port = std::stoi(tokens[1]);
            m._load = 0;
            m._mtx = new std::mutex();

            int id = _machine.size();
            _online.emplace_back(id);
            _machine.emplace_back(m);
        }

        ifs.close();
        return true;
    }

    // 负载均衡选择
    // 返回主机ID和主机
    // 这里要拿到machine[id]主机的地址，不能用值拷贝，值拷贝无法获取负载情况
    bool IntelligentSelect(int &id, Machine **mc)
    {
        _mtx.lock();
        int online_sz = _online.size();
        if (online_sz == 0)
        {
            // 这里要解锁！！！！！！！！
            _mtx.unlock();
            return false;
        }

        // 负载均衡算法
        // 默认选择在线主机中第一个主机
        *mc = &(_machine[_online[0]]);
        id = _online[0];
        // 轮循遍历，每次找到负载最小的主机
        uint64_t min_load = _machine[0]._load;
        for (int i = 0; i < _online.size(); i++)
        {
            uint64_t cur_load = _machine[_online[i]]._load;
            if (min_load > cur_load)
            {
                min_load = cur_load;
                *mc = &(_machine[_online[i]]);
                id = _online[i];
                LOG(DEBUG) << " [选择新主机:" << id << "]" << std::endl;
            }
        }

        _mtx.unlock();

        return true;
    }

    // 上线主机
    void OnlineMachine()
    {
        LOG(INFO) << " [恢复所有主机开始]" << std::endl;
        // _mtx.lock();
        _online.insert(_online.end(), _offline.begin(), _offline.end());
        _offline.erase(_offline.begin(), _offline.end());

        // _mtx.unlock();
        LOG(INFO) << " [恢复所有主机完成]" << std::endl;
    }

    // 下线主机
    void OfflineMachine(const int &id)
    {
        // 防止离线时，出现另一个客户端选择主机的情况
        _mtx.lock();

        for (auto it = _online.begin(); it != _online.end(); ++it)
        {
            if (*it == id)
            {
                _machine[id]._load = 0;
                _online.erase(it);
                _offline.emplace_back(id);
                break; // 这里可能会迭代器失效
            }
        }

        _mtx.unlock();
    }

    // DEBUG
    void PrintMachine()
    {
        LOG(DEBUG) << " [当前在线主机个数:" << _online.size() << " "
                   << "当前离线主机个数:" << _offline.size() << "]" << std::endl;
    }
};
```

### 文件版题库的设计

- 需要有一个`head.cpp`，这是给用户预设的代码
- 还需要有一个`tail.cpp`，这是测试用例的代码
- 当用户提交之后，把`tail.cpp`和`head.cpp`内容拼接，此时就能构成一个完整代码，然后交给编译服务运行

```c++
//head.cpp
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

using namespace std;

class Solution {
public:
    int majorityElement(vector<int>& nums) {
        //write your code
    }
};
```

```c++
//tail.cpp
#ifndef INCLUDE_HEAD
#include "head.cpp"
#endif

void TestCase1()
{
    vector<int> nums = {1, 2, 3, 2, 2, 2, 5, 4, 2};
    int ret = Solution().majorityElement(nums);
    if (ret == 2)
    {
        cout << "通过用例通过:1/2..." << endl;  
    }
    else
    {
        cout << "测试用例通过:0/2..." << endl;
        cout << "用例输入:[1, 2, 3, 2, 2, 2, 5, 4, 2]" << endl;
        cout << "预期输出:5 你的输出:" << ret << endl;
        exit(0);
    }
}

void TestCase2()
{
    vector<int> nums = {1, 6, 6, 7, 9, 6, 6};
    int ret = Solution().majorityElement(nums);
    if (ret == 6)
    {
        cout << "通过用例通过:2/2..." << endl;  
    }
    else
    {
        cout << "用例输入:[1, 6, 6, 7, 9, 6, 6]" << endl;
        cout << "预期输出:6 你的输出:" << ret << endl;
        exit(0);
    }
}

int main()
{
    TestCase1();
    TestCase2();

    return 0;
}
```

- `tail.cpp`有一个宏，因为在拼接的时候，不需要`#include"head.cpp"`
- 但是如果设计题库的时候不引入，会有语法报错，因此定义一个宏来引入
- 编译的时候在外部定义这个宏，就能把`#include"head.cpp"`去掉