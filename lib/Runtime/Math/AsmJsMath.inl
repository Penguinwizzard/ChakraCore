//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
namespace Js
{
    template<typename T>
    __inline T AsmJsMath::Min(T aLeft, T aRight)
    {
        return aLeft < aRight ? aLeft : aRight;
    }

    template<>
    __inline double AsmJsMath::Min<double>(double aLeft, double aRight)
    {
        if (NumberUtilities::IsNan(aLeft) || NumberUtilities::IsNan(aRight))
        {
            return NumberConstants::NaN;
        }
        return aLeft < aRight ? aLeft : aRight;
    }

    template<typename T>
    __inline T AsmJsMath::Max(T aLeft, T aRight)
    {
        return aLeft > aRight ? aLeft : aRight;
    }

    template<>
    __inline double AsmJsMath::Max<double>(double aLeft, double aRight)
    {
        if (NumberUtilities::IsNan(aLeft) || NumberUtilities::IsNan(aRight))
        {
            return NumberConstants::NaN;
        }
        return aLeft > aRight ? aLeft : aRight;
    }

    template<typename T>
    __inline T AsmJsMath::Add( T aLeft, T aRight )
    {
        return aLeft + aRight;
    }

    template<>
    __inline int32 AsmJsMath::Div<int32>( int32 aLeft, int32 aRight )
    {
        return aRight == 0 ? 0 : ( aLeft == (1<<31) && aRight == -1) ? aLeft : aLeft / aRight;
    }

    template<>
    __inline uint32 AsmJsMath::Div<uint32>(uint32 aLeft, uint32 aRight)
    {
        return aRight == 0 ? 0 : aLeft / aRight;
    }

    template<typename T>
    __inline T AsmJsMath::Div( T aLeft, T aRight )
    {
        return aLeft / aRight;
    }

    template<typename T>
    __inline T AsmJsMath::Mul( T aLeft, T aRight )
    {
        return aLeft * aRight;
    }

    template<typename T>
    __inline T AsmJsMath::Sub( T aLeft, T aRight )
    {
        return aLeft - aRight;
    }

    template<typename T> __inline int AsmJsMath::CmpLt( T aLeft, T aRight ){return (int)(aLeft <  aRight);}
    template<typename T> __inline int AsmJsMath::CmpLe( T aLeft, T aRight ){return (int)(aLeft <= aRight);}
    template<typename T> __inline int AsmJsMath::CmpGt( T aLeft, T aRight ){return (int)(aLeft >  aRight);}
    template<typename T> __inline int AsmJsMath::CmpGe( T aLeft, T aRight ){return (int)(aLeft >= aRight);}
    template<typename T> __inline int AsmJsMath::CmpEq( T aLeft, T aRight ){return (int)(aLeft == aRight);}
    template<typename T> __inline int AsmJsMath::CmpNe( T aLeft, T aRight ){return (int)(aLeft != aRight);}

    template<typename T>
    __inline T AsmJsMath::Rem( T aLeft, T aRight )
    {
        return (aRight == 0) ? 0 : aLeft % aRight;
    }

    template<>
    __inline int AsmJsMath::Rem<int>( int aLeft, int aRight )
    {
        return ((aRight == 0) || (aLeft == (1<<31) && aRight == -1)) ? 0 : aLeft % aRight;
    }

    template<>
    __inline double AsmJsMath::Rem<double>( double aLeft, double aRight )
    {
        return NumberUtilities::Modulus( aLeft, aRight );
    }

    __inline int AsmJsMath::And( int aLeft, int aRight )
    {
        return aLeft & aRight;
    }

    __inline int AsmJsMath::Or( int aLeft, int aRight )
    {
        return aLeft | aRight;
    }

    __inline int AsmJsMath::Xor( int aLeft, int aRight )
    {
        return aLeft ^ aRight;
    }

    __inline int AsmJsMath::Shl( int aLeft, int aRight )
    {
        return aLeft << aRight;
    }

    __inline int AsmJsMath::Shr( int aLeft, int aRight )
    {
        return aLeft >> aRight;
    }

    __inline int AsmJsMath::ShrU( int aLeft, int aRight )
    {
        return (unsigned int)aLeft >> (unsigned int)aRight;
    }

    __inline int AsmJsMath::Rol( int aLeft, int aRight )
    {
        return _rotl(aLeft, aRight);
    }

    __inline int AsmJsMath::Ror( int aLeft, int aRight )
    {
        return _rotr(aLeft, aRight);
    }

    template<typename T>
    __inline T AsmJsMath::Neg( T aLeft )
    {
        return -aLeft;
    }

    __inline int AsmJsMath::Not( int aLeft )
    {
        return ~aLeft;
    }

    __inline int AsmJsMath::LogNot( int aLeft )
    {
        return !aLeft;
    }

    __inline int AsmJsMath::ToBool( int aLeft )
    {
        return !!aLeft;
    }

    inline int AsmJsMath::Clz32( int value)
    {
        DWORD index;
        if (_BitScanReverse(&index, value))
        {
            return 31 - index;
        }
        return 32;
    }

    inline int AsmJsMath::Eqz(int value)
    {
        return value == 0;
    }

    template<typename T>
    inline T AsmJsMath::Copysign(T aLeft, T aRight)
    {
        return copysign(aLeft, aRight);
    }

    template<typename T>
    inline T AsmJsMath::Trunc(T value)
    {
        if (value == 0.0)
        {
            return value;
        }
        else
        {
            T result;
            if (value < 0.0)
            {
                result = ceil(value);
            }
            else
            {
                result = floor(value);
            }
            if (NumberUtilities::IsNan(result))
            {
                return (T)(((uint)value | (sizeof(T)*8)) * NumberConstants::NaN);  // XXX
            }
            return result;
        }
    }

    template<typename T>
    inline T AsmJsMath::Nearest(T value)
    {
        if (value == 0.0)
        {
            return value;
        }
        else
        {
            T result;
            T u = ceil(value);
            T d = floor(value);
            T um = fabs(value - u);
            T dm = fabs(value - d);
            if (um < dm || (um == dm && floor(u / 2) == u / 2))
            {
                result = u;
            }
            else
            {
                result = d;
            }
            if (NumberUtilities::IsNan(result))
            {
                return (T)(((uint)value | (sizeof(T) * 8)) * NumberConstants::NaN); // XXX
            }
            return result;
        }
    }
}
