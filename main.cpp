#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <climits>
#include <chrono>
#include <unordered_set>
#include <bit>
#include <bitset>
#include <board.h>

using namespace std;

int main(int argc, char** argv)
{
    constexpr size_t w = 4, h = 4;
    Board<w,h> board;
    
    auto start = chrono::steady_clock::now();
    auto cycles = board.find_cycles();
    auto end = chrono::steady_clock::now();
    auto ms = chrono::duration <double, milli> (end - start).count();
    
    cout << "Program took: " << ms << " ms" << endl;
    cout << "Board size: " << board.Width << " x " << board.Height << '\n'; 
    cout << "Total configurations searched: " << (1 << (board.Total)) << endl;

    for (const auto& [period, count] : frequencies)
        cout << count << " configurations converge to a cycle of " << period << " frames\n";

    for (auto it = stable.begin(); it != stable.end();) {
        uint64_t currentElement = *it;
        bool predicateSatisfied = false;

        // Check if there exists another element b such that P(a, b) is satisfied
        for (uint64_t b : stable) {
            if (Board<w,h>::is_equivalent(currentElement, b) && b != currentElement) {
                predicateSatisfied = true;
                break;
            }
        }

        // If the predicate is satisfied, erase the current element
        if (predicateSatisfied) {
            it = stable.erase(it);
        } else {
            ++it;
        }
    }
    
    cout << "Unique 1-period states: " << stable.size() << '\n';
    Board<w,h> temp;
    for (const auto& state : stable)
    {
        temp.set(state);
        cout << bitset<temp.Total>(state) << '\n';
        cout << temp << '\n';
    }

    //cout << BoardCompare<4, 4>()(0b0010000110000100, 0b0010010010000001) << '\n';
}