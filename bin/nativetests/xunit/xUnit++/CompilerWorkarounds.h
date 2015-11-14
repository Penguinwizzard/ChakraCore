#ifndef COMPILERWORKAROUNDS_H_
#define COMPILERWORKAROUNDS_H_

// Workarounds for noncompliant with C++11 compilers.
#if defined(_MSC_VER) && (_MSC_VER < 1900) // For MSVC below v14.0 (before MS Visual Studio 2015).
# define XU_DEFAULT {}
# define XU_NOEXCEPT(x)
#else
# define XU_CPP11_COMPLIANT_COMPILER
# define XU_DEFAULT = default;
# define XU_NOEXCEPT(x) noexcept(x)
#endif

#endif // COMPILERWORKAROUNDS_H_
