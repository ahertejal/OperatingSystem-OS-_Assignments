#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <windows.h>

using namespace std;

struct Student
{
    int id;
    int marks;
    int attendance;
};

struct Result
{
    long long totalMarks;
    int passCount;
    int distinctionCount;
    long long totalAttendance;

    Result()
    {
        totalMarks = 0;
        passCount = 0;
        distinctionCount = 0;
        totalAttendance = 0;
    }
};

struct ThreadData
{
    const vector<Student>* students;
    int start;
    int end;
    Result* result;
};

DWORD WINAPI processRange(LPVOID parameter)
{
    ThreadData* data = (ThreadData*)parameter;

    const vector<Student>& students = *(data->students);
    Result& result = *(data->result);

    for (int i = data->start; i < data->end; i++)
    {
        result.totalMarks += students[i].marks;

        if (students[i].marks >= 40)
        {
            result.passCount++;
        }

        if (students[i].marks >= 75)
        {
            result.distinctionCount++;
        }

        result.totalAttendance += students[i].attendance;

        volatile double value = students[i].marks;

        for (int j = 0; j < 1000; j++)
        {
            value = sqrt(value * value + 1.0);
        }
    }

    return 0;
}

Result singleThreadProcessing(const vector<Student>& students)
{
    Result result;

    for (int i = 0; i < (int)students.size(); i++)
    {
        result.totalMarks += students[i].marks;

        if (students[i].marks >= 40)
        {
            result.passCount++;
        }

        if (students[i].marks >= 75)
        {
            result.distinctionCount++;
        }

        result.totalAttendance += students[i].attendance;

        volatile double value = students[i].marks;

        for (int j = 0; j < 1000; j++)
        {
            value = sqrt(value * value + 1.0);
        }
    }

    return result;
}

Result combineResults(const vector<Result>& results)
{
    Result finalResult;

    for (int i = 0; i < (int)results.size(); i++)
    {
        finalResult.totalMarks += results[i].totalMarks;
        finalResult.passCount += results[i].passCount;
        finalResult.distinctionCount += results[i].distinctionCount;
        finalResult.totalAttendance += results[i].totalAttendance;
    }

    return finalResult;
}

void displayResult(const Result& result, int totalRecords)
{
    double averageMarks =
        (double)result.totalMarks / totalRecords;

    double averageAttendance =
        (double)result.totalAttendance / totalRecords;

    cout << "\nTotal Records Processed : "
         << totalRecords << endl;

    cout << "Average Marks           : "
         << fixed << setprecision(2)
         << averageMarks << endl;

    cout << "Passed Students         : "
         << result.passCount << endl;

    cout << "Distinction Students    : "
         << result.distinctionCount << endl;

    cout << "Average Attendance      : "
         << fixed << setprecision(2)
         << averageAttendance << "%" << endl;
}

int main()
{
    const int DATASET_SIZE = 10000;

    cout << "============================================================" << endl;
    cout << "       MULTITHREADED AI DATA PROCESSING SYSTEM" << endl;
    cout << "============================================================" << endl;

    cout << "\nDataset: Student Performance Records" << endl;
    cout << "Dataset Size: " << DATASET_SIZE << endl;

    vector<Student> students(DATASET_SIZE);

    for (int i = 0; i < DATASET_SIZE; i++)
    {
        students[i].id = i + 1;
        students[i].marks = 40 + ((i * 37) % 61);
        students[i].attendance = 60 + ((i * 17) % 41);
    }

    cout << "\nSample Records:" << endl;

    cout << left
         << setw(15) << "Student ID"
         << setw(10) << "Marks"
         << setw(15) << "Attendance"
         << endl;

    cout << "----------------------------------------" << endl;

    for (int i = 0; i < 5; i++)
    {
        cout << left
             << setw(15) << students[i].id
             << setw(10) << students[i].marks
             << setw(15) << students[i].attendance
             << "%" << endl;
    }

    cout << "\n============================================================" << endl;
    cout << "             SINGLE-THREADED PROCESSING" << endl;
    cout << "============================================================" << endl;

    auto singleStart = chrono::high_resolution_clock::now();

    Result singleResult = singleThreadProcessing(students);

    auto singleEnd = chrono::high_resolution_clock::now();

    long long singleTime =
        chrono::duration_cast<chrono::microseconds>(
            singleEnd - singleStart
        ).count();

    displayResult(singleResult, DATASET_SIZE);

    cout << "\nSingle-Thread Execution Time: "
         << singleTime
         << " microseconds" << endl;

    int numberOfThreads;

    cout << "\nEnter number of threads: ";
    cin >> numberOfThreads;

    if (cin.fail() ||
        numberOfThreads <= 0 ||
        numberOfThreads > DATASET_SIZE)
    {
        cout << "\nInvalid number of threads." << endl;
        return 1;
    }

    cout << "\n============================================================" << endl;
    cout << "             MULTITHREADED PROCESSING" << endl;
    cout << "============================================================" << endl;

    cout << "\nNumber of Threads: "
         << numberOfThreads << endl;

    int baseSize = DATASET_SIZE / numberOfThreads;
    int remainder = DATASET_SIZE % numberOfThreads;

    vector<HANDLE> threadHandles(numberOfThreads);
    vector<ThreadData> threadData(numberOfThreads);
    vector<Result> results(numberOfThreads);

    int start = 0;

    cout << "\nWorkload Division:" << endl;

    for (int i = 0; i < numberOfThreads; i++)
    {
        int currentSize = baseSize;

        if (i < remainder)
        {
            currentSize++;
        }

        int end = start + currentSize;

        threadData[i].students = &students;
        threadData[i].start = start;
        threadData[i].end = end;
        threadData[i].result = &results[i];

        cout << "Thread " << i + 1
             << " -> Records "
             << start + 1
             << " to "
             << end
             << " ("
             << currentSize
             << " records)"
             << endl;

        start = end;
    }

    cout << "\nStarting threads..." << endl;

    auto multiStart = chrono::high_resolution_clock::now();

    for (int i = 0; i < numberOfThreads; i++)
    {
        threadHandles[i] = CreateThread(
            NULL,
            0,
            processRange,
            &threadData[i],
            0,
            NULL
        );

        if (threadHandles[i] == NULL)
        {
            cout << "Error creating Thread "
                 << i + 1 << endl;

            return 1;
        }
    }

    WaitForMultipleObjects(
        numberOfThreads,
        &threadHandles[0],
        TRUE,
        INFINITE
    );

    auto multiEnd = chrono::high_resolution_clock::now();

    long long multiTime =
        chrono::duration_cast<chrono::microseconds>(
            multiEnd - multiStart
        ).count();

    for (int i = 0; i < numberOfThreads; i++)
    {
        CloseHandle(threadHandles[i]);
    }

    cout << "\nThread Execution Completed:" << endl;

    for (int i = 0; i < numberOfThreads; i++)
    {
        cout << "Thread " << i + 1
             << " completed | Records processed: "
             << threadData[i].end - threadData[i].start
             << " | Range: "
             << threadData[i].start + 1
             << " - "
             << threadData[i].end
             << endl;
    }

    Result multiResult = combineResults(results);

    displayResult(multiResult, DATASET_SIZE);

    cout << "\nMulti-Thread Execution Time: "
         << multiTime
         << " microseconds" << endl;

    cout << "\n============================================================" << endl;
    cout << "               PERFORMANCE COMPARISON" << endl;
    cout << "============================================================" << endl;

    cout << "\nSingle-Thread Time : "
         << singleTime
         << " microseconds" << endl;

    cout << "Multi-Thread Time  : "
         << multiTime
         << " microseconds" << endl;

    if (singleTime > 0 && multiTime > 0)
    {
        double speedup =
            (double)singleTime / multiTime;

        double performanceChange =
            ((double)(singleTime - multiTime) /
             singleTime) * 100.0;

        cout << "Speedup            : "
             << fixed << setprecision(2)
             << speedup
             << "x" << endl;

        cout << "Performance Change : "
             << fixed << setprecision(2)
             << performanceChange
             << "%" << endl;
    }

    cout << "\n============================================================" << endl;
    cout << "                 RESULT VERIFICATION" << endl;
    cout << "============================================================" << endl;

    bool resultsMatch =
        singleResult.totalMarks == multiResult.totalMarks &&
        singleResult.passCount == multiResult.passCount &&
        singleResult.distinctionCount == multiResult.distinctionCount &&
        singleResult.totalAttendance == multiResult.totalAttendance;

    if (resultsMatch)
    {
        cout << "\nSingle-threaded and multithreaded results match." << endl;
        cout << "Data processing is correct." << endl;
    }
    else
    {
        cout << "\nResult mismatch detected." << endl;
    }

    cout << "\n============================================================" << endl;
    cout << "                  PROGRAM COMPLETED" << endl;
    cout << "============================================================" << endl;

    return 0;
}
