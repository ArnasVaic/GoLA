#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <climits>
#include <chrono>
#include <unordered_set>
#include <bit>
#include <board.h>

using namespace std;

int main(int argc, char** argv)
{
    constexpr size_t w = 4, h = 4;
    Board<w, h> board;
    unordered_set<uint64_t> stable;
    unordered_map<size_t, size_t> frequencies;

    auto start = chrono::steady_clock::now();
    board.find_cycles(stable, frequencies);
    auto end = chrono::steady_clock::now();
    auto ms = chrono::duration <double, milli> (end - start).count();
    
    cout << "Program took: " << ms << " ms" << endl;
    cout << "Board size: " << board.Width << " x " << board.Height << '\n'; 
    cout << "Total configurations searched: " << (1 << (board.Total)) << endl;

    for (const auto& [period, count] : frequencies)
        cout << count << " configurations converge to a cycle of " << period << " frames\n";

    cout << "Unique 1-period states: " << stable.size() << '\n';

    for (const auto& state : stable)
        cout << state;
}