#pragma once
#include <any>
#include <future>
#include <map>
#include <numeric>
#include <iostream>

#include "VulkanTypeDefine.h"

// DLL export and import definitions
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)







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



inline void InlineTest() {
	Print(printf("MLB %p\n", _AddressOfReturnAddress()));
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

// ___restrict 关键字在 clang 11 中不可用，在 gcc 12.1 中可用
inline void RestrictFunc(int* __restrict restrictPtr,float const * constPtr)
{
	int* __restrict RestrictTest_Internal = restrictPtr;
}


inline int memTest()
{
	int a;
	std::cin >> a;
	return a;
}

class MLBClass
{
public:

	int _m = ::memTest();

	MLBClass() {
		Print("MLB Cotr");
	}

	~MLBClass()
	{
		Print("MLB Dtor");
	}

    static void StaticFuncTest();

private:

};

 //  ADL ʵ���������� ����ͨ�������ҵ���Ӧ�������ռ�
namespace ADLTest 
{
	struct MyStruct
	{

	};

	inline  void ADL( MyStruct Arg)
	{

	}
}

inline void ADL(ADLTest::MyStruct Arg)
{

}

// if some code read padding address, it will cause crash?
struct alignas(32) FStructWithPadding
{
	int FirstAddress;
    char* SecondAddress;
};


class FBaseClass
{
public:
	
    virtual ~FBaseClass() = default;
protected:
	int* MemberClass = nullptr;
};


class FDerivedClass final:public FBaseClass
{
public:
    FDerivedClass()
    {
        MemberClass = new int(5);
    }

	virtual ~FDerivedClass() override
    {
        delete MemberClass;
    }
};





class FMath
{
    /** Returns higher value in a generic way */
    template <class T>
    static constexpr inline T Max(const T A, const T B)
    {
        return (A >= B) ? A : B;
    }
};

template<int32 Size,int32 Alignment>
struct TAlignedBytes;

template<int32 Size>
struct TAlignedBytes<Size,1>
{
    uint8 Data[Size];
};

#define AlignedBytes(alignment) \
template<int32 Size> \
struct TAlignedBytes<Size, alignment> \
{ \
    struct alignas(alignment) my_struct\
    {\
       uint8 Data[Size]; \
    };\
    my_struct s;\
};

AlignedBytes(2)
AlignedBytes(4)
AlignedBytes(8)
AlignedBytes(16)

template<typename T>
struct TCompatibleAlignment : public TAlignedBytes<sizeof(T),alignof(T)>
{
	
	T* GetTypePtr()
	{
        return reinterpret_cast<T*>(&TAlignedBytes<sizeof(T), alignof(T)>::s);
	}

	const T* GetTypePtr() const
	{
		return reinterpret_cast<T*>(&TAlignedBytes<sizeof(T), alignof(T)>::s);
	}
};

template<typename T>
struct TNot {
    static constexpr bool value = !T::value;
};

template<typename Ret,typename ...Args>
struct TFunction
{
    //using FuncType = Ret(Args...);
	// TFunction(FuncType Fun)
	// {
	// 	Function = Fun;
	// }

	TFunction(TYPE_OF_NULLPTR = nullptr)
	{
		Function = nullptr;
	}

	template<typename T,typename = typename  std::enable_if<TNot<std::is_function<T>>::value>::type>
    TFunction(T&& Func)
	{
		Function = Func;
	}

	Ret operator()(Args...args)
	{
       return  Function(args...);
	}

private:
    typedef Ret(*FunctionType)(Args...);
	FunctionType Function;
};


struct FPrint
{
	static FPrint GetPrint(){
		return FPrint{};
    }
    std::string Str = "MLB";
    FPrint()
    {
		Print("Ctor FPrint");
    }

	~FPrint()
    {
		Print("Dtor FPrint");
    }
};



#if __cplusplus >= 202002L || 1


template<typename T>
concept IsSmall = sizeof(T) < 32;


template<typename ...T>
concept IsIntegral = (std::is_integral_v<T> && ... ) && (IsSmall<T> && ...);


template<typename ...T>
concept IsVector = ((std::is_same_v<T, std::vector<typename T::value_type>> || std::is_array_v<T>) && ...);
//函数声明
template< typename T,typename ...Args>
[[noreturn]] void _fastcall CallFunc(Args&& ...args);
//函数定义
template<typename T, typename ...Args>
inline void(CallFunc)(T&& arg, Args&& ...args) {


    if constexpr  (IsVector<T>)
    {
        Print("IsVector");
    }
    else
    {
		Print("testFunc", arg);
    }

	if (sizeof...(args)>0)
	{
        CallFunc(std::forward<Args>(args)...);
	}
};

#endif

// ceate a new one when  include this .h once;
//static int a = 1;

// link error if include twice
// int  a = 1;

// valid in one module include multi , other module will create new one
// inline int a =1;

//inline  MLBClass* TestClass = new MLBClass;