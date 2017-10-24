// -*- C++ -*-

#pragma once

#include "scheduler.h"
#include "job.h"
#include <vector>
#include "dbl.h"

class Simulator
{
private:
        Scheduler *sched;
        int time;
        std::vector<Job*> jobs;
        int crit;
        JobCmp cmp;

public:
        Simulator(Scheduler *_sched, int _time);
        ~Simulator();

        Job* nextJob();
        int nextSchedEventFor(Job *job);

        void findCritDeadlinesUpto(int t);

        void removeJob(Job *job);
        void updateJob(Job *oldjob, Job* newjob);
        void raise_crit();

        void run();
};
