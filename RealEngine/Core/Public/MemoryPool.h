#pragma once
#include <array>
#include <cstdlib>
#include <map>
#include <unordered_map>
static constexpr size_t BasicMallocSize = 8;
static constexpr size_t StandMultiSize = 128;

template<size_t MemSize>
class MemoryPool
{
    MemoryPool()
    {

    };

    //static constexpr size_t StandMultiSize = Size - Size % BasicMallocSize;
public:
    void* Alloca();

    void Dealloc(void* p);
private:

    template<size_t BlockSie>
    struct FFreeBlock
    {
        unsigned char data[BlockSie];
        FFreeBlock* Next;
    };

    //template<size_t MemSize>
    struct FMemChunk
    {
        FFreeBlock<MemSize>* Free__Block_Head = nullptr;

        FMemChunk* Next;
    };
    FMemChunk* Head = nullptr;
    FMemChunk* CurrentHead = nullptr;
    //std::unordered_map<size_t, FMemChunk>               MemeData;
};

template <size_t MemSize>
void* MemoryPool<MemSize>::Alloca()
{
    static constexpr  size_t InSize = MemSize + MemSize % StandMultiSize;
    if (Head == nullptr)
    {
        Head = new FMemChunk;
        CurrentHead = Head;
        Head->Free__Block_Head = new FFreeBlock<InSize>;
        auto HeadNext = Head->Free__Block_Head->Next;
        for (size_t i = 0 ; i<49;++i)
        {
            HeadNext = new FFreeBlock<InSize>;
            HeadNext = HeadNext->Next;
        }
    }
    if (CurrentHead->Free__Block_Head != nullptr)   
    {
        void* O = CurrentHead->Free__Block_Head;
        CurrentHead->Free__Block_Head = CurrentHead->Free__Block_Head->Next;

        return  O;
    }
    else
    {
        //#TODO Ôö¼ÓHead
    }
}


