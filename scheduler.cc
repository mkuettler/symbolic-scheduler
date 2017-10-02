
#include <cmath>

#include "scheduler.h"
#include "job.h"

using std::fmod;

double gcd(double a, double b)
{
        while (b) {
                double t = b;
                b = fmod(a, b);
                a = t;
        }
        return a;
}

double lcm(double a, double b)
{
        return (a*b) / gcd(a,b);
}

double hyperperiod(std::vector<Task*> const & tasks)
{
        double h = 1;
        for (auto t: tasks) {
                h = lcm(h, t->period);
        }
        return h;
}

int JobCmp::operator () (Job *j1, Job *j2)
{
        return sched->prioLessThanEq(j1, j2, crit);
}

Scheduler::Scheduler(std::vector<Task*> const & t) : tasks(t)
{
        hp = hyperperiod(tasks);
        for (auto& t: tasks) {
                for (int i = 0; i < hp/t->period; ++i) {
                        jobs.push_back(t->getJob(i));
                }
        }
}

Scheduler::~Scheduler()
{}

Scheduler *Scheduler::create(char c, std::vector<Task*> const & t)
{
        static Scheduler *sched = nullptr;
        assert(!sched);
        switch(c) {
        case 'd':
                printf("Using rate-monotonic scheduler.\n");
                return sched = new DefaultScheduler(t);
        case 'e':
                printf("Using edf scheduler.\n");
                return sched = new EDFScheduler(t);
        default:
                printf("Unknown Scheduler-character: %c", c);
                assert(false);
        }
}

int DefaultScheduler::prioLessThanEq(Job const *j1, Job const *j2, int)
{
        return j1->crit() < j2->crit() ||
                (j1->crit() == j2->crit()
                 && (j1->period() > j2->period() ||
                     (j1->period() == j2->period() &&
                      j1->taskName() <= j2->taskName())));
}

int EDFScheduler::prioLessThanEq(Job const *j1, Job const *j2, int crit)
{
        if ((j1->crit() >= crit) == (j2->crit() >= crit)) {
                return j1->deadline() > j2->deadline() ||
                        (j1->deadline() == j2->deadline() &&
                         j1->taskName() <= j2->taskName());
        } else if (j1->crit() >= crit) {
                return false;
        } else {
                return true;
        }
}
