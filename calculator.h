// -*- C++ -*-

#include <vector>
#include <iostream>

#include "job.h"
#include "scheduler.h"
#include "dbl.h"

class TaskCmp
{
private:
        JobCmp cmp;
public:
        TaskCmp(JobCmp _cmp) : cmp(_cmp) {}

        int operator () (Task *t1, Task *t2)
        {
                return cmp(t1->jobs[0], t2->jobs[0]);
        }
};

class Calculator
{
private:
        Scheduler *sched;
        std::vector<Task*> tasks;
        std::vector<Job*> jobs;
        TaskCmp cmp;
        int num_high;
        int num_low;

public:
        Calculator(Scheduler *_sched)
                : sched(_sched), tasks(sched->tasks), jobs(sched->jobs),
                  cmp(sched->jobCmp(0))
        {
                // Check that crit is 0 or 1 for every task
                for (int i = 0; i < tasks.size(); ++i) {
                        if (tasks[i]->crit != 0 && tasks[i]->crit != 1) {
                                std::cerr << "Task " << tasks[i]
                                          << " has crit = " << tasks[i]->crit
                                          << std::endl;
                        }
                }
                // TODO: Make sure that each task has a fixed priority

                // sort the tasks by priority
                std::sort(tasks.begin(), tasks.end(),
                          [this](Task *t1, Task *t2)
                          { return cmp(t2, t1); });


        }

        void run();
        void run_case0();
        void run_case1();
        void run_case2();
        void run_case3();
};
