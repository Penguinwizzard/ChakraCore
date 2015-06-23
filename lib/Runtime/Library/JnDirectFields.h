//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

//
// NOTE: This file is intended to be "#include" multiple times.  The call site should define various
// macros to be executed for each entry.  Macros that are not provided will be given a default,
// empty implementation.
//

#if !defined(ENTRY_INTERNAL_SYMBOL)
#define ENTRY_INTERNAL_SYMBOL
#endif
#if !defined(ENTRY_SYMBOL)
#define ENTRY_SYMBOL(name, description)
#endif
#if !defined(ENTRY)
#define ENTRY(name)
#endif
#if !defined(ENTRY2)
#define ENTRY2(name, str)
#endif
ENTRY_INTERNAL_SYMBOL(_lexicalThisSlotSymbol)
ENTRY_INTERNAL_SYMBOL(_superReferenceSymbol)
ENTRY_SYMBOL(_symbolHasInstance, L"Symbol.hasInstance")
ENTRY_SYMBOL(_symbolIsConcatSpreadable, L"Symbol.isConcatSpreadable")
ENTRY_SYMBOL(_symbolIterator, L"Symbol.iterator")
ENTRY_SYMBOL(_symbolSpecies, L"[Symbol.species]")
ENTRY_SYMBOL(_symbolToPrimitive, L"Symbol.toPrimitive")
ENTRY_SYMBOL(_symbolToStringTag, L"Symbol.toStringTag")
ENTRY_SYMBOL(_symbolUnscopables, L"Symbol.unscopables")

// math functions must remain contiguous for quick modification check
ENTRY(abs)
ENTRY(acos)
ENTRY(asin)
ENTRY(atan)
ENTRY(atan2)
ENTRY(ceil)
ENTRY(cos)
ENTRY(exp)
ENTRY(floor)
ENTRY(log)
ENTRY(LOG10E)
ENTRY(LOG2E)
ENTRY(min)
ENTRY(PI)
ENTRY(pow)
ENTRY(random)
ENTRY(round)
ENTRY(sin)
ENTRY(sqrt)
ENTRY(tan)
ENTRY(log10)
ENTRY(log2)
ENTRY(log1p)
ENTRY(expm1)
ENTRY(cosh)
ENTRY(sinh)
ENTRY(tanh)
ENTRY(acosh)
ENTRY(asinh)
ENTRY(atanh)
ENTRY(hypot)
ENTRY(trunc)
ENTRY(sign)
ENTRY(cbrt)
ENTRY(imul)
ENTRY(clz32)
ENTRY(fround)
// End math functions

#ifdef SIMD_JS_ENABLED
// SIMD 
ENTRY(signMask)
ENTRY(float32x4)
ENTRY(float64x2)
ENTRY(int32x4)
ENTRY(zero)
ENTRY(splat)
ENTRY2(bool_, L"bool") // "bool" cannot be an identifier in C++ so using "bool_" instead

ENTRY(fromFloat32x4)
ENTRY(fromFloat32x4Bits)
ENTRY(fromInt32x4)
ENTRY(fromInt32x4Bits)
ENTRY(fromFloat64x2)
ENTRY(fromFloat64x2Bits)

// keep these contiguous
ENTRY(withX)
ENTRY(withY)
ENTRY(withZ)
ENTRY(withW)
ENTRY(x)
ENTRY(y)
ENTRY(z)
ENTRY(w)
ENTRY(withFlagX)
ENTRY(withFlagY)
ENTRY(withFlagZ)
ENTRY(withFlagW)
ENTRY(flagX)
ENTRY(flagY)
ENTRY(flagZ)
ENTRY(flagW)
//
ENTRY(mul)
ENTRY(div)
ENTRY(and)
ENTRY(or)
ENTRY(xor)
ENTRY(neg)
ENTRY(not)
ENTRY(shuffle)
ENTRY(shuffleMix)
ENTRY(clamp)
ENTRY(select)
ENTRY(reciprocal)
ENTRY(reciprocalSqrt)
ENTRY(scale)
ENTRY(lessThan)
ENTRY(lessThanOrEqual)
ENTRY(equal)
ENTRY(notEqual)
ENTRY(greaterThanOrEqual)
ENTRY(greaterThan)

ENTRY(shiftLeft)
ENTRY(shiftRightLogical)
ENTRY(shiftRightArithmetic)

// Shuffle Masks
#define MACRO(maskName, maskValue) ENTRY(maskName)
#include "SIMDShuffleMasks.h"
// End SIMD
#endif

ENTRY(add)
ENTRY(all)
ENTRY(anchor)
ENTRY(apply)
ENTRY(Array)
ENTRY(assign)
ENTRY(atEnd)
ENTRY(big)
ENTRY(bind)
ENTRY(blink)
ENTRY(bold)
ENTRY(Boolean)
ENTRY(call)
ENTRY(CanvasPixelArray)
ENTRY(cast)
ENTRY2(catch_, L"catch") // "catch" cannot be an identifier in C++ so using "catch_" instead
ENTRY(charAt)
ENTRY(charCodeAt)
ENTRY(codePointAt)
ENTRY(clear)
ENTRY(copyWithin)
ENTRY(CollectGarbage)
ENTRY(concat)
ENTRY(construct)
ENTRY(constructor)
ENTRY(ConversionError)
ENTRY(create)
ENTRY(Date)
ENTRY(decodeURI)
ENTRY(decodeURIComponent)
ENTRY2(delete_, L"delete") // "delete" cannot be an identifier in C++ so using "delete_" instead
ENTRY(deleteProperty)
ENTRY(description)
ENTRY(done)
ENTRY(E)
ENTRY(encodeURI)
ENTRY(encodeURIComponent)
ENTRY(endsWith)
ENTRY(entries)
ENTRY(enumerate)
ENTRY(Enumerator)
ENTRY(EPSILON)
ENTRY(escape)
ENTRY(Error)
ENTRY(eval)
ENTRY(EvalError)
ENTRY(every)
ENTRY(exec)
ENTRY(flags)
ENTRY(fill)
ENTRY(filter)
ENTRY(find)
ENTRY(findIndex)
ENTRY(fixed)
ENTRY(fontcolor)
ENTRY(fontsize)
ENTRY2(for_, L"for") // "for" cannot be an identifier in C++ so using "for_" instead
ENTRY(forEach)
ENTRY(freeze)
ENTRY(from)
ENTRY(fromCharCode)
ENTRY(fromCodePoint)
ENTRY(Function)
ENTRY(getDate)
ENTRY(getDay)
ENTRY(getFullYear)
ENTRY(getHours)
ENTRY(getMilliseconds)
ENTRY(getMinutes)
ENTRY(getMonth)
ENTRY(GetObject)
ENTRY(getOwnPropertyDescriptor)
ENTRY(getOwnPropertyNames)
ENTRY(getOwnPropertySymbols)
ENTRY(getSeconds)
ENTRY(getTime)
ENTRY(getTimezoneOffset)
ENTRY(getUTCDate)
ENTRY(getUTCDay)
ENTRY(getUTCFullYear)
ENTRY(getUTCHours)
ENTRY(getUTCMilliseconds)
ENTRY(getUTCMinutes)
ENTRY(getUTCMonth)
ENTRY(getUTCSeconds)
ENTRY(getVarDate)
ENTRY(getYear)
ENTRY(has)
ENTRY(hasInstance)
ENTRY(hasOwnProperty)
ENTRY(includes)
ENTRY(indexOf)
ENTRY(Infinity)
ENTRY(Intl)
ENTRY(is)
ENTRY(isArray)
ENTRY(isConcatSpreadable)
ENTRY(isExtensible)
ENTRY(isFinite)
ENTRY(isFrozen)
ENTRY(isInteger)
ENTRY(isNaN)
ENTRY(isPrototypeOf)
ENTRY(isSafeInteger)
ENTRY(isSealed)
ENTRY(isView)
ENTRY(italics)
ENTRY(item)
ENTRY(iterator)
ENTRY(join)
ENTRY(JSON)
ENTRY(keyFor)
ENTRY(keys)
ENTRY(lastIndexOf)
ENTRY(length)
ENTRY(link)
ENTRY(LN10)
ENTRY(LN2)
ENTRY(localeCompare)
ENTRY(map)
ENTRY(Map)
ENTRY(Math)

// SIMD
#ifdef SIMD_JS_ENABLED
ENTRY(SIMD)
#endif

ENTRY(max)
ENTRY(MAX_SAFE_INTEGER)
ENTRY(MAX_VALUE)
ENTRY(match)
ENTRY(message)
ENTRY(MIN_SAFE_INTEGER)
ENTRY(MIN_VALUE)
ENTRY(moveFirst)
ENTRY(moveNext)
ENTRY(NaN)
ENTRY(name)
ENTRY(next)
ENTRY(normalize)
ENTRY(now)
ENTRY(Number)
ENTRY(number)
ENTRY(NEGATIVE_INFINITY)
ENTRY(ownKeys)
ENTRY(Object)
ENTRY(of)
ENTRY(parse)
ENTRY(parseFloat)
ENTRY(parseInt)
ENTRY(pop)
ENTRY(POSITIVE_INFINITY)
ENTRY(preventExtensions)
ENTRY(propertyIsEnumerable)
ENTRY(prototype)
ENTRY(proxy)
ENTRY(Proxy)
ENTRY(push)
ENTRY(race)
ENTRY(RangeError)
ENTRY(raw)
ENTRY(reduce)
ENTRY(reduceRight)
ENTRY(ReferenceError)
ENTRY(Reflect)
ENTRY(RegExpError)
ENTRY(reject)
ENTRY(replace)
ENTRY(resolve)
ENTRY2(return_, L"return") // "return" cannot be an identifier in C++ so using "return_" instead
ENTRY(reverse)
ENTRY(revocable)
ENTRY(revoke)
ENTRY(RegExp)
ENTRY(repeat)
ENTRY(seal)
ENTRY(search)
ENTRY(Set)
ENTRY(setDate)
ENTRY(setFullYear)
ENTRY(setHours)
ENTRY(setMilliseconds)
ENTRY(setMinutes)
ENTRY(setMonth)
ENTRY(setSeconds)
ENTRY(setTime)
ENTRY(setUTCDate)
ENTRY(setUTCFullYear)
ENTRY(setUTCHours)
ENTRY(setUTCMilliseconds)
ENTRY(setUTCMinutes)
ENTRY(setUTCMonth)
ENTRY(setUTCSeconds)
ENTRY(setYear)
ENTRY(shift)
ENTRY(size)
ENTRY(slice)
ENTRY(some)
ENTRY(sort)
ENTRY(source)
ENTRY(species)
ENTRY(splice)
ENTRY(split)
ENTRY(SQRT1_2)
ENTRY(SQRT2)
ENTRY(strike)
ENTRY(String)
ENTRY(sub)
ENTRY(substring)
ENTRY(substr)
ENTRY(sup)
ENTRY(stack)
ENTRY(stackTraceLimit)
ENTRY(startsWith)
ENTRY(stringify)
ENTRY(Symbol)
ENTRY(SyntaxError)
ENTRY(test)
ENTRY(then)
ENTRY2(throw_, L"throw") // "throw" cannot be an identifier in C++ so using "throw_" instead
ENTRY(toDateString)
ENTRY(toExponential)
ENTRY(toFixed)
ENTRY(toISOString)
ENTRY(toJSON)
ENTRY(toLocaleDateString)
ENTRY(toLocaleLowerCase)
ENTRY(toLocaleString)
ENTRY(toLocaleTimeString)
ENTRY(toLocaleUpperCase)
ENTRY(toLowerCase)
ENTRY(toMethod)
ENTRY(toPrecision)
ENTRY(toPrimitive)
ENTRY(toString)
ENTRY(toStringTag)
ENTRY(toTimeString)
ENTRY(toUpperCase)
ENTRY(toUTCString)
ENTRY(transfer)
ENTRY(trim)
ENTRY(trimLeft)
ENTRY(trimRight)
ENTRY(TypeError)
ENTRY(undefined)
ENTRY(unescape)
ENTRY(unscopables)
ENTRY(unshift)
ENTRY(URIError)
ENTRY(UTC)
ENTRY(valueOf)
ENTRY(values)
ENTRY(WeakMap)
ENTRY(WeakSet)
ENTRY(WinRTError)
ENTRY(write)
ENTRY(writeln)
ENTRY(setNonUserCodeExceptions)
ENTRY(debuggerEnabled)
ENTRY(enumerable)
ENTRY(configurable)
ENTRY(writable)
ENTRY(value)
ENTRY(get)
ENTRY(getPrototypeOf)
ENTRY(setPrototypeOf)
ENTRY(set)
ENTRY(defineProperty)
ENTRY(defineProperties)
ENTRY(toGMTString)
ENTRY(compile)
ENTRY(global)
ENTRY(lastIndex)
ENTRY(multiline)
ENTRY(ignoreCase)
ENTRY(unicode)
ENTRY(sticky)
ENTRY(ScriptEngine)
ENTRY(ScriptEngineMajorVersion)
ENTRY(ScriptEngineMinorVersion)
ENTRY(ScriptEngineBuildVersion)
ENTRY(byteOffset)
ENTRY(byteLength)
ENTRY(buffer)
ENTRY(BYTES_PER_ELEMENT)
ENTRY(ArrayBuffer)
ENTRY(Int8Array)
ENTRY(Int16Array)
ENTRY(Int32Array)
ENTRY(Uint8Array)
ENTRY(Uint8ClampedArray)
ENTRY(Uint16Array)
ENTRY(Uint32Array)
ENTRY(Float32Array)
ENTRY(Float64Array)
ENTRY(CharArray)
ENTRY(Int64Array)
ENTRY(Uint64Array)
ENTRY(BooleanArray)
ENTRY(DataView)
ENTRY(setInt8)
ENTRY(setUint8)
ENTRY(setInt16)
ENTRY(setUint16)
ENTRY(setInt32)
ENTRY(setUint32)
ENTRY(setFloat32)
ENTRY(setFloat64)
ENTRY(getInt8)
ENTRY(getUint8)
ENTRY(getInt16)
ENTRY(getUint16)
ENTRY(getInt32)
ENTRY(getUint32)
ENTRY(getFloat32)
ENTRY(getFloat64)
ENTRY(subarray)
ENTRY(Debug)
ENTRY(caller)
ENTRY(callee)
ENTRY(arguments)
ENTRY($_)
ENTRY($1)
ENTRY($2)
ENTRY($3)
ENTRY($4)
ENTRY($5)
ENTRY($6)
ENTRY($7)
ENTRY($8)
ENTRY($9)
ENTRY(index)
ENTRY(input)
ENTRY(lastMatch)
ENTRY(lastParen)
ENTRY(leftContext)
ENTRY(rightContext)
ENTRY(options)
ENTRY2(__instance, L"$__instance")
ENTRY2(Small, L"small")
ENTRY2($Ampersand, L"$&")
ENTRY2($Plus, L"$+")
ENTRY2($BackTick, L"$`")
ENTRY2($Tick, L"$\'")
ENTRY(__defineGetter__)
ENTRY(__defineSetter__)
ENTRY(__lookupGetter__)
ENTRY(__lookupSetter__)
ENTRY(__proto__)
//Intl
ENTRY(EngineInterface)
ENTRY(raiseAssert)
ENTRY(__minimumFractionDigits)
ENTRY(__maximumFractionDigits)
ENTRY(__minimumIntegerDigits)
ENTRY(__minimumSignificantDigits)
ENTRY(__maximumSignificantDigits)
ENTRY(minimumSignificantDigits)
ENTRY(maximumSignificantDigits)
ENTRY(__isDecimalPointAlwaysDisplayed)
ENTRY(__useGrouping)
ENTRY(__numberingSystem)
ENTRY(__formatterToUse)
ENTRY(__currency)
ENTRY(__currencyDisplay)
ENTRY(__currencyDisplayToUse)
ENTRY(__locale)
ENTRY(__templateString)
ENTRY(Collator)
ENTRY(NumberFormat)
ENTRY(DateTimeFormat)
ENTRY(format)
ENTRY(resolvedOptions)
ENTRY(compare)
ENTRY(locale)
ENTRY(sensitivity)
ENTRY(numeric)
ENTRY(ignorePunctuation)
ENTRY(__windowsClock)
ENTRY(__windowsCalendar)
ENTRY(__patternStrings)
ENTRY(isWellFormedLanguageTag)
ENTRY(normalizeLanguageTag)
ENTRY(compareString)
ENTRY(resolveLocaleLookup)
ENTRY(resolveLocaleBestFit)
ENTRY(getDefaultLocale)
ENTRY(getExtensions)
ENTRY(formatNumber)
ENTRY(cacheNumberFormat)
ENTRY(createDateTimeFormat)
ENTRY(currencyDigits)
ENTRY(formatDateTime)
ENTRY(registerBuiltInFunction)
ENTRY(getHiddenObject)
ENTRY(setHiddenObject)
ENTRY(builtInSetPrototype)
ENTRY(builtInGetArrayLength)
ENTRY(builtInRegexMatch)
ENTRY(builtInCallInstanceFunction)
ENTRY(subTags)
ENTRY(builtInMathAbs)
ENTRY(builtInMathFloor)
ENTRY(builtInMathMax)
ENTRY(builtInMathPow)
ENTRY(builtInJavascriptObjectEntryDefineProperty)
ENTRY(builtInJavascriptObjectEntryGetPrototypeOf)
ENTRY(builtInJavascriptObjectEntryIsExtensible)
ENTRY(builtInJavascriptObjectEntryGetOwnPropertyNames)
ENTRY(builtInJavascriptObjectEntryHasOwnProperty)
ENTRY(builtInJavascriptObjectCreate)
ENTRY(builtInJavascriptObjectPreventExtensions)
ENTRY(builtInJavascriptObjectGetOwnPropertyDescriptor)
ENTRY(builtInJavascriptArrayEntryForEach)
ENTRY(builtInJavascriptArrayEntryIndexOf)
ENTRY(builtInJavascriptArrayEntryPush)
ENTRY(builtInJavascriptArrayEntryJoin)
ENTRY(builtInJavascriptFunctionEntryBind)
ENTRY(builtInJavascriptFunctionEntryToMethod)
ENTRY(builtInJavascriptDateEntryGetDate)
ENTRY(builtInJavascriptDateEntryNow)
ENTRY(builtInJavascriptRegExpEntryTest)
ENTRY(builtInJavascriptStringEntryMatch)
ENTRY(builtInJavascriptStringEntryReplace)
ENTRY(builtInJavascriptStringEntryToLowerCase)
ENTRY(builtInJavascriptStringEntryToUpperCase)
ENTRY(builtInGlobalObjectEntryIsFinite)
ENTRY(builtInGlobalObjectEntryIsNaN)
ENTRY(builtInGlobalObjectEval)
ENTRY(raiseNeedObject)
ENTRY(raiseNeedObjectOrString)
ENTRY(raiseThis_NullOrUndefined)
ENTRY(raiseNotAConstructor)
ENTRY(raiseInvalidDate)
ENTRY(ToLogicalBoolean)
ENTRY(ToString)
ENTRY(ToNumber)
ENTRY(raiseOptionValueOutOfRange_3)
ENTRY(raiseOptionValueOutOfRange)
ENTRY(ToUint32)
ENTRY(raiseLocaleNotWellFormed)
ENTRY(isValid)
ENTRY(localeMatcher)
ENTRY(raiseObjectIsAlreadyInitialized)
ENTRY(raiseObjectIsNonExtensible)
ENTRY(raiseNeedObjectOfType)
ENTRY(raiseInvalidCurrencyCode)
ENTRY(raiseMissingCurrencyCode)
ENTRY(__initializedIntlObject)
ENTRY(__initializedNumberFormat)
ENTRY(__initializedDateTimeFormat)
ENTRY(localeWithoutSubtags)
ENTRY(__matcher)
ENTRY(__localeForCompare)
ENTRY(__usage)
ENTRY(__sensitivity)
ENTRY(__ignorePunctuation)
ENTRY(__caseFirst)
ENTRY(__numeric)
ENTRY(__collation)
ENTRY(__initializedCollator)
ENTRY(__boundCompare)
ENTRY(__boundFormat)
ENTRY(supportedLocalesOf)
ENTRY(__style)
ENTRY(weekday)
ENTRY(year)
ENTRY(month)
ENTRY(day)
ENTRY(minute)
ENTRY(hour)
ENTRY(second)
ENTRY(era)
ENTRY(timeZone)
ENTRY(timeZoneName)
ENTRY(hour12)
ENTRY(__weekday)
ENTRY(__year)
ENTRY(__month)
ENTRY(__day)
ENTRY(__minute)
ENTRY(__hour)
ENTRY(__second)
ENTRY(__era)
ENTRY(__timeZone)
ENTRY(__timeZoneName)
ENTRY(__hour12)
ENTRY(__formatMatcher)
ENTRY(__calendar)
ENTRY(calendar)
ENTRY(__relevantExtensionKeys)
ENTRY(numberingSystem)
ENTRY(MS_ASYNC_OP_STATUS_SUCCESS)
ENTRY(MS_ASYNC_OP_STATUS_CANCELED)
ENTRY(MS_ASYNC_OP_STATUS_ERROR)
ENTRY(MS_ASYNC_CALLBACK_STATUS_ASSIGN_DELEGATE)
ENTRY(MS_ASYNC_CALLBACK_STATUS_JOIN)
ENTRY(MS_ASYNC_CALLBACK_STATUS_CHOOSEANY)
ENTRY(MS_ASYNC_CALLBACK_STATUS_CANCEL)
ENTRY(MS_ASYNC_CALLBACK_STATUS_ERROR)
ENTRY(msTraceAsyncOperationStarting)
ENTRY(msTraceAsyncCallbackStarting)
ENTRY(msTraceAsyncCallbackCompleted)
ENTRY(msUpdateAsyncCallbackRelation)
ENTRY(msTraceAsyncOperationCompleted)
ENTRY(getErrorMessage)
ENTRY(logDebugMessage)
ENTRY(tagPublicLibraryCode)
ENTRY(enqueueTask)
ENTRY(Common)
ENTRY(Promise)
ENTRY(MissingProperty)

// These are ES5/6/7+ builtins that are tracked for telemetry purposes, but currently not implemented by Chakra or are otherwise special.
#ifdef TELEMETRY_ESB
ENTRY(observe)
ENTRY(unobserve)
ENTRY(setUInt16)
ENTRY(getOwnPropertyDescriptors)
ENTRY(sticky)
ENTRY(unicode)
ENTRY(__constructor) // represents an invocation of the constructor function rather than a use of the constructor property (i.e. `new Foo()` rather than `(new Foo()).constructor`).
#endif

// SymbolFunctionNameId: for RuntimeFunction nameId
ENTRY2(_RuntimeFunctionNameId_toStringTag, L"[Symbol.toStringTag]")
ENTRY2(_RuntimeFunctionNameId_toPrimitive, L"[Symbol.toPrimitive]")
ENTRY2(_RuntimeFunctionNameId_iterator, L"[Symbol.iterator]")
ENTRY2(_RuntimeFunctionNameId_species, L"[Symbol.species]")

//Previously we would add these right after standard built-ins were generated; now adding them here.
ENTRY(document)
ENTRY2(_event, L"event")
ENTRY(history)
ENTRY(location)
ENTRY(navigator)
ENTRY(screen)


// Note: Do not add fields for conditionally-compiled PropertyIds into this file.
//       The bytecode for internal javascript libraries is built on chk but re-used in fre builds.
//       Having a mismatch in the number of PropertyIds will cause a failure loading bytecode.
//       See BLUE #467459

#undef ENTRY_INTERNAL_SYMBOL
#undef ENTRY_SYMBOL
#undef ENTRY
#undef ENTRY2
