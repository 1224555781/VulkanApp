#pragma once
#include <any>
#include <future>
#include <map>
#include <numeric>

template<typename T,typename ...Args>
void Print(T&& Param,const Args& ...args)
{
	//
	T t = std::forward<T>(Param);
	std::cout << t << "\n";
    if constexpr (sizeof ...(Args) > 0)
    {
		Print(args...);
    }
}


// primary template
template <int N>  // non-type parameter N
struct binary {
	// an template instantiation inside the template itself, which contruct a recursion
	static constexpr int value = binary<N / 10>::value << 1 | N % 10;
};

// full specialization when n == 0
template <> struct binary<0> {
	static constexpr int value = 0;
};



template<typename T, T t>
struct const_value
{
	using type = const_value;
	using value_type = T;
	constexpr static T value = t;
	value_type operator()()
	{
		return  value;
	}
};

template<bool value>
using bool_const = const_value<bool, value>;

//template<>
using bool_value_true = bool_const<true>;
using bool_value_false = bool_const<false>;

template<int value>
using integer_const = const_value<int, value>;





template<typename T> struct _Is_Reference_ :bool_value_false
{
	//constexpr static  bool value = false;
};


template<typename T> struct _Is_Reference_<T&> :bool_value_true
{
	//constexpr static  bool value = true;
};


template<typename T>struct _Is_Reference_<T&&> : bool_value_true
{
	//constexpr static  bool value = true;

};


template<typename T>
struct Identify { using type = T; };

template<bool enable, typename U = void>
struct enable_if : Identify<U> {};
template<>
struct enable_if<false> {};

template<typename T>
inline void Order(T)
{
	Print(1);
}

template</*typename T*/>
inline  void Order(int*)
{
	Print(3);
}

template<typename T, typename U = std::enable_if_t<std::is_integral<decltype(3)>::value>>
inline void Order(T*)
{
	Print(2);
}


template<typename T, typename U = std::enable_if<std::is_integral<int>::value, int>::type>
inline void Order(T, U u)
{
	Print(2);
}

template<typename T, typename U = typename std::enable_if<std::is_floating_point<T>::value, int>::type>
inline void Orde(T, U u)
{
	Print(5);
}

template<typename T = void>
void ff()
{
	Print("disable------>");
}

template<int value>
enable_if<integer_const<value>::value> ff()
{
	Print("enable------>");
}

template<typename ...>
using void_t = void;

template<typename, typename T = void>
struct HasMember : bool_value_false
{
	static void Print()
	{
		std::cout << "this is Primary \n";
	}
};

template<typename T>
struct HasMember<T, void_t< typename T::type, int, T>> : bool_value_true
{

	static  void Print()
	{
		std::cout << "this is Partical \n";
	}
};

template<typename T, typename ...U>
struct MLB
{
	static void Print()
	{
		//Print();

		//std::cout << "This Is Primary \n " << typeid(typename  U...).name();
		//printf("aaaaaa %s", typeid(U...).name());
	}

	static void Print(std::string Str)
	{
		//Print();
		std::cout << Str << "\n";
		//std::cout << "This Is Primary \n " << typeid(typename  U...).name();
		//printf("aaaaaa %s", typeid(U...).name());
	}

private:

};



template<>
struct MLB<bool>
{
	static void Print()
	{
		std::cout << "This Is Partical \n";
	}
};

template<typename ...Ty>
struct MLB_Tuple
{
	MLB_Tuple(Ty...) {}

};

template<typename T, typename ...Ty>
struct MLB_Tuple<T, Ty ...> : MLB_Tuple<Ty ...>
{

	MLB_Tuple(T value, Ty ... Parm)
		:MLB_Tuple<Ty ...>(Parm...), _V(value)
	{
		Print(typeid(T).name());
		Print(_V);
	}

	T _V;
};



struct CommonT
{
	template<typename _Ty, typename ...Args, typename std::enable_if<std::is_constructible<_Ty>::value, int>::type = 0>
	CommonT(std::in_place_type_t<_Ty>, Args... Arg) {

	}

	template<typename _Ty>
	CommonT(_Ty value)
	{

	}

	template<typename _Ty, typename std::enable_if<std::is_same<_Ty, int>::value, int>::type = 0>
	CommonT(_Ty&& value)
		:Value(value)
	{

	}

	void Print()
	{

		//std::cout << "This Is Primary   " << Value << "\n";
	}

	struct RTTIBase {

	};

	template<typename T = void>
	struct RTTI :RTTIBase
	{
	public:
		RTTI() {};
		~RTTI() {};

		template<typename _Ty, std::enable_if_t<std::is_same_v<T, std::decay_t<_Ty>>, void>>
		RTTI(_Ty&& ty)
			:RealPtr(std::forward<_Ty>(ty))
		{
		}
	private:
		T RealPtr;
	};


private:
	RTTIBase Value;
};
/// <summary>
///  UnName Param to sfine right func template
/// </summary>
/// <typeparam name="_Ty"></typeparam>
/// <returns></returns>
template<typename _Ty, typename std::enable_if<std::is_same_v<_Ty, bool>, int>::type* = nullptr>decltype(auto) Commom()
{
	std::cout << "This Is bool \n";
	return false;
}

template<typename _Ty, typename std::enable_if<std::is_same_v<_Ty, int>, int>::type* = nullptr>decltype(auto) Commom()
{
	std::cout << "This Is int \n";
	return 32;
}

struct MyStruct
{
	MyStruct() = default;
};

template<typename T>
struct TestConstructable
{
	inline static int value = __is_constructible(T);
};


template<typename T,typename U>
struct Conversion
{
private:
	typedef char small;
	class Big { char dummy[2]; };
	static T MakeT();
	static small TestU(U);
	static Big TestU(...);

public:
    enum {exits = sizeof(TestU(MakeT())) == sizeof(small) ,SameType = false };
};

template<typename T>
struct Conversion<T,T>
{
public:
    enum  {SameType = true,exits = true };
};

template<typename T,typename U>
struct TDerivedFrom
{
	enum { Result = Conversion<T, U>::exits && !Conversion<T, U>::SameType };
};

template<typename T>
class TypeTraits
{
	template<typename  U>
	struct IsPointerType
	{
        enum  {result =false  };
	};
	template<typename U>
	struct IsPointerType<U*>
	{
        enum  {reslut = true  };
	};

public:
    enum  {Result  =true  };

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
inline Test* Test::template_test = new Test();

inline void InlineTest() {
	Print(printf("MLB %llu\n", _AddressOfReturnAddress()));
}


class EmpltyClass
{
	using StringPair = std::pair<std::string,std::string>;	
};


inline void accumulate(std::vector<int>::iterator first,
	std::vector<int>::iterator last,
	std::promise<int> accumulate_promise)
{
	Print("Start Calc Data");
	int sum = std::accumulate(first, last, 0);
	accumulate_promise.set_value(sum);  // Notify future
}

// __restrict 关键字对clang 11 是没用的  gcc12.1 有优化 
inline void RestrictFunc(int* __restrict restrictPtr,float const * constPtr)
{
	int* __restrict RestrictTest_Internal = restrictPtr;
}


inline void Test::TestFunction()
{

	//MLBClass Class;
	//Func g = &MLBClass::Function;


	//typedef void func(void);
	//Func* f = (Func*)0x7FF7C320114028;
	//(Class.*g)();
	
	float* TestNewFloat = new float[2]{0.f};

	auto temp = binary<102>::value;
	Print(temp);
	Print(4 | 1);
	delete []TestNewFloat;

	Print(::sqrt( 2));
	delete &TestNewFloat[3];
	int&& testRightint = 3;
	int& p = testRightint;
	std::string Result = _Is_Reference_<decltype(3)>::value ? "true" : "false";
	Print(Result);

	Print(std::is_same<int, float>::value ? "true" : "false");



	Print(std::is_integral<char>::value ? "true" : "false");
	Order(new int(5), 5);
	Orde(new int(5), 2.0);
	Print(HasMember<bool_value_true>::value ? "true" : "false");
	MLB_Tuple(3, "aaa");

	//CommonT T = 3;
	std::any an = 10;
	auto te = std::any_cast<int>(an);
	Print(te);

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

   
	Print("Empty Class: ",sizeof(EmpltyClass));


	// Demonstrate using promise<int> to transmit a result between threads.
	std::vector<int> numbers = { 1, 2, 3, 4, 5, 6 };
	std::promise<int> accumulate_promise;
	std::future<int> accumulate_future = accumulate_promise.get_future();
	std::thread work_thread(accumulate, numbers.begin(), numbers.end(),
		std::move(accumulate_promise));

	// future::get() will wait until the future has a valid result and retrieves it.
	// Calling wait() before get() is not needed
	//accumulate_future.wait();  // wait for result
	//std::cout << "thread result = " << accumulate_future.get() << '\n';
	Print(accumulate_future.get());
	work_thread.join();  // wait for thread completion
	Print("Join work_thread");
	delete base;
}

// 多个cpp 包含 会在每个cpp生成一个实体  内存开销++
//static int a = 1;

//多个cpp 包含 - 外部链接 - 会产生链接错误 
// int  a = 1;

// 内部链接 无视多个cpp 包含
// inline int a =1;