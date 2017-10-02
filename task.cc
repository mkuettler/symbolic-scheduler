
#include "job.h"
#include "task.h"

Task::Task(std::string name, Time const & et, int period, int deadline,
           std::vector<int> const & wcets) :
        name(name), period(period), deadline(deadline),
        crit(wcets.size()-1), jobs()
{
        constructEts(et, wcets);
}

Task::Task(std::istream& is)
{
        Time _time;
        std::vector<int> _wcets;
        //double p, d;
        is >> std::skipws >> name >> period >> deadline;
        //period = static_cast<int>(p);
        //deadline = static_cast<int>(d);
        std::cout << "Task " << name << " "  << period << " " << deadline << std::endl;
        //assert(period == p);
        //assert(deadline == d);
        is >> _time;
        int tmp;
        double dtmp;
        is >> dtmp;
        tmp = static_cast<int>(dtmp);
        assert(tmp == dtmp);
        _wcets.push_back(tmp);
        char c;
        while (is.peek() == ',') {
                is >> c >> dtmp;
                tmp = static_cast<int>(dtmp);
                assert(tmp == dtmp);
                _wcets.push_back(tmp);
        }
        crit = _wcets.size() - 1;
        constructEts(_time, _wcets);
}

void Task::constructEts(Time const & et, std::vector<int> const & _wcets)
{
        assert(crit + 1 == _wcets.size());
        ets.resize(crit + 1);
        wcets.resize(_wcets.size());
        ets[crit].first = et;
        wcets[crit] = _wcets[crit];
        for (int i = crit - 1; i >= 0; --i) {
                wcets[i] = _wcets[i];
                ets[i+1].first.split(_wcets[i], &ets[i].first, &ets[i].second);
                //ets[i].second.scale(1 / (1 - ets[i].sum()));
                ets[i].second -= _wcets[i];
        }
}


Task::~Task()
{
        for (auto it = jobs.begin(); it != jobs.end(); ++it)
                delete *it;
}

void Task::readTasks(std::istream& is, std::vector<Task*>& tasks)
{
        tasks.clear();
        int num_tasks;
        is >> num_tasks;
        tasks.resize(num_tasks);
        for (int i = 0; i < num_tasks; ++i) {
                tasks[i] = new Task(is);
                //std::cout << tasks[i] << " ";
        }
}

Job* Task::getJob(unsigned int n)
{
        while (n >= jobs.size()) {
                jobs.push_back(new Job(this, jobs.size()));
        }
        return jobs[n];
}


std::ostream& operator << (std::ostream& os, Task const &task)
{
        os << task.name << ' ' << task.period << " C" << task.crit << ' '
           << task.ets[task.crit].first;
        return os;
}
