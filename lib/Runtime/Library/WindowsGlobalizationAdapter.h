//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

#if defined(ENABLE_INTL_OBJECT) || defined(ENABLE_ES6_CHAR_CLASSIFIER)
#include "Windows.Globalization.h"
#ifndef NTBUILD
#include "windows.globalization.numberformatting.h"
#include "windows.globalization.datetimeformatting.h"
#include "Windows.Data.Text.h"
#include "activation.h"
using namespace ABI;
#endif

class ThreadContext;

namespace Js
{
    class WindowsGlobalizationAdapter
    {
    private:
        bool initialized;
        bool failedToInitialize;
#ifdef ENABLE_INTL_OBJECT
        AutoCOMPtr<Windows::Globalization::ILanguageFactory> languageFactory;
        AutoCOMPtr<Windows::Globalization::ILanguageStatics> languageStatics;
        AutoCOMPtr<Windows::Globalization::NumberFormatting::ICurrencyFormatterFactory> currencyFormatterFactory;
        AutoCOMPtr<Windows::Globalization::NumberFormatting::IDecimalFormatterFactory> decimalFormatterFactory;
        AutoCOMPtr<Windows::Globalization::NumberFormatting::IPercentFormatterFactory> percentFormatterFactory;
        AutoCOMPtr<Windows::Globalization::DateTimeFormatting::IDateTimeFormatterFactory> dateTimeFormatterFactory;
        AutoCOMPtr<IActivationFactory> incrementNumberRounderActivationFactory;
        AutoCOMPtr<IActivationFactory> significantDigitsRounderActivationFactory;
#endif
        AutoCOMPtr<Windows::Data::Text::IUnicodeCharactersStatics> unicodeStatics;

        DelayLoadWindowsGlobalization* GetWindowsGlobalizationLibrary(_In_ ScriptContext* scriptContext);
        DelayLoadWindowsGlobalization* GetWindowsGlobalizationLibrary(_In_ ThreadContext* threadContext);
        template <typename T>
        HRESULT GetActivationFactory(DelayLoadWindowsGlobalization *library, LPCWSTR factoryName, T** instance);

    public:
        WindowsGlobalizationAdapter()
            : initialized(false),
            failedToInitialize(false),
#ifdef ENABLE_INTL_OBJECT
            languageFactory(nullptr),
            languageStatics(nullptr),
            currencyFormatterFactory(nullptr),
            decimalFormatterFactory(nullptr),
            percentFormatterFactory(nullptr),
            dateTimeFormatterFactory(nullptr),
            incrementNumberRounderActivationFactory(nullptr),
            significantDigitsRounderActivationFactory(nullptr),
#endif // ENABLE_INTL_OBJECT
            unicodeStatics(nullptr)
        { }

        HRESULT EnsureInitialized(ScriptContext *scriptContext);
        HRESULT EnsureInitialized(DelayLoadWindowsGlobalization *library, bool isES6Mode);
#ifdef ENABLE_INTL_OBJECT
        HRESULT CreateLanguage(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR languageTag, Windows::Globalization::ILanguage** language);
        boolean IsWellFormedLanguageTag(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR languageTag);
        HRESULT NormalizeLanguageTag(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR languageTag, HSTRING *result);
        HRESULT CreateCurrencyFormatterCode(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR currencyCode, Windows::Globalization::NumberFormatting::ICurrencyFormatter** currencyFormatter);
        HRESULT CreateCurrencyFormatter(_In_ ScriptContext* scriptContext, PCWSTR* localeStrings, uint32 numLocaleStrings, _In_z_ PCWSTR currencyCode, Windows::Globalization::NumberFormatting::ICurrencyFormatter** currencyFormatter);
        HRESULT CreateNumberFormatter(_In_ ScriptContext* scriptContext, PCWSTR* localeStrings, uint32 numLocaleStrings, Windows::Globalization::NumberFormatting::INumberFormatter** numberFormatter);
        HRESULT CreatePercentFormatter(_In_ ScriptContext* scriptContext, PCWSTR* localeStrings, uint32 numLocaleStrings, Windows::Globalization::NumberFormatting::INumberFormatter** numberFormatter);
        HRESULT CreateDateTimeFormatter(_In_ ScriptContext* scriptContext, _In_z_ PCWSTR formatString, _In_z_ PCWSTR* localeStrings, uint32 numLocaleStrings, 
            _In_opt_z_ PCWSTR calendar, _In_opt_z_ PCWSTR clock, _Out_ Windows::Globalization::DateTimeFormatting::IDateTimeFormatter** formatter);
        HRESULT CreateIncrementNumberRounder(_In_ ScriptContext* scriptContext, Windows::Globalization::NumberFormatting::INumberRounder** numberRounder);
        HRESULT CreateSignificantDigitsRounder(_In_ ScriptContext* scriptContext, Windows::Globalization::NumberFormatting::INumberRounder** numberRounder);
#endif // ENABLE_INTL_OBJECT
        Windows::Data::Text::IUnicodeCharactersStatics* GetUnicodeStatics()
        {
            return unicodeStatics;
        }
    };
}

#endif