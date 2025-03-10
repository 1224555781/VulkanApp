#include "Core/Public/Main.h"

#include "Core/Public/template_test.h"
#include "Core/Public/SmartPointer.h"
#include<memoryapi.h>
#include <variant>
Test* Test::template_test = new Test();



template<typename ...T>
struct overload : T...
{
    using T::operator()...;
};

template< typename ...T>
overload(T ...) -> overload<T...>;





struct FVector2D
{
    float x;
    float y;

	static bool IsNearlyEqual(float a, float b, float epsilon = 1e-5f) {
		return std::fabs(a - b) < epsilon;
	}

    friend std::partial_ordering operator<=>(const FVector2D&A, const FVector2D&B)
    {
        if (A.x>B.x && A.y>B.y)
        {
			return  std::partial_ordering::greater;
        }

		if (A.x <  B.x && A.y < B.y)
		{
			return  std::partial_ordering::less;
		}


		if (IsNearlyEqual(A.x ,B.x,0.003f) && IsNearlyEqual( A.y,B.y))
		{
			return  std::partial_ordering::equivalent;
		}

		return std::partial_ordering::unordered;
    }
};


TestSharedPtr::~TestSharedPtr()
{
    Print("TestSharedPtr Destroy");
}

void Test::TestFunction()
{


#pragma region C20Book
	ADLTest::MyStruct adltest;
	(ADL)(adltest);
	Print(__cplusplus);
    //CallFunc(5);
    
    CallFunc(std::vector<int32>{1,2,0,2,3,3,23,2},32);
	static_assert(IsVector<std::vector<int32>, int32[4]>);
	static_assert(IsIntegral<int,int64,int64>);
#pragma endregion



#pragma region Util
	FVector2D A{ 6.f,0.5f };
	FVector2D B{ 8.f,1.5f };
    if (A>B)
    {
        Print("A > B");
    }
    else if (A < B)
    {
		Print("A < B");
    }
#pragma endregion


#pragma region Union
    std::variant<int, float, std::string> v;

    v = 12;
    //consteval  int testV =  std::get<int>(v);
	//Print(testV);
    v = 18.0f;
    v = "MLB";

	std::visit(
		overload{ [](auto arg) { Print(arg); },
		[](float arg) {Print(arg); },
			[](std::string const& arg) {Print(arg); }
		}, v);
	Print(std::get<int>(v));

#pragma endregion

#pragma region R_XValueTest
	{
		Print("Start ");
		FPrint&& RPrint = FPrint::GetPrint();

		Print("End ");
	}
	

#pragma endregion

#pragma region FuntionTest

	//TFunction<void(int32)> Func = [](int32 a) {Print(a); };
	std::function<void(int32)> Func = [](int32 a) {Print(a); };
    Func(5);
#pragma endregion



#pragma  region ProtectMemTest
    my_struct* s = new my_struct;
	DWORD oldProtect;
	//int* protectMem =(int*)VirtualProtect(s, sizeof(my_struct), PAGE_READONLY, &oldProtect);



	//s->a = 99;
#pragma endregion
	delete s;
	TestSharedPtr* test_shared_ptr = new TestSharedPtrDrive;
	delete test_shared_ptr;
   
#pragma region SharePtr

	{
		TSharedPtr<TestSharedPtr> Sptr = MakeShareable(new TestSharedPtr);
		TSharedPtr<TestSharedPtr> Sptr2 = Sptr;
	}
#pragma endregion


#pragma region ReadPaddingAddress
	FStructWithPadding StructWithPadding{};
	uint8* StartAddress = reinterpret_cast<uint8*>(&StructWithPadding);
	StartAddress += 16;
	uint8 V_Padding = *StartAddress;
#pragma endregion

#pragma region ReadPaddingAddress

	FBaseClass* BaseClass = new FDerivedClass();

	delete BaseClass;
#pragma endregion

#pragma region SIMD
	int a = 10;
	int b = 20;
	int result;




	FASMTest* obj;
	__asm {


		    mov eax, a;    // 将变量 a 的值移动到 eax 寄存器
		    add eax, b;   // 将变量 b 的值加到 eax 寄存器
		    mov result, eax;// 将 eax 寄存器的值移动到 result 变量


			// Free the memory
			//add esp, 8

	}
	

	Print(result);
#pragma endregion
	Print("Start MLB Cotr");
	//MLBClass* C = new MLBClass;
	//Func g = &MLBClass::Function;
	//typedef void func(void);
	//Func* f = (Func*)0x7FF7C320114028;
	//(Class.*g)();
	//MLBClass* d = new (C)MLBClass();
	float* TestNewFloat = new float[2] {0.f};

	float* PNewFloat = new (TestNewFloat) float(5.f);

	auto temp = binary<102>::value;
	Print(temp);
	Print(4 | 1);
	std::cout << std::hex << TestNewFloat << "\n";
	Print(printf("TestNewFloat address %p", TestNewFloat));
	Print(printf("TestNewFloat address %p", PNewFloat));
	Print(printf("TestNewFloat address %f", *PNewFloat));
	//test  AddressSan
	//TestNewFloat[2] = 58.f;
	//delete []TestNewFloat;

	Print(::sqrt(2));

	int&& testRightint = 3;
	int& p = testRightint;
	std::string Result = _Is_Reference_<decltype(321)>::value ? "true" : "false";
	Print(Result);

	Print(std::is_same<int, float>::value ? "true" : "false");



	Print(std::is_integral<char>::value ? "true" : "false");
	Order(new int(5), 5);
	Orde(new int(5), 2.0);
	Print(HasMember<bool_value_true>::value ? "true" : "false");
	MLB_Tuple(3, "aaa");


	//MultiThreadTest MultiThread;
	//MultiThread.Run();
	int* pInt = new int(50);
	void* VoidpInt = pInt;
	std::cout << &(*pInt) << "\n";
	std::cout << &(VoidpInt) << "\n";
	std::string pChar = "MLB";
	void* pStr = &pChar;
	//VoidpInt = pStr;

	std::string Result1 = reinterpret_cast<const char*>(pStr);
	int* Result11 = reinterpret_cast<int*>(VoidpInt);
	Print(*Result11);
	const char* testcharlength = "aaaaa";
	Print(std::strlen(testcharlength));
	Print(sizeof(testcharlength));
	std::vector<int> vec{ 1,3,5,9,4,45 };
	for (size_t i = 0; i < vec.capacity(); i++)
	{
		Print(vec[i]);
	}
	Print("--------------------");
	vec.erase(vec.begin() + 2, vec.begin() + 3);
	Print(vec.capacity());
	vec.shrink_to_fit();
	Print("--------------------");
	Print(vec.capacity());
	/*for (size_t i = 0; i < vec.capacity(); i++)
	{
		Print(vec[i]);

	}*/

	//MultiThreadTest thread_test;
	//thread_test.PromisTestRun();

	class Base
	{
	public:
		const std::map<int, std::string> CopyOrEmptyReference()
		{
			std::map<int, std::string> RVO;
			return RVO;
		}
	};

	class Derived :public Base
	{
	public:

	};

	Base* base = new Derived();
	//std::map<int, std::string> er = base->CopyOrEmptyReference();


	Print(TDerivedFrom<float, std::string>::Result ? "true" : "false");
	std::string Str = "============ = over============ = ";
	Print(Str);
	std::cout << "cc" << Str << "\n";
	Print("game over");
#ifdef MM
	Print("MM");
#else
	Print("CC");
#endif
	InlineTest();


	Print("Empty Class: ", sizeof(EmpltyClass));

#pragma region  stdThreadTest
	// Demonstrate using promise<int> to transmit a result between threads.
	// std::vector<int> numbers = { 1, 2, 3, 4, 5, 6 };
	// std::promise<int> accumulate_promise;
	// std::future<int> accumulate_future = accumulate_promise.get_future();
	// std::thread work_thread(accumulate, numbers.begin(), numbers.end(),
	// 	std::move(accumulate_promise));

	// future::get() will wait until the future has a valid result and retrieves it.
	// Calling wait() before get() is not needed
	//accumulate_future.wait();  // wait for result
	//std::cout << "thread result = " << accumulate_future.get() << '\n';
	///(accumulate_future.get());
	//work_thread.join();  // wait for thread completion
	//Print("Join work_thread");
#pragma  endregion
	delete base;
	delete[]TestNewFloat;
}
