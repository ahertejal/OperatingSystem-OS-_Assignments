#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
using namespace std;

struct Result {
    int hits = 0;
    int faults = 0;
};

void displayFrames(vector<int> frames) {
    for (int page : frames) {
        if (page == -1)
            cout << "-\t";
        else
            cout << page << "\t";
    }
}

void displayStats(string name, Result r, int total) {
    double hitRatio = (double)r.hits / total;
    double faultRatio = (double)r.faults / total;

    cout << "\n" << name << " RESULTS\n";
    cout << "Total References : " << total << endl;
    cout << "Page Hits        : " << r.hits << endl;
    cout << "Page Faults      : " << r.faults << endl;
    cout << fixed << setprecision(2);
    cout << "Hit Ratio        : " << hitRatio
         << " (" << hitRatio * 100 << "%)" << endl;
    cout << "Fault Ratio      : " << faultRatio
         << " (" << faultRatio * 100 << "%)" << endl;
}

// 1. FIFO PAGE REPLACEMENT
Result FIFO(vector<int> pages, int capacity) {
    vector<int> frames(capacity, -1);
    int pointer = 0;
    Result r;

    cout << "\n========== FIFO ALGORITHM ==========\n";
    cout << "Page\tFrames\t\tStatus\n";

    for (int page : pages) {
        bool found = false;

        for (int i = 0; i < capacity; i++) {
            if (frames[i] == page) {
                found = true;
                break;
            }
        }

        if (found) {
            r.hits++;
            cout << page << "\t";
            displayFrames(frames);
            cout << "\tHIT\n";
        }
        else {
            r.faults++;

            bool inserted = false;
            for (int i = 0; i < capacity; i++) {
                if (frames[i] == -1) {
                    frames[i] = page;
                    inserted = true;
                    break;
                }
            }

            if (!inserted) {
                frames[pointer] = page;
                pointer = (pointer + 1) % capacity;
            }

            cout << page << "\t";
            displayFrames(frames);
            cout << "\tFAULT\n";
        }
    }

    displayStats("FIFO", r, pages.size());
    return r;
}

// 2. LRU PAGE REPLACEMENT
Result LRU(vector<int> pages, int capacity) {
    vector<int> frames(capacity, -1);
    vector<int> lastUsed(capacity, -1);
    Result r;

    cout << "\n========== LRU ALGORITHM ==========\n";
    cout << "Page\tFrames\t\tStatus\n";

    for (int t = 0; t < (int)pages.size(); t++) {
        int page = pages[t];
        int index = -1;

        for (int i = 0; i < capacity; i++) {
            if (frames[i] == page) {
                index = i;
                break;
            }
        }

        if (index != -1) {
            r.hits++;
            lastUsed[index] = t;

            cout << page << "\t";
            displayFrames(frames);
            cout << "\tHIT\n";
        }
        else {
            r.faults++;

            int replace = -1;

            for (int i = 0; i < capacity; i++) {
                if (frames[i] == -1) {
                    replace = i;
                    break;
                }
            }

            if (replace == -1) {
                replace = 0;

                for (int i = 1; i < capacity; i++) {
                    if (lastUsed[i] < lastUsed[replace])
                        replace = i;
                }
            }

            frames[replace] = page;
            lastUsed[replace] = t;

            cout << page << "\t";
            displayFrames(frames);
            cout << "\tFAULT\n";
        }
    }

    displayStats("LRU", r, pages.size());
    return r;
}

// 3. OPTIMAL PAGE REPLACEMENT
Result Optimal(vector<int> pages, int capacity) {
    vector<int> frames(capacity, -1);
    Result r;

    cout << "\n========== OPTIMAL ALGORITHM ==========\n";
    cout << "Page\tFrames\t\tStatus\n";

    for (int t = 0; t < (int)pages.size(); t++) {
        int page = pages[t];
        bool found = false;

        for (int i = 0; i < capacity; i++) {
            if (frames[i] == page) {
                found = true;
                break;
            }
        }

        if (found) {
            r.hits++;

            cout << page << "\t";
            displayFrames(frames);
            cout << "\tHIT\n";
        }
        else {
            r.faults++;

            int replace = -1;

            for (int i = 0; i < capacity; i++) {
                if (frames[i] == -1) {
                    replace = i;
                    break;
                }
            }

            if (replace == -1) {
                int farthest = -1;

                for (int i = 0; i < capacity; i++) {
                    int nextUse = -1;

                    for (int j = t + 1;
                         j < (int)pages.size(); j++) {
                        if (pages[j] == frames[i]) {
                            nextUse = j;
                            break;
                        }
                    }

                    if (nextUse == -1) {
                        replace = i;
                        break;
                    }

                    if (nextUse > farthest) {
                        farthest = nextUse;
                        replace = i;
                    }
                }
            }

            frames[replace] = page;

            cout << page << "\t";
            displayFrames(frames);
            cout << "\tFAULT\n";
        }
    }

    displayStats("OPTIMAL", r, pages.size());
    return r;
}

// MAIN FUNCTION
int main() {
    vector<int> pages;
    int capacity;
    string input;

    cout << "============================================\n";
    cout << " VIRTUAL MEMORY & PAGE REPLACEMENT SIMULATOR\n";
    cout << "============================================\n";

    cout << "\nEnter page reference string: ";
    getline(cin, input);

    stringstream ss(input);
    int page;

    while (ss >> page) {
        if (page < 0) {
            cout << "Error: Page numbers cannot be negative.\n";
            return 1;
        }
        pages.push_back(page);
    }

    if (pages.empty()) {
        cout << "Error: Invalid or empty reference string.\n";
        return 1;
    }

    cout << "Enter number of physical frames: ";
    if (!(cin >> capacity) || capacity <= 0) {
        cout << "Error: Frame count must be positive.\n";
        return 1;
    }

    cout << "\nPage Reference String: ";
    for (int p : pages)
        cout << p << " ";

    cout << "\nNumber of Frames: " << capacity << "\n";

    Result f = FIFO(pages, capacity);
    Result l = LRU(pages, capacity);
    Result o = Optimal(pages, capacity);

    // FINAL COMPARISON
    cout << "\n============================================\n";
    cout << "         FINAL PERFORMANCE COMPARISON\n";
    cout << "============================================\n";

    cout << left << setw(15) << "Algorithm"
         << setw(15) << "Page Faults"
         << setw(15) << "Page Hits" << endl;

    cout << setw(15) << "FIFO"
         << setw(15) << f.faults
         << setw(15) << f.hits << endl;

    cout << setw(15) << "LRU"
         << setw(15) << l.faults
         << setw(15) << l.hits << endl;

    cout << setw(15) << "Optimal"
         << setw(15) << o.faults
         << setw(15) << o.hits << endl;

    int minimum = min({f.faults, l.faults, o.faults});

    cout << "\nBest Performing Algorithm(s): ";

    if (f.faults == minimum) cout << "FIFO ";
    if (l.faults == minimum) cout << "LRU ";
    if (o.faults == minimum) cout << "Optimal ";

    cout << "\n============================================\n";

    return 0;
}
