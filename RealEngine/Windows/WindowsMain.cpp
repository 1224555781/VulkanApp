#pragma once

#include "iostream"
#include "any"
#include "array"
#include <vector>
#include <thread>
//#include "MultiThreadTest.h"
#include "glm/glm.hpp"
#include "GLFW/include/GLFW/glfw3.h"
#include <map>

#include "Core/Public/Main.h"
#include "Core/Public/VulkanApplication.h"



class FMemmorySystem
{
public:
	FMemmorySystem() {

	}

	virtual ~FMemmorySystem() {

	}

	virtual void* Malloc(size_t size)
	{
		return ::malloc(size);
	}

	virtual void Free(void* Ptr)
	{
		::free(Ptr);
	}

	virtual void* Realloc(void* Ptr, size_t size)
	{
		return ::realloc(Ptr, size);
	}
};

class FAnsiMalloc :public FMemmorySystem
{
	virtual void* Malloc(size_t size) override
	{

	}

};

void* operator new(size_t size)
{
	void* ReturnAddress = ::malloc(size);

	Print(printf( "New Somthing %p", ReturnAddress));
	return ReturnAddress;
}

void operator delete(void* Ptr)
{
	Print("Delete ", Ptr);
	::free(Ptr);
    memset(Ptr, 0xcd, sizeof(Ptr));
}



int main()
{

#if __cplusplus == 202002L
	auto TemplateLamb = []<typename T>(T t, auto && U)->decltype(auto) { return t + U; };

	auto Value = TemplateLamb(5, 32);

	Print(Value);
#endif // __cplusplus == 202002L
	Test::template_test->TestFunction();
	VulkanApplication* application = new VulkanApplication;
	application->InitApplication();
	application->MainLoop();
	application->Destroy();
	delete application;

	return 0;
}