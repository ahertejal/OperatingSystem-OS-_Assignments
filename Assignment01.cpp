#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <string>
#include <climits>

using namespace std;

struct Process
{
    string pid;
    string taskName;
    int arrivalTime;
    int burstTime;
    int priority;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
    int remainingTime;
};

struct Segment
{
    string pid;
    int start;
    int end;
};

void calculateTimes(vector<Process>& p)
{
    for (auto& x : p)
    {
        x.turnaroundTime = x.completionTime - x.arrivalTime;
        x.waitingTime = x.turnaroundTime - x.burstTime;
    }
}

void displayResults(const vector<Process>& p, const vector<Segment>& gantt, const string& algorithm)
{
    double totalWT = 0;
    double totalTAT = 0;

    cout << "\n============================================================\n";
    cout << "              " << algorithm << " SCHEDULING\n";
    cout << "============================================================\n";

    cout << "\nGantt Chart:\n";

    for (const auto& s : gantt)
    {
        cout << "| " << s.pid << " ";
    }
    cout << "|\n";

    if (!gantt.empty())
    {
        cout << gantt[0].start;
        for (const auto& s : gantt)
        {
            cout << setw(5) << s.end;
        }
        cout << "\n";
    }

    cout << "\n";
    cout << left << setw(10) << "PID"
         << setw(12) << "CT"
         << setw(12) << "TAT"
         << setw(12) << "WT" << "\n";

    cout << "----------------------------------------------\n";

    vector<Process> result = p;

    sort(result.begin(), result.end(), [](const Process& a, const Process& b)
    {
        return a.pid < b.pid;
    });

    for (const auto& x : result)
    {
        cout << left << setw(10) << x.pid
             << setw(12) << x.completionTime
             << setw(12) << x.turnaroundTime
             << setw(12) << x.waitingTime << "\n";

        totalWT += x.waitingTime;
        totalTAT += x.turnaroundTime;
    }

    cout << "\nAverage Waiting Time    : "
         << fixed << setprecision(2)
         << totalWT / p.size() << "\n";

    cout << "Average Turnaround Time : "
         << fixed << setprecision(2)
         << totalTAT / p.size() << "\n";
}

vector<Process> fcfs(vector<Process> p, vector<Segment>& gantt)
{
    sort(p.begin(), p.end(), [](const Process& a, const Process& b)
    {
        if (a.arrivalTime != b.arrivalTime)
            return a.arrivalTime < b.arrivalTime;

        return a.pid < b.pid;
    });

    int currentTime = 0;

    for (auto& x : p)
    {
        if (currentTime < x.arrivalTime)
        {
            currentTime = x.arrivalTime;
        }

        int start = currentTime;
        currentTime += x.burstTime;
        x.completionTime = currentTime;

        gantt.push_back({x.pid, start, currentTime});
    }

    calculateTimes(p);
    return p;
}

vector<Process> sjf(vector<Process> p, vector<Segment>& gantt)
{
    int n = p.size();
    vector<bool> completed(n, false);
    int completedCount = 0;
    int currentTime = 0;

    while (completedCount < n)
    {
        int selected = -1;
        int shortestBurst = INT_MAX;

        for (int i = 0; i < n; i++)
        {
            if (!completed[i] && p[i].arrivalTime <= currentTime)
            {
                if (p[i].burstTime < shortestBurst)
                {
                    shortestBurst = p[i].burstTime;
                    selected = i;
                }
                else if (p[i].burstTime == shortestBurst)
                {
                    if (p[i].arrivalTime < p[selected].arrivalTime)
                    {
                        selected = i;
                    }
                    else if (p[i].arrivalTime == p[selected].arrivalTime &&
                             p[i].pid < p[selected].pid)
                    {
                        selected = i;
                    }
                }
            }
        }

        if (selected == -1)
        {
            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++)
            {
                if (!completed[i])
                    nextArrival = min(nextArrival, p[i].arrivalTime);
            }

            currentTime = nextArrival;
            continue;
        }

        int start = currentTime;
        currentTime += p[selected].burstTime;
        p[selected].completionTime = currentTime;
        completed[selected] = true;
        completedCount++;

        gantt.push_back({p[selected].pid, start, currentTime});
    }

    calculateTimes(p);
    return p;
}

vector<Process> priorityScheduling(vector<Process> p, vector<Segment>& gantt)
{
    int n = p.size();
    vector<bool> completed(n, false);
    int completedCount = 0;
    int currentTime = 0;

    while (completedCount < n)
    {
        int selected = -1;
        int highestPriority = INT_MAX;

        for (int i = 0; i < n; i++)
        {
            if (!completed[i] && p[i].arrivalTime <= currentTime)
            {
                if (p[i].priority < highestPriority)
                {
                    highestPriority = p[i].priority;
                    selected = i;
                }
                else if (p[i].priority == highestPriority)
                {
                    if (p[i].arrivalTime < p[selected].arrivalTime)
                    {
                        selected = i;
                    }
                    else if (p[i].arrivalTime == p[selected].arrivalTime &&
                             p[i].pid < p[selected].pid)
                    {
                        selected = i;
                    }
                }
            }
        }

        if (selected == -1)
        {
            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++)
            {
                if (!completed[i])
                    nextArrival = min(nextArrival, p[i].arrivalTime);
            }

            currentTime = nextArrival;
            continue;
        }

        int start = currentTime;
        currentTime += p[selected].burstTime;
        p[selected].completionTime = currentTime;
        completed[selected] = true;
        completedCount++;

        gantt.push_back({p[selected].pid, start, currentTime});
    }

    calculateTimes(p);
    return p;
}

vector<Process> roundRobin(vector<Process> p, int quantum, vector<Segment>& gantt)
{
    int n = p.size();

    sort(p.begin(), p.end(), [](const Process& a, const Process& b)
    {
        if (a.arrivalTime != b.arrivalTime)
            return a.arrivalTime < b.arrivalTime;

        return a.pid < b.pid;
    });

    for (auto& x : p)
        x.remainingTime = x.burstTime;

    queue<int> readyQueue;

    int currentTime = 0;
    int nextProcess = 0;
    int completedCount = 0;

    while (completedCount < n)
    {
        if (readyQueue.empty())
        {
            if (nextProcess < n && currentTime < p[nextProcess].arrivalTime)
            {
                currentTime = p[nextProcess].arrivalTime;
            }

            while (nextProcess < n &&
                   p[nextProcess].arrivalTime <= currentTime)
            {
                readyQueue.push(nextProcess);
                nextProcess++;
            }
        }

        if (readyQueue.empty())
            continue;

        int index = readyQueue.front();
        readyQueue.pop();

        int start = currentTime;
        int executionTime = min(quantum, p[index].remainingTime);

        currentTime += executionTime;
        p[index].remainingTime -= executionTime;

        gantt.push_back({p[index].pid, start, currentTime});

        while (nextProcess < n &&
               p[nextProcess].arrivalTime <= currentTime)
        {
            readyQueue.push(nextProcess);
            nextProcess++;
        }

        if (p[index].remainingTime > 0)
        {
            readyQueue.push(index);
        }
        else
        {
            p[index].completionTime = currentTime;
            completedCount++;
        }
    }

    calculateTimes(p);
    return p;
}

void displayInput(const vector<Process>& p)
{
    cout << "\n============================================================\n";
    cout << "                 INPUT AI/ML TASK TABLE\n";
    cout << "============================================================\n";

    cout << left << setw(8) << "PID"
         << setw(25) << "AI/ML Task"
         << setw(10) << "AT"
         << setw(10) << "BT"
         << setw(10) << "Priority" << "\n";

    cout << "------------------------------------------------------------\n";

    for (const auto& x : p)
    {
        cout << left << setw(8) << x.pid
             << setw(25) << x.taskName
             << setw(10) << x.arrivalTime
             << setw(10) << x.burstTime
             << setw(10) << x.priority << "\n";
    }
}

void displayComparison(
    const vector<Process>& fcfsResult,
    const vector<Process>& sjfResult,
    const vector<Process>& rrResult,
    const vector<Process>& priorityResult)
{
    auto getAverageWT = [](const vector<Process>& p)
    {
        double total = 0;

        for (const auto& x : p)
            total += x.waitingTime;

        return total / p.size();
    };

    auto getAverageTAT = [](const vector<Process>& p)
    {
        double total = 0;

        for (const auto& x : p)
            total += x.turnaroundTime;

        return total / p.size();
    };

    double fcfsWT = getAverageWT(fcfsResult);
    double sjfWT = getAverageWT(sjfResult);
    double rrWT = getAverageWT(rrResult);
    double priorityWT = getAverageWT(priorityResult);

    double fcfsTAT = getAverageTAT(fcfsResult);
    double sjfTAT = getAverageTAT(sjfResult);
    double rrTAT = getAverageTAT(rrResult);
    double priorityTAT = getAverageTAT(priorityResult);

    cout << "\n============================================================\n";
    cout << "                 ALGORITHM COMPARISON\n";
    cout << "============================================================\n";

    cout << left << setw(22) << "Algorithm"
         << setw(25) << "Average Waiting Time"
         << setw(28) << "Average Turnaround Time" << "\n";

    cout << "---------------------------------------------------------------------\n";

    cout << left << setw(22) << "FCFS"
         << setw(25) << fixed << setprecision(2) << fcfsWT
         << setw(28) << fcfsTAT << "\n";

    cout << left << setw(22) << "SJF"
         << setw(25) << sjfWT
         << setw(28) << sjfTAT << "\n";

    cout << left << setw(22) << "Round Robin"
         << setw(25) << rrWT
         << setw(28) << rrTAT << "\n";

    cout << left << setw(22) << "Priority"
         << setw(25) << priorityWT
         << setw(28) << priorityTAT << "\n";

    double lowestWT = min({fcfsWT, sjfWT, rrWT, priorityWT});
    double lowestTAT = min({fcfsTAT, sjfTAT, rrTAT, priorityTAT});

    string bestWT;
    string bestTAT;

    if (lowestWT == fcfsWT)
        bestWT = "FCFS";
    else if (lowestWT == sjfWT)
        bestWT = "SJF";
    else if (lowestWT == rrWT)
        bestWT = "Round Robin";
    else
        bestWT = "Priority";

    if (lowestTAT == fcfsTAT)
        bestTAT = "FCFS";
    else if (lowestTAT == sjfTAT)
        bestTAT = "SJF";
    else if (lowestTAT == rrTAT)
        bestTAT = "Round Robin";
    else
        bestTAT = "Priority";

    cout << "\nBest Average Waiting Time    : "
         << bestWT << " (" << lowestWT << ")\n";

    cout << "Best Average Turnaround Time : "
         << bestTAT << " (" << lowestTAT << ")\n";

    if (bestWT == bestTAT)
    {
        cout << "\nOverall Result: "
             << bestWT
             << " performs best for this workload based on both average metrics.\n";
    }
    else
    {
        cout << "\nOverall Result: Different algorithms perform best for different metrics.\n";
    }
}

int main()
{
    cout << "============================================================\n";
    cout << "             AI TASK SCHEDULER - CPU SCHEDULING\n";
    cout << "============================================================\n";

    cout << "\nThis simulator compares:\n";
    cout << "1. FCFS Scheduling\n";
    cout << "2. SJF Scheduling\n";
    cout << "3. Round Robin Scheduling\n";
    cout << "4. Priority Scheduling\n";

    int n;

    cout << "\nEnter number of AI/ML tasks: ";
    cin >> n;

    if (n <= 0)
    {
        cout << "\nInvalid number of tasks.\n";
        return 0;
    }

    vector<Process> processes(n);

    cout << "\nEnter task details:\n";

    for (int i = 0; i < n; i++)
    {
        processes[i].pid = "P" + to_string(i + 1);

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "\nEnter AI/ML task name: ";
        getline(cin, processes[i].taskName);

        cout << "Enter Arrival Time: ";
        cin >> processes[i].arrivalTime;

        cout << "Enter Burst Time: ";
        cin >> processes[i].burstTime;

        cout << "Enter Priority: ";
        cin >> processes[i].priority;

        processes[i].completionTime = 0;
        processes[i].turnaroundTime = 0;
        processes[i].waitingTime = 0;
        processes[i].remainingTime = processes[i].burstTime;
    }

    int quantum;

    cout << "\nEnter Round Robin time quantum: ";
    cin >> quantum;

    if (quantum <= 0)
    {
        cout << "\nInvalid time quantum.\n";
        return 0;
    }

    displayInput(processes);

    vector<Segment> fcfsGantt;
    vector<Segment> sjfGantt;
    vector<Segment> rrGantt;
    vector<Segment> priorityGantt;

    vector<Process> fcfsResult = fcfs(processes, fcfsGantt);
    vector<Process> sjfResult = sjf(processes, sjfGantt);
    vector<Process> rrResult = roundRobin(processes, quantum, rrGantt);
    vector<Process> priorityResult = priorityScheduling(processes, priorityGantt);

    displayResults(fcfsResult, fcfsGantt, "FCFS");
    displayResults(sjfResult, sjfGantt, "SJF");
    displayResults(rrResult, rrGantt, "ROUND ROBIN");
    displayResults(priorityResult, priorityGantt, "PRIORITY");

    displayComparison(
        fcfsResult,
        sjfResult,
        rrResult,
        priorityResult
    );

    cout << "\n============================================================\n";
    cout << "                    PROGRAM COMPLETED\n";
    cout << "============================================================\n";

    return 0;
}
