
#include "symbolic_scheduler.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <atomic>
#include <iomanip>

#define IFOUT if (0)
#define OUT IFOUT std::cout

static std::array<std::atomic<int>, 1000> case1_count;
static std::array<std::atomic<int>, 1000> case2_count;
static std::array<std::atomic<int>, 1000> case3_jobpart_count;
static std::array<std::atomic<int>, 1000> case3_nojobpart_count;
static std::array<std::atomic<int>, 1000> case1and3_nojobpart_count;

std::vector<std::unique_ptr<Job>> job_parts;

Semaphore SymbolicScheduler::sem(0);
Semaphore SymbolicScheduler::psem(0);
std::mutex SymbolicScheduler::mutex;
SymbolicScheduler *SymbolicScheduler::sim_list = nullptr;

const int split_level = 26;

void SymbolicScheduler::print_statistic()
{
	auto print_vec = [](auto&& vec) {
		auto ep = std::find_if(vec.rbegin(), vec.rend(),
				       [](auto&& i) {return i != 0; }).base();
		for (auto it = vec.begin(); it != ep; ++it)
			std::cout << std::setw(4) << *it << ", ";
	};
	std::cout << "Case 1:" << std::endl;
	print_vec(case1_count);
	std::cout << std::endl << "Case 2:" << std::endl;
	print_vec(case2_count);
	std::cout << std::endl << "Case 3, with jobpart:" << std::endl;
	print_vec(case3_jobpart_count);
	std::cout << std::endl << "Case 3, no jobpart:" << std::endl;
	print_vec(case3_nojobpart_count);
	std::cout << std::endl << "Case 1+3, no jobpart:" << std::endl;
	print_vec(case1and3_nojobpart_count);
	std::cout << std::endl;
}

void SymbolicScheduler::setJobPart(std::unique_ptr<Job> j)
{
        jobPart = std::move(j);
}

void SymbolicScheduler::jumpToTime(int t)
{
        assert(t >= time.minVal());
        time = time >> t;
}

Job *SymbolicScheduler::nextJob()
{
        Job *job = nullptr;
        auto it = jobs.rbegin();
        for (;
             it != jobs.rend() && (*it)->release() <= time.minVal();
             ++it) {
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
                jumpToTime(job->release());
        }
        return job;
}

void SymbolicScheduler::removeJob(Job *oldJ)
{
        int i = jobs.size() - 1;
        if (oldJ->parentJob()) oldJ = oldJ->parentJob();
        for (; i >= 0 && jobs[i] != oldJ &&
                     jobs[i]->parentJob() != oldJ; --i)
                ;
        if (i >= 0)
                jobs.erase(jobs.begin() + i);
}

void SymbolicScheduler::updateJob(Job *oldJ, Job *newJ)
{
        int i = jobs.size() - 1;
        if (oldJ->parentJob()) oldJ = oldJ->parentJob();
        for (; i >= 0 && jobs[i] != oldJ &&
                     jobs[i]->parentJob() != oldJ; --i)
                ;
        if (i >= 0) {
                jobs[i] = newJ;
        } else {
                for (i = jobs.size() - 1; i >= 0 &&
                             jobs[i]->release() < newJ->release(); --i)
                        ;
                jobs.insert(jobs.begin()+i+1, newJ);
        }
}

int SymbolicScheduler::nextSchedEventFor(Job *job)
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

void SymbolicScheduler::findCritDeadlinesUpto(Time const & t, std::vector<Job*> *_jobs)
{
        int tmin = t.minVal();
        for (auto it = _jobs->rbegin();
             it != _jobs->rend() && (*it)->release() <= tmin;
             ++it) {
                if ((*it)->deadline() <= tmin) {
                        if ((*it)->crit() >= crit) {
                                //std::cout << "Crit deadline miss! "
                                //          << *(*it) << std::endl;
                                (*it)->addCritDL(t.sum());
                        }
                        // This is ugly!! reverse_iterator<..>::base
                        // returns an iterator to the element _after_ the
                        // reverse iterator's...
                        //std::cout << "Dropping " << **it << " ("
                        //          << *it << " == " << *(it.base() - 1) << ")"
                        //          << std::endl;
                        _jobs->erase(it.base() - 1);
                }
        }
}

void SymbolicScheduler::runNextLevel(SymbolicScheduler& sim, int level)
{
        if (level != split_level) {
                sim.run(level+1);
                // dirty ugly hack!
                // Keep all jobParts below the split level around, they might be needed later.
                if (level < split_level and sim.jobPart != nullptr) {
	                job_parts.push_back(std::move(sim.jobPart));
                }
        } else {
                {
			auto newsim = std::make_unique<SymbolicScheduler>
				(std::move(sim)).release();
                        std::cout << " Add workpackage." << std::endl;
                        std::unique_lock<std::mutex> lock(mutex);
                        newsim->sim_list_next = sim_list;
                        sim_list = newsim;
                }
                sem.up();
                psem.down();
        }
}

// TODO:
//
//  - Don't allocate and delete all the SymbolicSchedulers, keep them on the
//    stack and only allocate on the split_level
//
//  - Merge case 1 and 3 below?  In both cases the current job is done,
//    and the criticaltiy level doesn't change
//
//  - I think level shouldn't be a parameter to the SymbolicScheduler, but
//    rather to run (and runNextLevel, where it is already provided)
//
//  - Avoid recomputing sub-trees.  Eg. T1(4), T2(8), T3(16).  T3#1
//    might finish before 8 or be interrupted there; T1#3, T1#4, and
//    T2#2 woun't change depending on that.  I shouldn't recompute them!

void SymbolicScheduler::run(int level)
{
	bool case1 = false;
        static std::string indent = "";
        IFOUT indent += " ";
        //if (level == 6)
        //        std::cout << "Level == 6" << std::endl;
        //if (level == 16)
        //        std::cout << " Level == 16" << std::endl;
        //if (level == 26)
        //        std::cout << "  Level == 26" << std::endl;
        Job *job = nextJob();
        if (!job) {
                IFOUT indent.pop_back();
                return;
        }

        OUT << indent
	    << "At " << time << " selected Job (" << job << "): "
	    << *job << std::endl;

        int nextSched = nextSchedEventFor(job);

        OUT << indent
	    << "Next event at " << nextSched << std::endl;

        Time et = job->et(crit);
        Time newtime = time + et;
        Time t1, t2;
        newtime.split(nextSched, &t1, &t2);
        OUT << indent << "Finished at: " << t1
	    << "   " << t2 << std::endl;
        // Case 1: Adding the time of computation before the next crit. switch
        //         gives a value less than before the next scheduling event.
        if (!t1.isEmpty()) {
		OUT << indent << "Case 1: " << *job << " finishes\n";
                job->addSuccess(t1.sum());
                auto sim = SymbolicScheduler(sched, t1, &jobs, crit);
                sim.removeJob(job);
                findCritDeadlinesUpto(t1, &sim.jobs);
		++case1_count[level];
		case1 = true;
                runNextLevel(sim, level);
        }

        Time ct1, ct2;
        if (crit < job->crit()) {
                Time critLength = job->wcet(crit);
                assert(et.sum() < 1.00001);
                critLength.scale(1-et.sum());
                Time critTime = time + critLength;
                critTime.split(nextSched, &ct1, &ct2);
        }
        OUT << indent << "Crit miss at: " << ct1
                  << "   " << ct2 << std::endl;
        // Case 2: The criticality switch happens before the next scheduling
        //         event
        if (!ct1.isEmpty()) {
		OUT << indent << "Case 2: " << *job << " crit miss\n";
                auto sim = SymbolicScheduler(sched, ct1, &jobs, crit+1);
                findCritDeadlinesUpto(ct1, &sim.jobs);
                ct1.normalize();
                sim.setJobPart(std::make_unique<Job>(job, crit+1));
                sim.updateJob(job, sim.jobPart.get());
		++case2_count[level];
                runNextLevel(sim, level);
        }

        double s = t2.sum() + ct2.sum();
        OUT << indent << "time after sched: " << s << std::endl;
        // Case 3: Neither of the above, i.e. the scheduling event
        //         triggers first.
        if (s > 0) {
		OUT << indent << "Case 3: " << *job << " descheduled\n";
                Time schedTime = nextSched | time;
                double sched_sum = schedTime.sum();
                if (sched_sum > s * 1.00001) {
                        std::cerr << "Error: Found starting after the next "
                                  << "scheduling event to be more likely "
                                  << "than ending there!\n"
                                  << "  (" << sched_sum << " > " << s
                                  << "  )\n";
                        assert(false);
                }
                if (sched_sum < s) {
                        schedTime.ugly_update_first_value_for_total_sum
                                (nextSched, s);
                        //schedTime.values[nextSched] = s - sched_sum;
                        //schedTime._sum = s;
                }
                auto sim = SymbolicScheduler(sched, schedTime, &jobs, crit);
                Time nextSchedTime(nextSched, schedTime.sum());
                findCritDeadlinesUpto(nextSchedTime, &sim.jobs);
                if (job->deadline() > nextSched) {
                        Time time_passed = nextSched - time;
			time_passed = time_passed >> 0;
                        time_passed.normalize();
			OUT << "In case 3: " << nextSched << " - " << time << " = " << time_passed << '\n';
                        sim.setJobPart(std::make_unique<Job>
				       (job, time_passed, crit));
                        sim.updateJob(job, sim.jobPart.get());
			++case3_jobpart_count[level];
                } else {
			++case3_nojobpart_count[level];
			if (case1)
				++case1and3_nojobpart_count[level];
		}
                runNextLevel(sim, level);
        }

        IFOUT indent.pop_back();
}
