#ifndef INCLUDE_HEAD
#include "head.cpp"
#endif

void TestCase1()
{
    vector<int> nums = {2, 3, 1, 0, 2, 5, 3};
    int ret = Solution().findRepeatNumber(nums);
    if (ret == 2 || ret == 3)
    {
        cout << "通过用例通过:1/2..." << endl;
    }
    else
    {
        cout << "测试用例通过:0/2..." << endl;
        cout << "用例输入:[1, 2, 3, 2, 2, 2, 5, 4, 2]" << endl;
        cout << "预期输出:2或3 你的输出:" << ret << endl;
        exit(0);
    }
}

void TestCase2()
{
    vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 8, 8, 9};
    int ret = Solution().findRepeatNumber(nums);
    if (ret == 9)
    {
        cout << "通过用例通过:2/2..." << endl;
    }
    else
    {
        cout << "用例输入:[1, 2, 3, 4, 5, 6, 7, 8, 8, 8, 9]" << endl;
        cout << "预期输出:8 你的输出:" << ret << endl;
        exit(0);
    }
}

int main()
{
    TestCase1();
    TestCase2();

    return 0;
}