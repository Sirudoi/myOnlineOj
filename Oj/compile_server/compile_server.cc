#include "compiler.hpp"
#include "runner.hpp"

int main()
{
    std::string src = "test";

    ns_complier::Compiler::CompileService(src);
    ns_runner::Runner::ExecuteService(src, 1, 20 * 1024);

    return 0;
}