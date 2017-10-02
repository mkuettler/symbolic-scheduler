// -*- C++ -*-

#pragma once

#include <map>
#include <vector>
#include <algorithm>
#include <istream>
#include <cassert>

#include <iostream>

#include "dbl.h"

#define VP(v, p) std::make_pair(v, p)

class Time
{
private:
        //std::map<int, double> values;
        typedef std::vector<std::pair<int, double> > ValueType;
        ValueType values;
        double _sum;

        static void sort_and_reduce(ValueType & values) {
                std::sort(values.begin(), values.end(),
                          [](auto a, auto b) { return a.first < b.first; });
                for (auto it = values.begin(); it != values.end(); ++it) {
                        auto it2 = it + 1;
                        for (; it2 != values.end() && it2->first == it->first;
                             ++it2) {
                                it->second += it2->second;
                        }
                        values.erase(it+1,it2);
                }
        }

public:
        Time() : _sum(0)
        {}

        Time(int val) : _sum(1)
        { values.push_back(VP(val, 1.0)); }

        Time(int val, double prob) : _sum(prob)
        { values.push_back(VP(val, prob)); }

        Time(ValueType const & vals)
                : values(vals), _sum(0)
        {
                for (auto it = vals.begin(); it != vals.end(); ++it) {
                        _sum += it->second;
                }
        }

        Time(std::map<int, double> const & vals) : values(), _sum(0)
        {
                values.reserve(vals.size());
                for (auto it = vals.begin(); it != vals.end(); ++it) {
                        values.push_back(*it);
                        _sum += it->second;
                }
        }

        int choose() const;

        Time merge(Time const & other) const
        {
                Time res;
                res.values.reserve(values.size() + other.values.size());
                auto itt = values.begin();
                auto ito = other.values.begin();
                while(itt != values.end() && ito != other.values.end()) {
                        if (itt->first < ito->first) {
                                res.values.push_back(*itt);
                                ++itt;
                        } else if (ito->first < itt->first) {
                                res.values.push_back(*ito);
                                ++ito;
                        } else {
                                res.values.push_back
                                        (VP(itt->first,
                                            itt->second+ito->second));
                                ++itt;
                                ++ito;
                        }
                }
                for (; itt != values.end(); ++itt) {
                        res.values.push_back(*itt);
                }
                for (; ito != other.values.end(); ++ito) {
                        res.values.push_back(*ito);
                }
                res._sum = _sum + other._sum;
                return res;
        }

        Time operator + (Time const & other) const
        {
                Time res;
                res.values.reserve(values.size() * other.values.size());
                for (auto i1 = values.begin(); i1 != values.end(); ++i1) {
                        for (auto i2 = other.values.begin();
                             i2 != other.values.end(); ++i2) {
                                res.values.push_back
                                        (VP(i1->first + i2->first,
                                            i1->second * i2->second));
                        }
                }
                sort_and_reduce(res.values);
                res._sum = _sum * other._sum;
                //res.tidy();
                return res;
        }

        Time operator + (int val) const
        {
                Time res;
                res.values.reserve(values.size());
                for (auto i1 = values.begin(); i1 != values.end(); ++i1) {
                        res.values.push_back(VP(i1->first + val, i1->second));
                }
                res._sum = _sum;
                return res;
        }

        Time& operator += (Time const & other)
        {
                (*this) = (*this) + other;
                return *this;
        }

        Time& operator += (int other)
        {
                for (auto i1 = values.begin(); i1 != values.end(); ++i1) {
                        i1->first += other;
                }
                return *this;
        }

        Time& operator -= (Time const & other)
        {
                (*this) = (*this) - other;
                return *this;
        }

        Time& operator -= (int other)
        {
                return (*this) += -other;
        }

        Time operator - (int other) const
        {
                return (*this) + (-other);
        }

        Time operator - (Time const & other) const
        {
                Time res;
                res.values.reserve(values.size() * other.values.size());
                for (auto i1 = values.begin(); i1 != values.end(); ++i1) {
                        for (auto i2 = other.values.begin();
                             i2 != other.values.end(); ++i2) {
                                res.values.push_back
                                        (VP(i1->first - i2->first,
                                            i1->second * i2->second));
                        }
                }
                sort_and_reduce(res.values);
                res._sum = _sum * other._sum;
                //res.tidy();
                return res;
        }


        Time operator << (int val) const
        {
                Time res;

                auto rit = values.rbegin();
                double s = 0;
                for (; rit != values.rend() && rit->first >= val; ++rit) {
                        s += rit->second;
                }

                auto it = rit.base();

                res.values.reserve(it-values.begin() + 1);
                res.values.insert(res.values.begin(), values.begin(), it);
                res.values.push_back(VP(val, s));

                res._sum = _sum;
                return res;
        }

        Time operator >> (int val) const
        {
                Time res;

                auto it = values.begin();
                double s = 0;
                for (; it != values.end() && it->first <= val; ++it) {
                        s += it->second;
                }

                res.values.reserve(values.end() - it + 1);
                res.values.push_back(VP(val, s));
                res.values.insert(res.values.end(), it, values.end());

                res._sum = _sum;
                return res;
        }

        Time operator | (int val) const
        {
                Time res;

                auto i1 = values.begin();
                for (; i1 != values.end(); ++i1) {
                        if (i1->first <= val) {
                                res._sum += i1->second;
                        } else
                                break;
                }
                res.values.insert(res.values.begin(), values.begin(), i1);
                return res;
        }

        void split(int val, Time *t1, Time *t2) const
        {
                auto i1 = values.begin();
                for (; i1 != values.end(); ++i1) {
                        if (i1->first <= val) {
                                t1->_sum += i1->second;
                        } else {
                                break;
                        }
                }
                t1->values.insert(t1->values.begin(), values.begin(), i1);
                t2->values.insert(t2->values.begin(), i1, values.end());
                // explicitly sum the values here, instead of using the
                // difference.
                for (; i1 != values.end(); ++i1) {
                        t2->_sum += i1->second;
                }
        }

        bool isEmpty() const
        {
                return values.empty() || _sum == 0;
        }

        int minVal() const
        {
                return values.begin()->first;
        }

        int maxVal() const
        {
                return values.rbegin()->first;
        }

        void scale(double s)
        {
                assert(s==s);
                for (auto i1 = values.begin(); i1 != values.end(); ++i1) {
                        i1->second *= s;
                }
                _sum *= s;
        }

        double sum() const
        {
                return _sum;
        }

        void normalize()
        {
                if (_sum) {
                        scale(1/_sum);
                }
        }

        double operator == (int v)
        {
                auto it = std::find_if(values.begin(), values.end(),
                                       [v](auto a) { return a.first == v; });
                if (it == values.end()) {
                        return 0.0;
                }
                return it->second;
        }

        void ugly_update_first_value_for_total_sum(int min, double s)
        {
                assert(values.size() == 0 || min < minVal());
                values.insert(values.begin(), VP(min, s - _sum));
                _sum = s;
        }

        friend Time operator | (int val, const Time& t);
        friend std::ostream& operator << (std::ostream & os, Time const & time);
        friend Time min(Time const & t1, Time const & t2);
};

Time operator | (int val, Time const & t);
Time operator - (int val, Time const & t);

inline Time operator + (int val, Time const & t)
{
        return t + val;
}

Time min(Time const & t1, Time const & t2);

std::ostream& operator << (std::ostream & is, Time const & time);
std::istream& operator >> (std::istream & is, Time & time);
