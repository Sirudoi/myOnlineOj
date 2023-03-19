#pragma once

#include "../common/log.hpp"
#include "../common/Util.hpp"
#include "model.hpp"
#include <ctemplate/template.h>

namespace ns_view
{
    using namespace ns_util;
    using namespace ns_model;
    using namespace ns_log;

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
}
