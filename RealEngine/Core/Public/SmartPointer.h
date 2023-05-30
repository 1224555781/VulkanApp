#include"Core/Public/template_test.h"
#include"Core/Public/VulkanTypeDefine.h"

class FThreadSafeCounter
{
public:
	FThreadSafeCounter()
    {
        Set(0);
    };
    FThreadSafeCounter(int32 InValue) 
    { 
        Set(InValue);
    };

    inline void Increament() {
        ::_InterlockedIncrement((long*)&Counter);
    }

    inline void Decreament()
    {
        ::_InterlockedDecrement((long*)&Counter);
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
    };

    void Release()
    {
        Counter.Decreament();
        if (Counter.GetValue() == 0)
        {
            delete this;
        }
    };

    inline uint32 GetRefCount() 
    {
        return Counter.GetValue();
    };

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
    /// 不允许传递 const by reference
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
    /// 临终值传递过来处理其真正的指针置为空-转移所有权到当前指针-壳子不做处理
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
