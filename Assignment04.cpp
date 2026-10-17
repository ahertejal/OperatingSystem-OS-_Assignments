#include <iostream>
#include <vector>
#include <string>
using namespace std;

using Matrix = vector<vector<int>>;
using Vec = vector<int>;

struct SafetyResult {
    bool safe;
    Vec sequence;
    Vec unfinished;
};

Matrix calculateNeed(const Matrix& maximum, const Matrix& allocation) {
    Matrix need = maximum;

    for (int i = 0; i < (int)maximum.size(); i++)
        for (int j = 0; j < (int)maximum[i].size(); j++)
            need[i][j] = maximum[i][j] - allocation[i][j];

    return need;
}

SafetyResult bankersSafety(const Matrix& allocation,
                           const Matrix& need,
                           const Vec& available) {
    int n = allocation.size();
    int m = available.size();

    Vec work = available, sequence;
    vector<bool> finish(n, false);

    bool progress = true;

    while ((int)sequence.size() < n && progress) {
        progress = false;

        for (int i = 0; i < n; i++) {
            if (finish[i]) continue;

            bool canFinish = true;

            for (int j = 0; j < m; j++)
                if (need[i][j] > work[j])
                    canFinish = false;

            if (canFinish) {
                for (int j = 0; j < m; j++)
                    work[j] += allocation[i][j];

                finish[i] = true;
                sequence.push_back(i);
                progress = true;
            }
        }
    }

    Vec unfinished;

    for (int i = 0; i < n; i++)
        if (!finish[i])
            unfinished.push_back(i);

    return {(int)sequence.size() == n, sequence, unfinished};
}

void printMatrix(const string& title, const Matrix& matrix,
                 const vector<string>& resources) {
    cout << "\n" << title << ":\n";
    cout << "       ";

    for (const string& name : resources)
        cout << name << "\t";

    cout << "\n";

    for (int i = 0; i < (int)matrix.size(); i++) {
        cout << "P" << i << "     ";

        for (int value : matrix[i])
            cout << value << "\t";

        cout << "\n";
    }
}

void printAvailable(const Vec& available,
                    const vector<string>& resources) {
    cout << "Available: ";

    for (int j = 0; j < (int)available.size(); j++)
        cout << resources[j] << "=" << available[j] << " ";

    cout << "\n";
}

void showSafetyResult(const SafetyResult& result) {
    if (result.safe) {
        cout << "\nSTATE: SAFE\nSafe Sequence: ";

        for (int i = 0; i < (int)result.sequence.size(); i++) {
            cout << "P" << result.sequence[i];

            if (i + 1 < (int)result.sequence.size())
                cout << " -> ";
        }

        cout << "\n";
    } else {
        cout << "\nSTATE: UNSAFE\n";
        cout << "Processes unable to finish: ";

        for (int p : result.unfinished)
            cout << "P" << p << " ";

        cout << "\nNo complete safe sequence exists.\n";
    }
}

void requestResources(int process, const Vec& request,
                      Matrix allocation, Matrix need, Vec available,
                      const vector<string>& resources) {
    cout << "\nP" << process << " requests: ";

    for (int j = 0; j < (int)request.size(); j++)
        cout << resources[j] << "=" << request[j] << " ";

    cout << "\n";

    for (int j = 0; j < (int)request.size(); j++) {
        if (request[j] > need[process][j]) {
            cout << "REJECTED: Request exceeds remaining need.\n";
            return;
        }

        if (request[j] > available[j]) {
            cout << "WAIT: Resources are not available.\n";
            return;
        }
    }

    for (int j = 0; j < (int)request.size(); j++) {
        available[j] -= request[j];
        allocation[process][j] += request[j];
        need[process][j] -= request[j];
    }

    SafetyResult result = bankersSafety(allocation, need, available);

    if (result.safe) {
        cout << "GRANTED: System remains safe.\n";
        showSafetyResult(result);
    } else {
        cout << "REJECTED: Request makes the system unsafe.\n";
        cout << "Original allocation remains unchanged.\n";
        showSafetyResult(result);
    }
}

Vec detectDeadlock(const Matrix& allocation,
                   const Matrix& request,
                   const Vec& available) {
    int n = allocation.size();
    int m = available.size();

    Vec work = available;
    vector<bool> finish(n, false);

    for (int i = 0; i < n; i++) {
        bool holdsResource = false;

        for (int j = 0; j < m; j++)
            if (allocation[i][j] > 0)
                holdsResource = true;

        if (!holdsResource)
            finish[i] = true;
    }

    bool progress = true;

    while (progress) {
        progress = false;

        for (int i = 0; i < n; i++) {
            if (finish[i]) continue;

            bool canProceed = true;

            for (int j = 0; j < m; j++)
                if (request[i][j] > work[j])
                    canProceed = false;

            if (canProceed) {
                for (int j = 0; j < m; j++)
                    work[j] += allocation[i][j];

                finish[i] = true;
                progress = true;
            }
        }
    }

    Vec deadlocked;

    for (int i = 0; i < n; i++)
        if (!finish[i])
            deadlocked.push_back(i);

    return deadlocked;
}

int main() {
    vector<string> resources = {"CPU", "Memory", "GPU"};

    Matrix allocation1 = {
        {0, 1, 0},
        {2, 0, 0},
        {3, 0, 2},
        {2, 1, 1},
        {0, 0, 2}
    };

    Matrix maximum1 = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };

    Vec available1 = {3, 3, 2};
    Matrix need1 = calculateNeed(maximum1, allocation1);

    cout << "========================================\n";
    cout << "RESOURCE ALLOCATION MANAGER\n";
    cout << "Deadlock Detection and Avoidance\n";
    cout << "========================================\n";

    cout << "\nCASE 1: SAFE RESOURCE ALLOCATION\n";

    printMatrix("Allocation Matrix", allocation1, resources);
    printMatrix("Maximum Matrix", maximum1, resources);
    printMatrix("Need Matrix", need1, resources);
    printAvailable(available1, resources);

    showSafetyResult(bankersSafety(allocation1, need1, available1));

    cout << "\nCASE 2: UNSAFE RESOURCE ALLOCATION\n";

    vector<string> resources2 = {"CPU", "Memory"};

    Matrix allocation2 = {
        {1, 0},
        {0, 1},
        {1, 1}
    };

    Matrix maximum2 = {
        {2, 1},
        {1, 2},
        {2, 2}
    };

    Vec available2 = {0, 0};
    Matrix need2 = calculateNeed(maximum2, allocation2);

    printMatrix("Allocation Matrix", allocation2, resources2);
    printMatrix("Maximum Matrix", maximum2, resources2);
    printMatrix("Need Matrix", need2, resources2);
    printAvailable(available2, resources2);

    showSafetyResult(bankersSafety(allocation2, need2, available2));

    cout << "\nCASE 3: SAFE RESOURCE REQUEST\n";

    requestResources(1, {1, 0, 2}, allocation1,
                     need1, available1, resources);

    cout << "\nCASE 4: UNSAFE RESOURCE REQUEST\n";

    requestResources(4, {3, 3, 0}, allocation1,
                     need1, available1, resources);

    cout << "\nCASE 5: ACTUAL DEADLOCK DETECTION\n";

    vector<string> resources3 = {"Resource-A", "Resource-B"};

    Matrix allocation3 = {
        {1, 0},
        {0, 1},
        {0, 0}
    };

    Matrix request3 = {
        {0, 1},
        {1, 0},
        {0, 0}
    };

    Vec available3 = {0, 0};

    printMatrix("Current Allocation", allocation3, resources3);
    printMatrix("Outstanding Request Matrix", request3, resources3);
    printAvailable(available3, resources3);

    Vec deadlocked = detectDeadlock(
        allocation3, request3, available3
    );

    if (deadlocked.empty()) {
        cout << "NO DEADLOCK DETECTED\n";
    } else {
        cout << "DEADLOCK DETECTED\n";
        cout << "Deadlocked Processes: ";

        for (int p : deadlocked)
            cout << "P" << p << " ";

        cout << "\nP0 waits for Resource-B held by P1, "
             << "while P1 waits for Resource-A held by P0.\n";
    }

    return 0;
}
