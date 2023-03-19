#ifndef INCLUDE_HEAD
#include "head.cpp"
#endif

void TestCase1()
{
    string s = "abaccdeff";
    char c = Solution().firstUniqChar(s);

    if (c == 'b')
    {
        cout << "测试用例通过:1/2..." << endl;
    }
    else
    {
        cout << "测试用例通过:0/2..." << endl;
        cout << "用例输入:abaccdeff" << endl;
        cout << "预期输出:b 你的输出:" << c << endl;
        exit(0);
    }
}

void TestCase2()
{
    string s = "asduiuiddss";
    char c = Solution().firstUniqChar(s);

    if (c == 'a')
    {
        cout << "测试用例通过:2/2..." << endl;
    }
    else
    {
        cout << "测试用例通过:1/2..." << endl;
        cout << "用例输入:asduiuiddss" << endl;
        cout << "预期输出:a 你的输出:" << c << endl;
        exit(0);
    }
}

int main()
{
    TestCase1();
    TestCase2();

    return 0;
}