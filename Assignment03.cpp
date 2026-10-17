#include <windows.h>
#include <iostream>
#include <iomanip>
using namespace std;

const int INITIAL_BALANCE = 1000;
const int AMOUNT = 100;
const int THREAD_COUNT = 12;
const int DEPOSIT_COUNT = 6;

int balance = INITIAL_BALANCE;
CRITICAL_SECTION balanceLock;
CRITICAL_SECTION outputLock;

bool useMutex = false;
volatile LONG readCount = 0;
volatile LONG depositCount = 0;

HANDLE allReadEvent;
HANDLE depositsDoneEvent;

struct Transaction {
    int id;
    bool deposit;
    int amount;
};

DWORD WINAPI performTransaction(LPVOID arg) {
    Transaction* t = (Transaction*)arg;
    int oldBalance, newBalance;

    if (useMutex) {
        // Synchronised transaction
        EnterCriticalSection(&balanceLock);

        oldBalance = balance;
        if (t->deposit)
            newBalance = oldBalance + t->amount;
        else
            newBalance = oldBalance - t->amount;

        balance = newBalance;

        LeaveCriticalSection(&balanceLock);
    }
    else {

        EnterCriticalSection(&balanceLock);
        oldBalance = balance;
        LeaveCriticalSection(&balanceLock);

        if (InterlockedIncrement(&readCount) == THREAD_COUNT)
            SetEvent(allReadEvent);

        WaitForSingleObject(allReadEvent, INFINITE);

        if (!t->deposit)
            WaitForSingleObject(depositsDoneEvent, INFINITE);

        if (t->deposit)
            newBalance = oldBalance + t->amount;
        else
            newBalance = oldBalance - t->amount;

        EnterCriticalSection(&balanceLock);
        balance = newBalance;
        LeaveCriticalSection(&balanceLock);

        if (t->deposit &&
            InterlockedIncrement(&depositCount) == DEPOSIT_COUNT)
            SetEvent(depositsDoneEvent);
    }

    EnterCriticalSection(&outputLock);
    cout << "T" << setw(2) << setfill('0') << t->id
         << setfill(' ')
         << (t->deposit ? " DEPOSIT " : " WITHDRAW")
         << " | Read: Rs." << oldBalance
         << " | Wrote: Rs." << newBalance << endl;
    LeaveCriticalSection(&outputLock);

    return 0;
}

void runExperiment(bool synchronized) {
    balance = INITIAL_BALANCE;
    useMutex = synchronized;
    readCount = 0;
    depositCount = 0;

    allReadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    depositsDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    Transaction transactions[THREAD_COUNT];
    HANDLE threads[THREAD_COUNT];

    cout << "\n========================================\n";
    cout << (synchronized
        ? "EXPERIMENT 2: WITH SYNCHRONISATION\n"
        : "EXPERIMENT 1: WITHOUT SYNCHRONISATION\n");
    cout << "========================================\n";

    cout << "Initial Balance: Rs." << INITIAL_BALANCE << endl;
    cout << "Transactions: 6 deposits and 6 withdrawals"
         << " of Rs." << AMOUNT << endl;
    cout << "\nTransaction Log:\n";

    for (int i = 0; i < THREAD_COUNT; i++) {
        transactions[i].id = i + 1;
        transactions[i].deposit = (i < DEPOSIT_COUNT);
        transactions[i].amount = AMOUNT;

        threads[i] = CreateThread(
            NULL, 0, performTransaction,
            &transactions[i], 0, NULL
        );

        if (threads[i] == NULL) {
            cerr << "Error creating thread.\n";
            ExitProcess(1);
        }
    }

    WaitForMultipleObjects(THREAD_COUNT, threads, TRUE, INFINITE);

    for (int i = 0; i < THREAD_COUNT; i++)
        CloseHandle(threads[i]);

    cout << "\nExpected Balance: Rs." << INITIAL_BALANCE << endl;
    cout << "Actual Balance:   Rs." << balance << endl;

    if (balance == INITIAL_BALANCE)
        cout << "Result: CONSISTENT\n";
    else
        cout << "Result: INCONSISTENT - Lost Updates\n";

    CloseHandle(allReadEvent);
    CloseHandle(depositsDoneEvent);
}

int main() {
    InitializeCriticalSection(&balanceLock);
    InitializeCriticalSection(&outputLock);

    cout << "CONCURRENT BANKING SYSTEM\n";
    cout << "Race Condition and Synchronisation\n";

    runExperiment(false);
    runExperiment(true);

    DeleteCriticalSection(&outputLock);
    DeleteCriticalSection(&balanceLock);

    return 0;
}
