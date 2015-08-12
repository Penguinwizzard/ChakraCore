

namespace Js
{
    template <typename T>
    class BranchDictionaryWrapper
    {
    public:
        typedef JsUtil::BaseDictionary<T, void*, NativeCodeData::Allocator> BranchDictionary;

        BranchDictionaryWrapper(NativeCodeData::Allocator * allocator, uint dictionarySize) :
            defaultTarget(null), dictionary(allocator)
        {
        }

        BranchDictionary dictionary;
        void* defaultTarget;

        static BranchDictionaryWrapper* New(NativeCodeData::Allocator * allocator, uint dictionarySize)
        {
            return NativeCodeDataNew(allocator, BranchDictionaryWrapper, allocator, dictionarySize);
        }

    }; 

    class JavascriptNativeOperators
    {
    public:
        static void * Op_SwitchStringLookUp(JavascriptString* str, Js::BranchDictionaryWrapper<Js::JavascriptString*>* stringDictionary, uintptr funcStart, uintptr funcEnd);
    };
};