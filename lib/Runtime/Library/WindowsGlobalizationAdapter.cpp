//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "RuntimeLibraryPch.h"

#if defined(ENABLE_INTL_OBJECT) || defined(ENABLE_ES6_CHAR_CLASSIFIER)

#include "strsafe.h"

#define __WRL_ASSERT__(cond) Assert(cond)

#include <wrl\implements.h>

#ifdef _M_X64_OR_ARM64
// TODO: Clean this warning up
#pragma warning(disable:4267) // 'var' : conversion from 'size_t' to 'type', possible loss of data
#endif

#ifdef NTBUILD
using namespace Windows::Globalization;
using namespace Windows::Foundation::Collections;
#else
using namespace ABI::Windows::Globalization;
using namespace ABI::Windows::Foundation::Collections;
#endif

#define IfFailThrowHr(op) \
    if (FAILED(hr=(op))) \
    { \
        JavascriptError::MapAndThrowError(scriptContext, hr);\
    } \

#define IfNullReturnError(EXPR, ERROR) do { if (!(EXPR)) { return (ERROR); } } while(FALSE)
#define IfFailedReturn(EXPR) do { hr = (EXPR); if (FAILED(hr)) { return hr; }} while(FALSE)
#define IfFailedGoLabel(expr, label) if (FAILED(expr)) { goto label; }
#define IfFailedGo(expr) IfFailedGoLabel(expr, LReturn)


#ifdef ENABLE_INTL_OBJECT

//The "helper" methods below are to resolve external symbol references to our delay-loaded libraries.
HRESULT WindowsCreateString(_In_reads_opt_(length) const WCHAR * sourceString, UINT32 length, _Outptr_result_maybenull_ _Result_nullonfailure_ HSTRING * string)
{
    return ThreadContext::GetContextForCurrentThread()->GetWindowsGlobalizationLibrary()->WindowsCreateString(sourceString, length, string);
}

HRESULT WindowsCreateStringReference(_In_reads_opt_(length + 1) const WCHAR * sourceString, UINT32 length, _Out_ HSTRING_HEADER * header, _Outptr_result_maybenull_ _Result_nullonfailure_ HSTRING * string)
{
    return ThreadContext::GetContextForCurrentThread()->GetWindowsGlobalizationLibrary()->WindowsCreateStringReference(sourceString, length, header, string);
}

HRESULT WindowsDeleteString(_In_opt_ HSTRING string)
{
    return ThreadContext::GetContextForCurrentThread()->GetWindowsGlobalizationLibrary()->WindowsDeleteString(string);
}

PCWSTR WindowsGetStringRawBuffer(_In_opt_ HSTRING string, _Out_opt_ UINT32 * length)
{
    return ThreadContext::GetContextForCurrentThread()->GetWindowsGlobalizationLibrary()->WindowsGetStringRawBuffer(string, length);
}

HRESULT WindowsCompareStringOrdinal(_In_opt_ HSTRING string1, _In_opt_ HSTRING string2, _Out_ INT32 * result)
{
    return ThreadContext::GetContextForCurrentThread()->GetWindowsGlobalizationLibrary()->WindowsCompareStringOrdinal(string1, string2, result);
}

HRESULT WindowsDuplicateString(_In_opt_ HSTRING original, _Outptr_result_maybenull_ _Result_nullonfailure_ HSTRING *newString)
{
    return ThreadContext::GetContextForCurrentThread()->GetWindowsGlobalizationLibrary()->WindowsDuplicateString(original, newString);
}
#endif
namespace Js
{
#ifdef ENABLE_INTL_OBJECT
    class HSTRINGIterator : public Microsoft::WRL::RuntimeClass<IIterator<HSTRING>>
    {

        HSTRING *items;
        uint32 length;
        boolean hasMore;
        uint32 currentPosition;

    public:
        HRESULT RuntimeClassInitialize(HSTRING *items, uint32 length)
        {
            this->items = items;
            this->currentPosition = 0;
            this->length = length;
            this->hasMore = currentPosition < this->length;

            return S_OK;
        }
        ~HSTRINGIterator()
        {
        }

        // IIterator
        IFACEMETHODIMP get_Current(_Out_ HSTRING *current)
        {
            if (current != nullptr)
            {
                if (hasMore)
                {
                    return WindowsDuplicateString(items[currentPosition], current);
                }
                else
                {
                    *current = nullptr;
                }
            }
            return E_BOUNDS;
        }

        IFACEMETHODIMP get_HasCurrent(_Out_ boolean *hasCurrent)
        {
            if (hasCurrent != nullptr)
            {
                *hasCurrent = hasMore;
            }
            return S_OK;
        }

        IFACEMETHODIMP MoveNext(_Out_opt_ boolean *hasCurrent) sealed
        {
            this->currentPosition++;

            this->hasMore = this->currentPosition < this->length;
            if (hasCurrent != nullptr)
            {
                *hasCurrent = hasMore;
            }
            return S_OK;
        }

        IFACEMETHODIMP GetMany(_In_ unsigned capacity,
                               _Out_writes_to_(capacity,*actual) HSTRING *value,
                               _Out_ unsigned *actual)
        {
            uint count = 0;
            while (this->hasMore)
            {
                if (count == capacity)
                {
                    break;
                }
                if (value != nullptr)
                {
                    get_Current(value + count);
                }

                count ++;
                this->MoveNext(nullptr);
            }
            if (actual != nullptr)
            {
                *actual = count;
            }

            return S_OK;
        }
        IFACEMETHOD(GetRuntimeClassName)(_Out_ HSTRING* runtimeName) sealed
        {
            *runtimeName = nullptr;
            HRESULT hr = S_OK;
            const wchar_t *name = L"Js.HSTRINGIterator";

            hr = WindowsCreateString(name, static_cast<UINT32>(wcslen(name)), runtimeName);
            return hr;
        }
        IFACEMETHOD(GetTrustLevel)(_Out_ TrustLevel* trustLvl)
        {
            *trustLvl = BaseTrust;
            return S_OK;
        }
        IFACEMETHOD(GetIids)(_Out_ ULONG *iidCount, _Outptr_result_buffer_(*iidCount) IID **)
        {
            iidCount;
            return E_NOTIMPL;
        }
    };

    class HSTRINGIterable : public Microsoft::WRL::RuntimeClass<IIterable<HSTRING>>
    {

        HSTRING *items;
        uint32 length;

    public:
        HRESULT RuntimeClassInitialize(HSTRING *string, uint32 length)
        {
            this->items = new HSTRING[length];

            if (this->items == nullptr)
            {
                return E_OUTOFMEMORY;
            }

            for(uint32 i = 0; i < length; i++)
            {
                this->items[i] = string[i];
            }
            this->length = length;

            return S_OK;
        }

        ~HSTRINGIterable()
        {
            if(this->items != nullptr)
            {
                delete [] items;
            }
        }

        IFACEMETHODIMP First(_Outptr_result_maybenull_ IIterator<HSTRING> **first)
        {
            return Microsoft::WRL::MakeAndInitialize<HSTRINGIterator>(first, this->items, this->length);
        }

        IFACEMETHOD(GetRuntimeClassName)(_Out_ HSTRING* runtimeName) sealed
        {
            *runtimeName = nullptr;
            HRESULT hr = S_OK;
            // Return type that does not exist in metadata
            const wchar_t *name = L"Js.HSTRINGIterable";
            hr = WindowsCreateString(name, static_cast<UINT32>(wcslen(name)), runtimeName);
            return hr;
        }
        IFACEMETHOD(GetTrustLevel)(_Out_ TrustLevel* trustLvl)
        {
            *trustLvl = BaseTrust;
            return S_OK;
        }
        IFACEMETHOD(GetIids)(_Out_ ULONG *iidCount, _Outptr_result_buffer_(*iidCount) IID **)
        {
            iidCount;
            return E_NOTIMPL;
        }
    };
#endif

    __inline DelayLoadWindowsGlobalization* WindowsGlobalizationAdapter::GetWindowsGlobalizationLibrary(_In_ ScriptContext* scriptContext)
    {
        return this->GetWindowsGlobalizationLibrary(scriptContext->GetThreadContext());
    }

    __inline DelayLoadWindowsGlobalization* WindowsGlobalizationAdapter::GetWindowsGlobalizationLibrary(_In_ ThreadContext* threadContext)
    {
        return threadContext->GetWindowsGlobalizationLibrary();
    }

    template<typename T>
    HRESULT WindowsGlobalizationAdapter::GetActivationFactory(DelayLoadWindowsGlobalization *delayLoadLibrary, LPCWSTR factoryName, T** instance)
    {
        *instance = nullptr;

        AutoCOMPtr<IActivationFactory> factory;
        HSTRING hString;
        HSTRING_HEADER hStringHdr;
        HRESULT hr;

        IfFailedReturn(delayLoadLibrary->WindowsCreateStringReference(factoryName, wcslen(factoryName), &hStringHdr, &hString));
        AnalysisAssert(hString);
        IfFailedReturn(delayLoadLibrary->DllGetActivationFactory(hString, &factory));

        return factory->QueryInterface(__uuidof(T), reinterpret_cast<void**>(instance));
    }

    HRESULT WindowsGlobalizationAdapter::EnsureInitialized(ScriptContext *scriptContext)
    {
        return this->EnsureInitialized(this->GetWindowsGlobalizationLibrary(scriptContext), scriptContext->GetConfig()->IsES6UnicodeExtensionsEnabled());
    }

    HRESULT WindowsGlobalizationAdapter::EnsureInitialized(DelayLoadWindowsGlobalization *library, bool isES6Mode)
    {
        HRESULT hr = S_OK;

        if (initialized)
        {
            return hr;
        }
        else if (failedToInitialize)
        {
            return S_FALSE;
        }

        failedToInitialize = true;
#ifdef ENABLE_INTL_OBJECT
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_Language, &languageFactory));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_Language, &languageStatics));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_NumberFormatting_CurrencyFormatter, &currencyFormatterFactory));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_NumberFormatting_DecimalFormatter, &decimalFormatterFactory));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_NumberFormatting_PercentFormatter, &percentFormatterFactory));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_DateTimeFormatting_DateTimeFormatter, &dateTimeFormatterFactory));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_Calendar, &calendarFactory));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_NumberFormatting_SignificantDigitsNumberRounder, &significantDigitsRounderActivationFactory));
        IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Globalization_NumberFormatting_IncrementNumberRounder, &incrementNumberRounderActivationFactory));
       
#endif
        if(isES6Mode)
        {
            IfFailedReturn(GetActivationFactory(library, RuntimeClass_Windows_Data_Text_UnicodeCharacters, &unicodeStatics));
        }

        failedToInitialize = false;
        initialized = true;

        return hr;
    }

#ifdef ENABLE_INTL_OBJECT
    HRESULT WindowsGlobalizationAdapter::CreateLanguage(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR languageTag, ILanguage** language)
    {
        HRESULT hr = S_OK;
        HSTRING hString;
        HSTRING_HEADER hStringHdr;
        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(languageTag, wcslen(languageTag), &hStringHdr, &hString));
        AnalysisAssert(hString);
        IfFailedReturn(this->languageFactory->CreateLanguage(hString, language));
        return hr;
    }

    boolean WindowsGlobalizationAdapter::IsWellFormedLanguageTag(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR languageTag)
    {
        boolean retVal;
        HRESULT hr;
        HSTRING hString;
        HSTRING_HEADER hStringHdr;
        IfFailThrowHr(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(languageTag, wcslen(languageTag), &hStringHdr, &hString));
        AnalysisAssert(hString);
        IfFailThrowHr(this->languageStatics->IsWellFormed(hString, &retVal));
        return retVal;
    }


    boolean WindowsGlobalizationAdapter::ValidateAndCanonicalizeTimeZone(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR timeZoneId, HSTRING *result)
    {
        HRESULT hr = S_OK;
        HSTRING timeZone;
        HSTRING_HEADER timeZoneHeader;

        // Construct HSTRING of timeZoneId passed
        IfFailThrowHr(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(timeZoneId, wcslen(timeZoneId), &timeZoneHeader, &timeZone));
        Assert(timeZone);

        if (timeZoneCalendar == nullptr)
        {
            IfFailThrowHr(this->CreateTimeZoneOnCalendar(this->GetWindowsGlobalizationLibrary(scriptContext), &timeZoneCalendar));
        }
        // ChangeTimeZone should fail if this is not a valid time zone
        hr = timeZoneCalendar->ChangeTimeZone(timeZone);
        if (hr != S_OK)
        {
            return false;
        }
        // Retrieve canonicalize timeZone name
        IfFailThrowHr(timeZoneCalendar->GetTimeZone(result));
        return true;
    }

    void WindowsGlobalizationAdapter::GetDefaultTimeZoneId(_In_ ScriptContext* scriptContext, HSTRING *result)
    {
        HRESULT hr = S_OK;
        if (defaultTimeZoneCalendar == nullptr)
        {
            IfFailThrowHr(this->CreateTimeZoneOnCalendar(this->GetWindowsGlobalizationLibrary(scriptContext), &defaultTimeZoneCalendar));
        }

        IfFailThrowHr(defaultTimeZoneCalendar->GetTimeZone(result));
    }

    void WindowsGlobalizationAdapter::ReleaseWindowsGlobalizationObjects()
    {
        if (this->dateTimeFormatterFactory != nullptr)
        {
            this->dateTimeFormatterFactory.Detach()->Release();
        }

        if (this->calendarFactory != nullptr)
        {
            this->calendarFactory.Detach()->Release();
        }

        if (this->timeZoneCalendar != nullptr)
        {
            this->timeZoneCalendar.Detach()->Release();
        }

        if (this->defaultTimeZoneCalendar != nullptr)
        {
            this->defaultTimeZoneCalendar.Detach()->Release();
        }
    }
   
    HRESULT WindowsGlobalizationAdapter::NormalizeLanguageTag(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR languageTag, HSTRING *result)
    {
        HRESULT hr;

        AutoCOMPtr<ILanguage> language;
        IfFailedReturn(CreateLanguage(scriptContext, languageTag, &language));

        IfFailedReturn(language->get_LanguageTag(result));
        return hr;
    }

    HRESULT WindowsGlobalizationAdapter::CreateCurrencyFormatterCode(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR currencyCode, NumberFormatting::ICurrencyFormatter** currencyFormatter)
    {
        HRESULT hr;
        HSTRING hString;
        HSTRING_HEADER hStringHdr;
        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(currencyCode, wcslen(currencyCode), &hStringHdr, &hString));
        AnalysisAssert(hString);
        IfFailedReturn(this->currencyFormatterFactory->CreateCurrencyFormatterCode(hString, currencyFormatter));
        return hr;
    }

    HRESULT WindowsGlobalizationAdapter::CreateCurrencyFormatter(_In_ ScriptContext* scriptContext, PCWSTR* localeStrings, uint32 numLocaleStrings, _In_z_ PCWSTR currencyCode, NumberFormatting::ICurrencyFormatter** currencyFormatter)
    {
        HRESULT hr;
        HSTRING hString;
        HSTRING_HEADER hStringHdr;

        AutoArrayPtr<HSTRING> arr(HeapNewArray(HSTRING, numLocaleStrings), numLocaleStrings);
        AutoArrayPtr<HSTRING_HEADER> headers(HeapNewArray(HSTRING_HEADER, numLocaleStrings), numLocaleStrings);
        for(uint32 i = 0; i< numLocaleStrings; i++)
        {
            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(localeStrings[i],  wcslen(localeStrings[i]), (headers + i), (arr + i)));
        }

        Microsoft::WRL::ComPtr<IIterable<HSTRING>> languages(nullptr);
        IfFailedReturn(Microsoft::WRL::MakeAndInitialize<HSTRINGIterable>(&languages, arr, numLocaleStrings));

        HSTRING geoString;
        HSTRING_HEADER geoStringHeader;
        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(L"ZZ", 2, &geoStringHeader, &geoString));

        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(currencyCode, wcslen(currencyCode), &hStringHdr, &hString));
        IfFailedReturn(this->currencyFormatterFactory->CreateCurrencyFormatterCodeContext(hString, languages.Get(), geoString, currencyFormatter));
        return hr;
    }

    HRESULT WindowsGlobalizationAdapter::CreateNumberFormatter(_In_ ScriptContext* scriptContext, PCWSTR* localeStrings, uint32 numLocaleStrings, NumberFormatting::INumberFormatter** numberFormatter)
    {
        HRESULT hr = S_OK;

        AutoArrayPtr<HSTRING> arr(HeapNewArray(HSTRING, numLocaleStrings), numLocaleStrings);
        AutoArrayPtr<HSTRING_HEADER> headers(HeapNewArray(HSTRING_HEADER, numLocaleStrings), numLocaleStrings);
        for(uint32 i = 0; i< numLocaleStrings; i++)
        {
            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(localeStrings[i],  wcslen(localeStrings[i]), (headers + i), (arr + i)));
        }

        Microsoft::WRL::ComPtr<IIterable<HSTRING>> languages(nullptr);
        IfFailedReturn(Microsoft::WRL::MakeAndInitialize<HSTRINGIterable>(&languages, arr, numLocaleStrings));

        HSTRING geoString;
        HSTRING_HEADER geoStringHeader;
        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(L"ZZ", 2, &geoStringHeader, &geoString));

        IfFailedReturn(this->decimalFormatterFactory->CreateDecimalFormatter(languages.Get(), geoString, numberFormatter));
        return hr;
    }

    HRESULT WindowsGlobalizationAdapter::CreatePercentFormatter(_In_ ScriptContext* scriptContext, PCWSTR* localeStrings, uint32 numLocaleStrings, NumberFormatting::INumberFormatter** numberFormatter)
    {
        HRESULT hr = S_OK;

        AutoArrayPtr<HSTRING> arr(HeapNewArray(HSTRING, numLocaleStrings), numLocaleStrings);
        AutoArrayPtr<HSTRING_HEADER> headers(HeapNewArray(HSTRING_HEADER, numLocaleStrings), numLocaleStrings);
        for(uint32 i = 0; i< numLocaleStrings; i++)
        {
            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(localeStrings[i],  wcslen(localeStrings[i]), (headers + i), (arr + i)));
        }

        Microsoft::WRL::ComPtr<IIterable<HSTRING>> languages(nullptr);
        IfFailedReturn(Microsoft::WRL::MakeAndInitialize<HSTRINGIterable>(&languages, arr, numLocaleStrings));

        HSTRING geoString;
        HSTRING_HEADER geoStringHeader;
        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(L"ZZ", 2, &geoStringHeader, &geoString));

        IfFailedReturn(this->percentFormatterFactory->CreatePercentFormatter(languages.Get(), geoString, numberFormatter));

        return hr;
    }

    HRESULT WindowsGlobalizationAdapter::CreateDateTimeFormatter(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR formatString, _In_z_ PCWSTR* localeStrings,
        uint32 numLocaleStrings, _In_opt_z_ PCWSTR calendar, _In_opt_z_ PCWSTR clock, _Out_ DateTimeFormatting::IDateTimeFormatter** result)
    {
        HRESULT hr = S_OK;

        if(numLocaleStrings == 0) return E_INVALIDARG;

        Assert((calendar == nullptr && clock == nullptr) || (calendar != nullptr && clock != nullptr));

        HSTRING fsHString;
        HSTRING_HEADER fsHStringHdr;

        IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(formatString, wcslen(formatString), &fsHStringHdr, &fsHString));

        AutoArrayPtr<HSTRING> arr(HeapNewArray(HSTRING, numLocaleStrings), numLocaleStrings);
        AutoArrayPtr<HSTRING_HEADER> headers(HeapNewArray(HSTRING_HEADER, numLocaleStrings), numLocaleStrings);
        for(uint32 i = 0; i< numLocaleStrings; i++)
        {
            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(localeStrings[i],  wcslen(localeStrings[i]), (headers + i), (arr + i)));
        }

        Microsoft::WRL::ComPtr<IIterable<HSTRING>> languages(nullptr);
        IfFailedReturn(Microsoft::WRL::MakeAndInitialize<HSTRINGIterable>(&languages, arr, numLocaleStrings));

        if(clock == nullptr)
        {
            IfFailedReturn(this->dateTimeFormatterFactory->CreateDateTimeFormatterLanguages(fsHString, languages.Get(), result));
        }
        else
        {
            HSTRING geoString;
            HSTRING_HEADER geoStringHeader;
            HSTRING calString;
            HSTRING_HEADER calStringHeader;
            HSTRING clockString;
            HSTRING_HEADER clockStringHeader;

            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(L"ZZ", 2, &geoStringHeader, &geoString));
            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(calendar, wcslen(calendar), &calStringHeader, &calString));
            IfFailedReturn(GetWindowsGlobalizationLibrary(scriptContext)->WindowsCreateStringReference(clock, wcslen(clock), &clockStringHeader, &clockString));
            IfFailedReturn(this->dateTimeFormatterFactory->CreateDateTimeFormatterContext(fsHString, languages.Get(), geoString, calString, clockString, result));
        }
        return hr;
    }

    HRESULT WindowsGlobalizationAdapter::CreateTimeZoneOnCalendar(_In_ DelayLoadWindowsGlobalization *library, __out ::ITimeZoneOnCalendar**  result)
    {
        AutoCOMPtr<::ICalendar> calendar;

        HRESULT hr = S_OK;

        // initialize hard-coded default languages
        AutoArrayPtr<HSTRING> arr(HeapNewArray(HSTRING, 1), 1);
        AutoArrayPtr<HSTRING_HEADER> headers(HeapNewArray(HSTRING_HEADER, 1), 1);
        IfFailedReturn(library->WindowsCreateStringReference(L"en-US", _countof(L"en-US") - 1, (headers), (arr)));
        Microsoft::WRL::ComPtr<IIterable<HSTRING>> defaultLanguages;
        IfFailedReturn(Microsoft::WRL::MakeAndInitialize<HSTRINGIterable>(&defaultLanguages, arr, 1));

        // Create calendar object
        IfFailedReturn(this->calendarFactory->CreateCalendarDefaultCalendarAndClock(defaultLanguages.Get(), &calendar));

        // Get ITimeZoneOnCalendar part of calendar object
        IfFailedReturn(calendar->QueryInterface(__uuidof(::ITimeZoneOnCalendar), reinterpret_cast<void**>(result)));

        return hr;
    }

    HRESULT WindowsGlobalizationAdapter::CreateIncrementNumberRounder(_In_ ScriptContext* scriptContext, NumberFormatting::INumberRounder** numberRounder)
    {
        return incrementNumberRounderActivationFactory->ActivateInstance(reinterpret_cast<IInspectable**>(numberRounder));
    }

    HRESULT WindowsGlobalizationAdapter::CreateSignificantDigitsRounder(_In_ ScriptContext* scriptContext, NumberFormatting::INumberRounder** numberRounder)
    {
        return significantDigitsRounderActivationFactory->ActivateInstance(reinterpret_cast<IInspectable**>(numberRounder));
    }
#endif
}


#endif
