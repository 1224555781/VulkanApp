
#include"Core/Public/template_test.h"
#include <intrin.h>
#include"Core/Public/VulkanTypeDefine.h"

class FThreadSafeCounter
{
public:
	FThreadSafeCounter()
    {
        Set(0);
    }
    FThreadSafeCounter(int32 InValue) 
    { 
        Set(InValue);
    }

    inline void Increament() {
        ::_InterlockedExchangeAdd8((char*)&Counter,1);
    }

    inline void Decreament()
    {
        ::_InterlockedIncrement16((short*)&Counter);
    }

    inline void Add(int32 Value)
    {
        ::_InterlockedExchangeAdd((long*)&Counter, Value);
    }

    inline void Subtract(int32 Value)
    {
        Add(Value);
    }

    inline void Set(int32 Value)
    {
        ::_InterlockedExchange((long*)&Counter, Value);
    }

    int32 GetValue()const;
private:
    volatile int32 Counter;
};


class FThreadCountObject
{
public:
    FThreadCountObject()
        :Counter(0)
    {}

    virtual ~FThreadCountObject() {
        check(Counter.GetValue() == 0);
    }

    void AddRef() 
    {
        Counter.Increament();
    }

    void Release()
    {
        Counter.Decreament();
        if (Counter.GetValue() == 0)
        {
            delete this;
        }
    }

    inline uint32 GetRefCount() 
    {
        return Counter.GetValue();
    }

private:
    mutable FThreadSafeCounter Counter;
};



template<typename R>
class TRefCountPtr
{
    typedef R ReferenceType;
public:
    explicit TRefCountPtr(ReferenceType* Ptr)
        :RawPtr(Ptr) 
    {
        if (RawPtr)
        {
            RawPtr->AddRef();
        }
    };
    ~TRefCountPtr() 
    {
        RawPtr->Release();
    };

    TRefCountPtr(TRefCountPtr* Ptr)
    {
        Ptr->RawPtr->AddRef();
        RawPtr = Ptr->RawPtr;
    }


    TRefCountPtr(const TRefCountPtr& Ptr)
    {
        if (Ptr.RawPtr)
        {
            Ptr.RawPtr->AddRef();
        }
        RawPtr = Ptr.RawPtr;
    }

    /// <summary>
    /// Copy Reference
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="Ptr"></param>
    template<typename T>
    explicit TRefCountPtr(const TRefCountPtr<T>& Ptr)
    {
        if (Ptr.RawPtr)
        {
            Ptr.RawPtr->AddRef();
        }
        RawPtr = Ptr.RawPtr;
    }



    TRefCountPtr(TRefCountPtr&& Ptr)
    {
        RawPtr = Ptr.RawPtr;
        Ptr.RawPtr = nullptr;
    }

    /// <summary>
    ///  Move Reference 
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="Ptr"></param>
    template<typename T>
    explicit TRefCountPtr(TRefCountPtr<T>&& Ptr)
    {
        RawPtr = Ptr.RawPtr;
        Ptr.RawPtr = nullptr;
    }



    TRefCountPtr& operator =(ReferenceType* Other)
    {
        ReferenceType* Old = RawPtr;
        RawPtr = Other;
        if (RawPtr)
        {
            RawPtr->AddRef();
        }
        Old->Release();
       
        return *this;
    }

    /// <summary>
    /// ���������� const by reference
    /// </summary>
    /// <param name="Other"></param>
    /// <returns>null</returns>
    TRefCountPtr& operator =(const ReferenceType& Other)
    {
        check(0);
        return *this;
    }

    template<typename T>
    TRefCountPtr& operator =(const TRefCountPtr<T>& Other)
    {
        *this = Other;

        return *this;
    }

    /// <summary>
    /// ����ֵ���ݹ���������������ָ����Ϊ��-ת������Ȩ����ǰָ��-���Ӳ�������
    /// </summary>
    /// <param name="Other"></param>
    /// <returns> "*this" </returns>
    TRefCountPtr& operator =(TRefCountPtr&& Other)
    {
        if (this!=&Other)
        {
            ReferenceType*Old = RawPtr;
            RawPtr = Other.RawPtr;
            Other.RawPtr = nullptr;

            Old->Release();
        }

        return *this;
    }

    bool IsValid()
    {
        return !!RawPtr;
    }
        

    ReferenceType* operator ->()const
    {
        return RawPtr;
    }

    operator ReferenceType*()const
    {
        return RawPtr;
    }


    void SafeRelease()
    {
        *this = nullptr;
    }

    uint32 GetRefCount()
    {
        if (RawPtr)
        {
            return RawPtr->GetRefCount();
        }
        return 0;
    }

    ReferenceType* GetPtr() {
        return RawPtr;
    }
private:


    ReferenceType* RawPtr;

    template <typename OtherType>
    friend class TRefCountPtr;
};


template<typename T>
class FRefControllerBase
{

public:
    virtual ~FRefControllerBase() = default;

    int32 RefCount = 0 ;

    void AddRef()
    {
        RefCount++;
    }

    void DecRef()
    {
        RefCount--;
    }

    virtual void DestroyObject() = 0;
};

template<typename T>
class FControllerPtr final :public FRefControllerBase<T>
{
    using Super = FRefControllerBase<T>;
    T* Object;
public:
    FControllerPtr(T* InObject)
        :Object(InObject)
    {
    }

    virtual void DestroyObject()override
    {
        delete Object;
    }
};


template<typename T>
class FControllerRef final :public FRefControllerBase<T>
{
    using Super = FRefControllerBase<T>;
    TCompatibleAlignment<T> Object;
public:
    template<typename ...Arg>
    FControllerRef(Arg && ... args)
    {
        new (&Object) T(Forward<Arg>(args)...);
    }

    T* GetObjectPtr()
    {
        return Object.GetTypePtr();
    }


    virtual void DestroyObject()override
    {
        if constexpr (std::is_trivially_destructible<T>::value)
        {
            GetObjectPtr()->T::~T();
        }
    }

};


template<typename T>
class TSharedPtr
{
public:
    TSharedPtr(T* InObject, FRefControllerBase<T>* InController)
        :Object(InObject), Controller(InController)
    {
        Controller->AddRef();
    }

    ~TSharedPtr()
    {
        Controller->DecRef();
        if (Controller->RefCount==0)
        {
            Controller->DestroyObject();
            delete Controller;
        }
    }

    TSharedPtr(const TSharedPtr& Ptr)
        :Object(Ptr.Object), Controller(Ptr.Controller)
    {
        Controller->AddRef();
    }

    TSharedPtr& operator =(const TSharedPtr& Ptr)
    {
        if (this != &Ptr)
        {
            Object = Ptr.Object;
            Controller = Ptr.Controller;
            Controller->AddRef();
        }
        return *this;
    }

    TSharedPtr& operator =(TSharedPtr&& Ptr) noexcept
    {
        if (this != &Ptr)
        {
            Object = Ptr.Object;
            Controller =  Ptr.Controller;
            Ptr.Controller = nullptr;
            Ptr.Object = nullptr;
        }

        
        return *this;
    }

    [[nodiscard]] bool IsValid()
    {
        return Object != nullptr;
    }

private:
    T* Object;
    FRefControllerBase<T>* Controller;

};

template<typename T>
inline FRefControllerBase<T>* MakeControllerPtr(T* Object)
{
    return new FControllerPtr<T>(Object);
}


template<typename T, typename ...Arg>
inline FRefControllerBase<T>* MakeControllerRef(Arg&& ...args)
{
    return new FControllerRef<T>(std::forward<Arg>(args)...);
}

template<typename T>
inline TSharedPtr<T> MakeShareable(T* Object)
{
    return TSharedPtr<T>(Object,MakeControllerPtr(Object));
}


template<typename T,typename ...Arg>
inline TSharedPtr<T> MakeShared(Arg&& ...args)
{
    FControllerRef<T>* Ref = static_cast<FControllerRef<T>*> (MakeControllerRef<T>(std::forward<Arg>(args)...));
    return TSharedPtr<T>(Ref->GetObjectPtr(), Ref);
}