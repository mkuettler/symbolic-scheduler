
#include "simulator.h"
#include "task.h"
#include "scheduler.h"

#include "sem.h"
#include "threadnum.h"

#include <vector>
#include <iostream>

int thread_local threadnum;

int main(int argc, char **argv)
{
        std::vector<Task*> tasks;

        Task::readTasks(std::cin, tasks);

        for(int i = 0; i < tasks.size(); ++i) {
                std::cout << *tasks[i] << std::endl;
        }

        /*std::map<double, double> t1et;
        t1et[1] = 0.5;
        t1et[2] = 0.3;
        t1et[3] = 0.2;
        Time t1(t1et);
        tasks.push_back(new Task("A", t1, 4, 4, {2, 4}));*/

        threadnum = 0;
        std::vector<std::thread> threadpool;
        for (int i = 0; i < std::thread::hardware_concurrency(); ++i)
                threadpool.emplace_back([i]{
                                threadnum = i+1;

                                while (1) {
                                        Simulator::psem.up();
                                        Simulator::sem.down();
                                        Simulator *s;
                                        {
                                        std::lock_guard<std::mutex> lock
                                                (Simulator::mutex);
                                        s = Simulator::sim_list;
                                        if (!s) {
                                                std::cout << "thread "
                                                          << std::this_thread
                                                        ::get_id()
                                                          << " is done."
                                                          << std::endl;
                                                return;
                                        }
                                        std::cout << "thread "
                                                << std::this_thread::get_id()
                                                << " picked up work."
                                                << std::endl;
                                        Simulator::sim_list = \
                                                Simulator::sim_list->
                                                sim_list_next;
                                        }
					// this is not correct, but it just
					// needs to be more than the
					// split level
                                        s->run(100);
                                        delete s;
                                }
                        });

        std::cout << "hardware concurrency == "
                  << std::thread::hardware_concurrency()
                  << std::endl;
        if (std::thread::hardware_concurrency() > 64) {
                std::cout << "That's too much!" << std::endl;
                return 1;
        }

        char sc = argc > 1 ? argv[1][0] : 'd';
        Scheduler *sched = Scheduler::create(sc, tasks);
        std::cout << sched->jobs.size() << " Jobs" << std::endl;
        Simulator sim(sched, Time(0,1), nullptr, 0);
        sim.run(0);

        for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
                Simulator::sem.up();
        }
        for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
                threadpool[i].join();
        }

        for (auto it = sched->jobs.begin(); it != sched->jobs.end(); ++it) {
                std::cout << **it << "  ";
                std::cout.precision(15); // << std::fixed
                std::cout << (*it)->successProb();
                if ((*it)->critDLCount() > 0) {
                        std::cout << " (" << (*it)->critDLCount()
                                  << " " << (*it)->critDLProb() << ")";
                }
                std::cout.precision(6);
                std::cout << std::endl;
        }

        std::cout << std::endl;
	sim.print_statistic();
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
