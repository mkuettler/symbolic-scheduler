
#include "job.h"

#include <iostream>
#include <cassert>
#include <cstring>

Job::Job(Task *task, int num)
        : task(task), job(nullptr), num(num), partnum(0),
          _release(num*task->period),
          _deadline(_release + task->deadline),
          _ets(nullptr), _wcets(nullptr),
          _successProb(PARALLEL_CNT),
          _critDLProb(PARALLEL_CNT),
          _critDLCount(PARALLEL_CNT)
{
        assert(task);
        //std::cout << this <<  " Job->et: " << et << std::endl;
}

Job::Job(Job *j, Time const & time_passed, int crit)
        : task(j->task), job(j->job ? j->job : j), num(j->num),
          partnum(j->partnum+1),
          _release(j->_release),
          _deadline(j->_deadline),
          _ets(std::make_unique<std::vector<Time>>(task->crit+1)),
          _wcets(std::make_unique<std::vector<Time>>(task->crit+1))
{
        assert(job);
        //if (task->name == "X2" && partnum == 1) {
        //        std::cout << "qwertz\n";
        //}
        crit = (crit <= task->crit) ? crit : task->crit;
        for (int i = crit; i <= task->crit; ++i) {
                double s = j->et(i).sum();
                (*_ets)[i] = 0 | (j->et(i) - time_passed);
                s -= (*_ets)[i].sum();
                if (s < 1) {
                        (*_ets)[i].scale(1 / (1-s));
                }
                (*_wcets)[i] = 0 | (j->wcet(i) - time_passed);
                (*_wcets)[i].normalize();
        }
        //std::cout << this << " Job->et: " << et  << " " << et->size()
        //          << std::endl;
}

Job::Job(Job *j, int crit)
        : task(j->task), job(j->job ? j->job : j), num(j->num),
          partnum(j->partnum+1),
          _release(j->_release),
          _deadline(j->_deadline),
          _ets(std::make_unique<std::vector<Time>>(task->crit+1)),
          _wcets(std::make_unique<std::vector<Time>>(task->crit+1))
{
        assert(job);
        //if (task->name == "X2" && partnum == 1) {
        //        assert(false);
        //}
        int c = (crit < task->crit) ? crit : task->crit;
        double s;
        if (c > 0) {
                s = task->ets[c-1].first.sum();
                assert(s);
                s = (s == 1) ? (double)1 : 1 / (1-s);
                (*_ets)[c] = task->ets[c-1].second;
                (*_ets)[c].scale(s);
        } else {
                std::cout << "Created JobPart after criticality miss, with "
                          << "new crit == " << crit
                          << ". This shouldn't happen" << std::endl;
                assert(false);
        }
        for (int i = crit+1; i <= task->crit; ++i) {
                (*_ets)[i] = (*_ets)[i-1].merge(task->ets[i-1].second);
                (*_ets)[i].scale(s);
        }
        for (int i = crit; i <= task->crit; ++i) {
                (*_wcets)[i] = task->wcets[i] - task->wcets[crit-1];
        }
}

std::ostream& operator << (std::ostream& os, Job const &job)
{
        os << job.task->name << '#' << job.num;
        if (job.job) {
                os << '#' << job.partnum;
        }
        os << " " << job.release() << " " << job.deadline() << " ("
           << (job._ets ? (*job._ets)[job.crit()] :
               job.task->ets[job.crit()].first) << ")";
        return os;
}
