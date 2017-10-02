// -*- C++ -*-

#pragma once

#include <string>
#include <vector>
#include <ostream>
#include <istream>

#include "time.h"
#include "job.h"
#include "dbl.h"

class Task
{
public:
        std::string name;
        std::vector<std::pair<Time, Time> > ets;
        std::vector<Time> wcets;
        int period;
        int deadline;
        int crit;
        std::vector<Job*> jobs;

private:
        void constructEts(Time const & et, std::vector<int> const & wcets);

public:
        Task(std::string name, Time const & et, int period, int deadline,
             std::vector<int> const & wcets);

        Task(std::istream&);

        Task(Task const &) = delete;
        Task(Task &&) = delete;

        Task& operator = (Task const &) = delete;
        Task& operator = (Task &&) = delete;

        ~Task();

        static void readTasks(std::istream& is, std::vector<Task*>& tasks);

        Job* getJob(unsigned int n);
};


std::ostream& operator << (std::ostream& os, Task const &task);
