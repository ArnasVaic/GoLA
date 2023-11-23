#include <cstdint>
#include <assert.h>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <climits>
#include <chrono>
#include <unordered_set>
#include <bit>
#include <set>
#include <bitset>
#include <board.hpp>
#include <cycle.hpp>
#include <frame.hpp>

using namespace std;
using namespace chrono;

int main(int argc, char** argv)
{
    // Cycle<4> c(std::vector<Frame<4>>
    // {
    //     Frame<4>(0b1000100010001000)
    // });
    Transform t;
    Frame<4> frame(0b1110110000000000);
    Frame<4> normalized_frame = frame.normalized(t);

    cout << "Original(" << bitset<16>(frame.get()) << "):\n" << frame << '\n';
    cout << "Normalized(" << bitset<16>(normalized_frame.get()) << "):\n" << normalized_frame << '\n';

    assert(normalized_frame.get() != 0);
    assert(t.col_offset == 0);
    assert(t.row_offset == 0);
    assert(!t.flip);
    assert(t.turn_count == 0);

    cout << "Row offset: " << t.row_offset << '\n';
    cout << "Col offset: " << t.col_offset << '\n';
    cout << "Turn count: " << t.turn_count << '\n';
    cout << "Flip: " << (t.flip ? "true" : "false") << '\n';

    // constexpr size_t len = 4;

    // Frame<len> f = 0;

    // for(int i = 0; i < 64; ++i)
    // {
    //     if(i % 2 == 0)
    //         f.set(i);
    // }

    // std::set<Frame<4>> s;
    // s.insert(Frame<4>(0));

    // Board<len> board;

    // auto start = steady_clock::now();
    // auto cycles = board.find_unique_non_zero_cycles();
    // cycles.insert(std::vector<uint64_t>{0});
    // auto end = steady_clock::now();
    // auto ms = duration<double, milli>(end - start).count();
    
    // cout << "total cycles:" << cycles.size() << '\n';

    // vector<uint64_t> c1
    // {
    //     0b0101011001000001,
    //     0b0110001100100101,
    //     0b0000001100101010,
    //     0b1000001001101010,
    //     0b1000001001101010,
    //     0b1010010011000110,
    //     0b1011001110000010,
    //     0b0101010011000000
    // };

    // vector<uint64_t> c2
    // {
    //     0b0100011101100001,
    //     0b0000101010001001,
    //     0b0010101011001000,
    //     0b1011100000101001,
    //     0b1010110001100100,
    //     0b0101000001100100,
    //     0b1100010110001001,
    //     0b0101000101001100
    // };

    // Board<len> temp;
    // for (const auto& cycle : vector<vector<uint64_t>>{c1, c2})
    // {
    //     cout << "Cycle\n";

    //     for(size_t j = 0; j < cycle.size(); ++j)
    //     {
    //         temp.set(cycle[j]);
    //         //cout << bitset<temp.Total>(state) << '\n';
    //         cout << temp << '\n';
    //     }
    // }

    // cout << VectorU64Equal<4>()(c1, c2) << '\n';

    // unordered_set<vector<uint64_t>, VectorU64Hash, VectorU64Equal<len>> s;

    // s.insert(c1);

    // cout << s.contains(c1) << ' ' << s.contains(c2) << '\n';

    // s.insert(c2);

    // cout << s.size() << '\n';

    // Board<len> temp;
    // for (const auto& cycle : cycles)
    // {
    //     cout << "Cycle\n";

    //     for(size_t j = 0; j < cycle.size(); ++j)
    //     {
    //         auto frame = cycle[j];
    //         temp.set(frame);
    //         cout << bitset<temp.Total>(frame) << '\n';
    //         cout << temp << '\n';
    //     }
    // }
}