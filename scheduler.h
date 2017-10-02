// -*- C++ -*-

#pragma once

#include "job.h"
#include "dbl.h"

class Scheduler;

class JobCmp
{
private:
        Scheduler *sched;
        int crit;
public:
        JobCmp(Scheduler *s, int c) : sched(s), crit(c)
        {}
        ~JobCmp()
        {}

        int operator () (Job *j1, Job *j2);
};

class Scheduler
{
public:
        std::vector<Task*> tasks;
        std::vector<Job*> jobs;
        double hp;
        int max_crit;

        Scheduler(std::vector<Task*> const & t);

        ~Scheduler();

        virtual int prioLessThanEq(Job const *j1, Job const *j2, int crit) = 0;

        static Scheduler *create(char c, std::vector<Task*> const & t);

        JobCmp jobCmp(int crit)
        { return JobCmp(this, crit); }
};

class DefaultScheduler : public Scheduler
{
public:
        DefaultScheduler(std::vector<Task*> const & t) : Scheduler(t)
        {}

        int prioLessThanEq(Job const *j1, Job const *j2, int);
};

class EDFScheduler : public Scheduler
{
public:
        EDFScheduler(std::vector<Task*> const & t) : Scheduler(t)
        {}

        int prioLessThanEq(Job const *j1, Job const *j2, int crit);
};
