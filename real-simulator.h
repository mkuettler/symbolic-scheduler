// -*- C++ -*-

#pragma once

#include "scheduler.h"
#include "job.h"
#include <vector>
#include "dbl.h"

class RealSim
{
private:
        Scheduler *sched;
        int time;
        std::vector<Job*> jobs;
        int crit;
        JobCmp cmp;

public:
        RealSim(Scheduler *_sched, int _time);
        ~RealSim();

        Job* nextJob();
        int nextSchedEventFor(Job *job);

        void findCritDeadlinesUpto(int t);

        void removeJob(Job *job);
        void updateJob(Job *oldjob, Job* newjob);
        void raise_crit();

        void run();
};
