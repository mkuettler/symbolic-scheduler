
#include "real-simulator.h"

#include <iostream>

RealSim::RealSim(Scheduler *_sched, int _time) :
        sched(_sched), time(_time), jobs(sched->jobs),
        crit(0), cmp(sched->jobCmp(crit))
{
        std::sort(jobs.begin(), jobs.end(), [](Job const *j1,
                                               Job const *j2)
                  { return j1->release() > j2->release(); });
}

RealSim::~RealSim()
{
}

Job *RealSim::nextJob()
{
        Job *job = nullptr;
        auto it = jobs.rbegin();
        for (; it != jobs.rend() && (*it)->release() <= time; ++it) {
                if (!job || cmp(job, *it)) {
                        job = *it;
                }
        }
        if (!job && it != jobs.rend()) {
                job = *it;
                // Among the jobs with the next release, find the one with
                // the highest prio
                ++it;
                for (; it != jobs.rend() && (*it)->release() == job->release();
                     ++it) {
                        if (cmp(job, *it)) {
                                job = *it;
                        }
                }
                time = job->release();
        }
        return job;
}

int RealSim::nextSchedEventFor(Job *job)
{
        int t = job->deadline();
        for (auto it = jobs.rbegin();
             it != jobs.rend() && (*it)->release() < t;
             ++it) {
                if (cmp(job, *it) && job != *it){
                        return (*it)->release();
                }
        }
        return t;
}

void RealSim::findCritDeadlinesUpto(int t)
{
        for (auto it = jobs.rbegin();
             it != jobs.rend() && (*it)->release() <= t;
             ++it) {
                if ((*it)->deadline() <= t) {
                        if ((*it)->crit() >= crit) {
                                (*it)->addCritDL(1);
                                //assert(false);
                        }
                        // This is ugly!! reverse_iterator<..>::base
                        // returns an iterator to the element _after_ the
                        // reverse iterator's...
                        //std::cout << "Dropping " << **it << " ("
                        //          << *it << " == " << *(it.base() - 1) << ")"
                        //          << std::endl;
                        jobs.erase(it.base() - 1);
                }
        }
}

void RealSim::removeJob(Job *job)
{
        for (auto it = jobs.rbegin(); it != jobs.rend(); ++it) {
                if (*it == job)
                        jobs.erase(it.base() - 1);
        }
        if (job->parentJob())
                delete job;
}

void RealSim::updateJob(Job *oldjob, Job *newjob)
{
        for (auto it = jobs.rbegin(); it != jobs.rend(); ++it) {
                if (*it == oldjob)
                        *it = newjob;
        }
        if (oldjob->parentJob())
                delete oldjob;
}

void RealSim::raise_crit()
{
        ++crit;
        cmp = sched->jobCmp(crit);
        // No need to resort jobs: They are sorted by the release time
        // so that the next one can be found easily.
}

void RealSim::run()
{
        while (!jobs.empty()) {
                Job *job = nextJob();
                int nextSched = nextSchedEventFor(job);
                int newtime = time + job->full_et().choose();
                Time const & wcet = job->wcet(crit);
                int critTime = wcet.isEmpty() ? nextSched + 1
                        : time + job->wcet(crit).minVal();
                //std::cout << "at " << time << ": "<< *job << " with "
                //<< newtime << " and " << nextSched << "/" << critTime
                //<< " (crit = " << crit << ")" << std::endl;
                if (newtime <= nextSched &&
                    newtime <= critTime) {
                        job->addSuccess(1);
                        removeJob(job);
                        time = newtime;
                        findCritDeadlinesUpto(time);
                } else if (critTime <= nextSched && crit < job->crit()) {
                        raise_crit();
                        findCritDeadlinesUpto(critTime);
                        Job *newjob = new Job(job, crit);
                        updateJob(job, newjob);
                        time = critTime;
                } else {
                        if (nextSched < job->deadline()) {
                                Job *newjob = new
                                        Job(job, Time(nextSched-time), crit);
                                updateJob(job, newjob);
                        }
                        time = nextSched;
                        findCritDeadlinesUpto(time);
                }
        }
}
