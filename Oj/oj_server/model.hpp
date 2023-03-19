#pragma once

#include <vector>
#include <fstream>
#include <unordered_map>
#include "../common/log.hpp"
#include "../common/Util.hpp"

namespace ns_model
{
    using namespace ns_util;
    using namespace ns_log;

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
}