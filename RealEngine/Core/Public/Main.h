#pragma once

#include "template_test.h"



#if !defined(_AMD64_) && !defined(_X86_) && !defined(_ARM64EC_) && !defined(_IA64_) && !defined(_ARM_) && !defined(_ARM64_)
#define _X86_ // 或者根据您的实际目标架构定义相应的宏
#endif

struct my_struct
{
	int32 a = 77;
	int32 b = 88;
    float c = 99.0f;
    const char* d = "This is a test";
};

class TestSharedPtr
{
public:
	int32 ID = 9588;
	virtual ~TestSharedPtr();
};

class  TestSharedPtrDrive :public TestSharedPtr
{

public:

    const char* Name = "TestSharedPtrDrive";
	int32* acc = nullptr;
	double aq = 0.f;
    virtual ~TestSharedPtrDrive() override
    {
        Print("TestSharedPtrDrive Destroy");
    }
};

class FASMTest {
public:
	FASMTest(int a, int b) {
		std::cout << "Constructor called with a = " << a << " and b = " << b << std::endl;
	}

	~FASMTest() {
		std::cout << "Destructor called" << std::endl;
	}

	void MyMemberFunction() {
		std::cout << "MyMemberFunction called" << std::endl;
	}
};

class Test
{
public:

	Test()
	{
		Print("Test Construct===");
		const int& a = 10;
		//int* b = &a;
		auto&& param0 = 99;


		auto&& param2 = &param0;
		const auto& param = 96;

	}

	static Test* template_test;
	void TestFunction();
};