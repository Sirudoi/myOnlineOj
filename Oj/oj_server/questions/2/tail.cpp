#ifndef INCLUDE_HEAD
#include "head.cpp"
#endif

void TestCase1()
{
    vector<int> nums = {1, 2, 3, 2, 2, 2, 5, 4, 2};
    int ret = Solution().MaxNumber(nums);
    if (ret == 5)
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
    vector<int> nums = {1,3,5,7,9,6,6};
    int ret = Solution().MaxNumber(nums);
    if (ret == 9)
    {
        cout << "通过用例通过:2/2..." << endl;  
    }
    else
    {
        cout << "用例输入:[1, 3, 5, 7, 9, 6, 6]" << endl;
        cout << "预期输出:9 你的输出:" << ret << endl;
        exit(0);
    }
}

int main()
{
    TestCase1();
    TestCase2();

    return 0;
}