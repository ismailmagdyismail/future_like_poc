#include "ThreadPool.h"
#include "ExecutionContext.hpp"
#include <iostream>
int main()
{
    ThreadPool pool;

    std::shared_ptr<ExecutionContext> context1 = pool.AddTask([]()
                                                              { std::cout << "Task 1 \n"; });
    int result = context1->Wait();
    std::cout << "REsult from context 1 " << result << std::endl;
}