
#include "real-simulator.h"
#include "task.h"
#include "scheduler.h"

#include "threadnum.h"

#include <vector>
#include <iostream>

int thread_local threadnum = 0;

int main(int argc, char **argv)
{
        std::vector<Task*> tasks;
        Task::readTasks(std::cin, tasks);
        for(int i = 0; i < tasks.size(); ++i) {
                std::cout << *tasks[i] << std::endl;
        }

        char sc = argc > 1 ? argv[1][0] : 'd';
        Scheduler *sched = Scheduler::create(sc, tasks);
        std::cout << sched->jobs.size() << " Jobs" << std::endl;
        int const count = 100000;
        for (int i = 0; i < count; ++i) {
                //if (i % 1000 == 0) std::cout << i << std::endl;
                RealSim sim(sched, 0);
                sim.run();
                //std::cout << std::endl;
        }

        for (auto it = sched->jobs.begin(); it != sched->jobs.end(); ++it) {
                std::cout << **it << "  " << (*it)->successProb() / count;
                if ((*it)->critDLCount() > 0) {
                        std::cout << " (" << static_cast<double>
                                ((*it)->critDLCount()) / count
                                  << " " << (*it)->critDLProb() / count << ")";
                }
                std::cout << std::endl;
        }
        std::cout << std::endl;
        for (auto it = tasks.begin(); it != tasks.end(); ++it) {
                double prob = 0;
                for (auto jt =  (*it)->jobs.begin();
                     jt != (*it)->jobs.end(); ++jt) {
                        prob += (*jt)->successProb();
                }
                prob /= (*it)->jobs.size();
                std::cout << **it << "  " << prob << std::endl;
        }
        return 0;
}
