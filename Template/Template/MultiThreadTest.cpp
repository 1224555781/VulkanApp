
//#include "MultiThreadTest.h"
//
//#include <future>
//
//#include "iostream"
//void MultiThreadTest::Run()
//{
//	IsSuccess = true; 
//	for (size_t i = 0; i < MultiThreads.capacity(); i++)
//	{
//		MultiThreads[i].join();
//	}
//}
//
//void MultiThreadTest::PromisTestRun()
//{
//	STD promise<int> result_Promise;
//	STD future<int> result_Future = result_Promise.get_future();
//	STD thread thread{ &MultiThreadTest::PromisTestFunc,STD move(result_Promise) };
//
//	int result = result_Future.get();
//	STD cout << " result: " << result << STD endl;
//	thread.join();
//}
//
//void MultiThreadTest::PromisTestFunc(STD promise<int> result_promise)
//{
//	result_promise.set_value(99);
//}
//void MultiThreadTest::Task(int Count)
//{
//	while(!IsSuccess)
//	{
//
//	}
//	for (size_t i = 0; i < 10000000; i++)
//	{
//
//	}
//	bool canNotify = atomic.test_and_set();
//	if (!canNotify)
//	{
//		STD cout << " Thread ID #  " << Count << "\n";
//	}
//	
//}