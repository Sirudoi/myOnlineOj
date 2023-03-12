#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
using namespace std;

// 测试时间戳转换
// int main(){
// 	char myStr[25] = { 0 };
// 	time_t cur_t = 1678503223;
// 	struct tm *t = gmtime(&cur_t);
// 	t->tm_hour += 8;//转为北京时间记的要加8
// 	std::string myFormat = "%Y-%m-%d:%H:%M:%S";

//     std::string s;
// 	strftime(myStr, sizeof(myStr), myFormat.c_str(), t);
// 	for (int i = 0; myStr[i]; ++i) {
// 		cout << myStr[i];
// 	}
// 	cout << endl;
// 	return 0;
// }

// 测试资源限制
int main()
{
    // 限制CPU占用时间
    struct rlimit cpu_limit;
    cpu_limit.rlim_cur = 10; //软资源，外部传入
    cpu_limit.rlim_max = RLIM_INFINITY; //硬资源，设置成无限
    setrlimit(RLIMIT_CPU, &cpu_limit);

    // 限制可开辟内存大小
    struct rlimit men_limit;
    cpu_limit.rlim_cur = 1024 * 1024; //软资源，外部传入
    cpu_limit.rlim_max = RLIM_INFINITY; //硬资源，设置成无限
    setrlimit(RLIMIT_AS, &cpu_limit);

    while (1) {
        new int[1024];
        cout << "new int[1024]" << endl;
    }


    return 0;
}                         