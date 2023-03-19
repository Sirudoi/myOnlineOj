#pragma once

#include <mutex>
#include <cassert>
#include <jsoncpp/json/json.h>

#include "model.hpp"
#include "view.hpp"
#include "../common/log.hpp"
#include "../common/Util.hpp"
#include "../common/httplib.h"

namespace ns_control
{
    using namespace ns_util;
    using namespace ns_log;
    using namespace ns_model;
    using namespace ns_view;

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
}