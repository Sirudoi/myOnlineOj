#ifndef INCLUDE_HEAD
#include "head.cpp"
#endif

void TestCase1()
{
    vector<int> nums = {2, 3, 1, 0, 2, 5, 3};
    vector<int> ret = Solution().exchange(nums);
    int left = 0;
    int right = ret.size();
    while (left < right)
    {
        if (ret[left] % 2 == 1 && ret[right] % 2 == 0)
        {
            ++left;
            --right;
        }
        else
        {
            cout << "测试用例通过:0/2..." << endl;
            cout << "用例输入:[2, 3, 1, 0, 2, 5, 3]" << endl;
            exit(0);
        }
    }
    cout << "测试用例通过:1/2..." << endl;
}

void TestCase2()
{
    vector<int> nums = {2, 4, 6, 1, 3, 5};
    vector<int> ret = Solution().exchange(nums);
    int left = 0;
    int right = ret.size();
    while (left < right)
    {
        if (ret[left] % 2 == 1 && ret[right] % 2 == 0)
        {
            ++left;
            --right;
        }
        else
        {
            cout << "测试用例通过:1/2..." << endl;
            cout << "用例输入:[2, 4, 6, 1, 3, 5]" << endl;
            exit(0);
        }
    }
    cout << "测试用例通过:2/2..." << endl;
}

int main()
{
    TestCase1();
    TestCase2();

    return 0;
}