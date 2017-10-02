// -*- C++ -*-

#pragma once

#include <iostream>
#include <cmath>

#define RES(x) double res = x; assert(res == res); return res;
#define RRES(x) x; assert(d == d); return *this;

struct dbl
{
        double d;
        typedef double Data;

        dbl(double other = 0)
        {
                assert(other == other);
                d = other;
        }

#define COP(OP) dbl operator OP (dbl other) const { RES(d OP other.d); } \
        dbl operator OP (double other) const {RES(d OP other); }

        COP(+)
        COP(-)
        COP(*)
        COP(/)

        dbl& operator -= (dbl other)
        {
                RRES(d -= other.d);
        }

        dbl& operator += (dbl other)
        {
                RRES(d += other.d);
        }

        dbl& operator /= (dbl other)
        {
                RRES(d /= other.d);
        }

        dbl& operator *= (dbl other)
        {
                RRES(d *= other.d);
        }

#define CMPOP_OTHER(OP, T) bool operator OP (T other) const \
        { return d OP other; }
#define CMPOP(OP) bool operator OP (dbl other) const { return d OP other.d; } \
        CMPOP_OTHER(OP, int) \
        CMPOP_OTHER(OP, unsigned) \
        CMPOP_OTHER(OP, double)



        CMPOP(==)
        CMPOP(<)
        CMPOP(>)
        CMPOP(<=)
        CMPOP(>=)
        CMPOP(!=)

        operator bool () const
        {
                return d;
        }
};

#define EOP(OP) inline dbl operator OP (double d, dbl db)   \
        {                                               \
                return dbl(d) OP db;                    \
        }                                               \

EOP(-);
EOP(+);
EOP(*);
EOP(/);

inline std::istream& operator >> (std::istream& is, dbl& d)
{
        is >> d.d;
        assert(d.d == d.d);
        return is;
}

inline std::ostream& operator << (std::ostream& is, dbl d)
{
        return is << d.d;
}

inline dbl fmod(dbl a, dbl b)
{
        return std::fmod(a.d, b.d);
}

//#define double dbl
