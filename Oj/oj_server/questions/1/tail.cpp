#include "head.cpp"

void TestCase1()
{
    vector<int> nums = {1, 2, 3, 2, 2, 2, 5, 4, 2};
    int ret = Solution().majorityElement(nums) == 2;
    if (ret == 2)
    {
        cout << "通过用例通过:1/2..." << endl;  
    }
    else
    {
        cout << "测试用例通过:0/2..." << endl;
        cout << "用例输入:[1, 2, 3, 2, 2, 2, 5, 4, 2]" << endl;
        cout << "预期输出:2 你的输出:" << ret << endl;
    }
}

void TestCase2()
{
    vector<int> nums = {1,3,5,7,9,6,6};
    int ret = Solution().majorityElement(nums);
    if (ret == 6)
    {
        cout << "通过用例通过:2/2..." << endl;  
    }
    else
    {
        cout << "测试用例通过:1/2..." << endl;
        cout << "用例输入:[1, 3, 5, 7, 9, 6, 6]" << endl;
        cout << "预期输出:6 你的输出:" << ret << endl;
    }
}

int main()
{
    TestCase1();
    TestCase2();

    return 0;
}