// -*- C++ -*-

#pragma once

#include <vector>
#include <set>
#include <algorithm>
#include <memory>

#include "sem.h"
#include "job.h"
#include "scheduler.h"
#include "dbl.h"

class Simulator
{
private:
        std::vector<Job*> jobs;
        Scheduler *sched;
	std::unique_ptr<Job> jobPart;
        JobCmp cmp;
        int crit;
        Time time;

        void setJobPart(std::unique_ptr<Job>);

public:
        Simulator *sim_list_next;

        static Semaphore sem;
        static Semaphore psem;
        static std::mutex mutex;
        static Simulator *sim_list;

public:
        Simulator(Scheduler *_sched, Time _time,
                  std::vector<Job*> *_jobs = nullptr, int _crit = 0)
                :
                jobs(_jobs ? *_jobs : _sched->jobs), sched(_sched),
                jobPart(nullptr), cmp(sched->jobCmp(_crit)),
                crit(_crit), time(_time)
        {
                // If a jobs vector is passed, it is expected to be sorted.
                // Else, jobs is sorted into the order of descending release
                // times.
                if (!_jobs) {
                        std::sort(jobs.begin(), jobs.end(),
                                  [](Job const *j1, Job const *j2)
                                  { return j1->release() > j2->release(); });
                }
        }

        ~Simulator() = default;
	Simulator(Simulator&&) = default;

	static void print_statistic();

        void jumpToTime(int t);
        Job* nextJob();
        int nextSchedEventFor(Job *job);

        void removeJob(Job *oldJ);
        void updateJob(Job *oldJ, Job *newJ);

        void findCritDeadlinesUpto(Time const & t, std::vector<Job*> *_jobs);

        void runNextLevel(Simulator& sim, int level);

        void run(int level);
};
