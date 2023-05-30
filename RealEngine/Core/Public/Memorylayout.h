#include"CoreMinimal.h"


namespace ETypeLayoutInterface
{
	enum Type : uint8
	{
		NonVirtual,
		Virtual,
		Abstract,
	};

	inline bool HasVTable(Type InType) { return InType != NonVirtual; }
}


/**
 * Traits class which does concept checking.
 */
template <typename Concept, typename... Args>
struct TModels
{
	template <typename... Ts>
	static char(&Resolve(decltype(&Concept::template Requires<Ts...>)*))[2];

	template <typename... Ts>
	static char(&Resolve(...))[1];

	static constexpr bool Value = sizeof(Resolve<Args...>(0)) == 2;
};


struct CProvidesStaticStruct
{
	template<typename T>
	auto Requires(const T&) -> decltype(T::StaticStruct());
};

template<typename T>
struct TProvidesStaticStruct
{
	static const bool Value = TModels<CProvidesStaticStruct, T>::Value;
};






template<typename T, typename BaseType>
struct TInitializeBaseHelper
{

	static void Initialize(FTypeLayoutDesc& TypeDesc) { InternalInitializeBaseHelper<T, BaseType>(TypeDesc); }
};

namespace EFieldLayoutFlags
{
	enum Type : uint8
	{
		None = 0u,
		WithEditorOnly = (1u << 0),
		WithRayTracing = (1u << 1),
		Transient = (1u << 2),
		UseInstanceWithNoProperty = (1u << 3),
	};

	inline Type MakeFlags(uint32 Flags = None) { return (Type)Flags; }
	inline Type MakeFlagsEditorOnly(uint32 Flags = None) { return (Type)(WithEditorOnly | Flags); }
	inline Type MakeFlagsRayTracing(uint32 Flags = None) { return (Type)(WithRayTracing | Flags); }
}


