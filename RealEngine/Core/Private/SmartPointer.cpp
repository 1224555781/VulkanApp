#include "Core/Public/SmartPointer.h"

int32 FThreadSafeCounter::GetValue() const
{
    return (int32)::_InterlockedCompareExchange((long*)&const_cast<FThreadSafeCounter*>(this)->Counter, 0, 0);
     ;
}
