
#include "time.h"

#include <random>

void min_split(Time const & t1, Time const & t2, Time * s1, Time * s2)
{
        t1.split(t2.minVal(), s1, s2);
        s1->scale(t2.sum());
}

Time min(Time const & t1, Time const & t2)
{
        if (t1.minVal() > t2.minVal())
                return min(t2, t1);
        Time res;
        Time t11, t12, t21, t22;
        t12 = t1;
        t22 = t2;
        while (!t12.isEmpty() && !t22.isEmpty()) {
                if (t12.minVal() < t22.minVal()) {
                        min_split(t12, t22, &t11, &t12);
                        res += t11;
                } else {
                        min_split(t22, t12, &t21, &t22);
                        res += t21;
                }
        }
        return res;
}

int Time::choose() const
{
        static std::uniform_real_distribution<dbl::Data /*double*/> unif(0,1);
        static std::random_device rd;
        static std::mt19937 re(rd());
        double r = unif(re) * _sum;
        for (auto it = values.begin(); it != values.end(); ++it) {
                r -= it->second;
                if (r <= 0) return it->first;
        }
        // If we got here our sum isn't quite exact. That's ok.
        return values.rbegin()->first;
}

Time operator | (int val, const Time& t)
{
        Time res;

        double s = 0;
        auto i1 = t.values.begin();
        for (; i1 != t.values.end(); ++i1) {
                if (i1->first <= val) {
                        //s += i1->second;
                } else {
                        break;
                }
        }
        res.values.reserve(t.values.end() - i1);
        res.values.insert(res.values.begin(), i1, t.values.end());
        for (; i1 != t.values.end(); ++i1) {
                s += i1->second;
        }
        res._sum = s;
        return res;
}

Time operator - (int val, Time const & t)
{
        return Time(val) - t;
}

std::ostream& operator << (std::ostream & os, Time const & time)
{
        if (!time.isEmpty()) {
                os << time.minVal() << "-" << time.maxVal() << " (" << time.sum()
                   << ")";
        } else {
                os << "---";
        }

        os << "[";
        for (auto it = time.values.begin(); it != time.values.end(); ++it) {
                os << it->first << ":" << it->second << ", ";
        }
        os << "]";
        return os;
}

std::istream& operator >> (std::istream & is, Time & time)
{
        std::map<int, double> vals;
        int v;
        double vd;
        double p;
        char c;
        is >> vd >> c >> p;
        v = static_cast<int>(vd);
        assert(v == vd);
        vals[v] = p;
        assert(c == ':');
        while (is.peek() == ',') {
                is >> c;
                is >> v >> c >> p;
                vals[v] = p;
        }
        time = Time(vals);
        return is;
}
