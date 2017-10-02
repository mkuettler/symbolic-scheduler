// -*- C++ -*-

#pragma once

#include <vector>
#include <memory>
#include <ostream>
#include <cstring>

#include "time.h"
#include "threadnum.h"
#include "dbl.h"

class Task;

#define PARALLEL_CNT 64+1

struct Double
{
private:
        double val[8];
public:
        Double() { std::memset(val, 0, 8*sizeof(double)); }
        ~Double() {}

        void add(double d)
        {
                int const step = 1 << 26;
                double l = 1.0 / step;
                int i;
                for (i = 7; d <= l && i; --i, l/=step)
                        ;
                val[i] += d;
        }

        double value() const
        {
                double v = 0;
                for (int i = 0; i < 8; ++i)
                        v += val[i];
                return v;
        }
} __attribute__((aligned(64)));

struct Int
{
        int val;
} __attribute__((aligned(64)));

class Job
{
public: //private:
        Task *task;
        Job *job;
        int num;
        int partnum;
        double _release;
        double _deadline;
	std::unique_ptr<std::vector<Time>> _ets;
	std::unique_ptr<std::vector<Time>> _wcets;
	std::vector<Double> _successProb;
	std::vector<Double> _critDLProb;
	std::vector<Int> _critDLCount;

public:
        Job(Task *task, int num);
        Job(Job *j, Time const & time_passed, int crit);
        Job(Job *j, int crit);

        Job(Job const &) = delete;
        Job(Job&&) = delete;

        Job& operator = (Job const &) = delete;
        Job& operator = (Job&&) = delete;

        ~Job() = default;

        Job *parentJob() const
        { return job; }

        int jobNum() const
        { return num; }

        inline double release() const;
        inline double deadline() const;

        inline Time const & full_et() const;
        inline Time const & et(int crit) const;

        inline Time const & wcet(int crit) const;

        inline int crit() const;

        inline double period() const;

        inline void addSuccess(double p);
        inline void addCritDL(double p);
        inline double successProb() const;
        inline double critDLProb() const;
        inline int critDLCount() const;

        inline std::string const& taskName() const;

        friend std::ostream& operator << (std::ostream& os, Job const& job);

};

#include "task.h"

inline double Job::release() const
{
        return _release;
}

inline double Job::deadline() const
{
        return _deadline;
}

inline Time const & Job::full_et() const
{
        return _ets ? (*_ets)[crit()] : task->ets[task->crit].first;
}

inline Time const & Job::et(int crit) const
{
        if (!_ets) {
                return crit <= task->crit ? task->ets[crit].first :
                        task->ets[task->crit].first;
        } else {
                return (*_ets)[crit <= task->crit ? crit : task->crit];
        }
}

inline Time const & Job::wcet(int crit) const
{
        static Time no_time = Time();
        if (!_wcets) {
                return crit <= task->crit ? task->wcets[crit] :
                        no_time;
        } else {
                return (*_wcets)[crit <= task->crit ? crit : task->crit];
        }
}

inline int Job::crit() const
{
        return task->crit;
}

inline double Job::period() const
{
        return task->period;
}

inline void Job::addSuccess(double p)
{
        if (job) {
                job->addSuccess(p);
        } else {
                _successProb[threadnum].add(p);
        }
}

inline void Job::addCritDL(double p)
{
        if (job) {
                job->addCritDL(p);
        } else {
                _critDLProb[threadnum].add(p);
                _critDLCount[threadnum].val += 1;
        }
}

inline double Job::successProb() const
{
        double s = 0;
        for (int i = 0; i < PARALLEL_CNT; ++i)
                s += _successProb[i].value();
        return s;
}

inline double Job::critDLProb() const
{
        double s = 0;
        for (int i = 0; i < PARALLEL_CNT; ++i)
                s += _critDLProb[i].value();
        return s;
}

inline int Job::critDLCount() const
{
        int s = 0;
        for (int i = 0; i < PARALLEL_CNT; ++i)
                s += _critDLCount[i].val;
        return s;
}

inline std::string const& Job::taskName() const
{
        return task->name;
}


std::ostream& operator << (std::ostream& os, Job const &job);
