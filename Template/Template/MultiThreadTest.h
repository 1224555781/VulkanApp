//#pragma once
//#include <vector>
//#include <thread>
//#include <atomic>
//#include <future>
//#define STD ::std::
//class MultiThreadTest
//{
//public:
//    void Run();
//    
//    MultiThreadTest()
//    {
//        for (int i = 0; i < 9; i++)
//        {
//            MultiThreads.push_back(std::thread(&MultiThreadTest::Task,this,i));
//        }
//    }
//
//    void PromisTestRun();
//    void PromisTestFunc(std::promise<int> result_promise);
//    void Task(int Count);
//private:
//    
//
//    std::vector<std::thread> MultiThreads;
//    std::atomic<bool> IsSuccess;
//    std::atomic_flag atomic = ATOMIC_FLAG_INIT;
//};
//
