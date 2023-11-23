#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <climits>
#include <chrono>
#include <unordered_set>
#include <bit>
#include <bitset>
#include <board.hpp>

using namespace std;
using namespace chrono;

int main(int argc, char** argv)
{
    constexpr size_t len = 4;
    Board<len> board;
    
    auto start = steady_clock::now();
    auto cycles = board.find_unique_non_zero_cycles();
    auto end = steady_clock::now();
    auto ms = duration<double, milli>(end - start).count();
    
    cout << "total cycles:" << cycles.size() << '\n';

    Board<len> temp;
    for (const auto& cycle : cycles)
    {
        cout << "Cycle\n";

        for(size_t j = 0; j < cycle.size(); ++j)
        {
            temp.set(cycle[j]);
            //cout << bitset<temp.Total>(state) << '\n';
            cout << temp << '\n';
        }
    }
}