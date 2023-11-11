#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <climits>
#include <chrono>
#include <bit>

using namespace std;

constexpr static size_t width = 4, height = 4;

constexpr static size_t to_index(size_t row, size_t col)
{
    return col + row * width;
}

constexpr void set_bit(size_t row, size_t col, uint64_t &input)
{
    input |= (1 << to_index(row, col));
}

static_assert(width <= 8 && height <= 8, "Width & height must both be smaller or equal to 8.");

constexpr static size_t total = width * height;
constexpr static array<uint64_t, total> neighbour_mask_lookup = { []() constexpr 
{
    array<uint64_t, total> table {};
    for (size_t row = 0; row < height; ++row)
    {
        array<size_t, 3> y = { (height + row - 1) % height, row, (row + 1) % height };

        for (size_t col = 0; col < width; ++col)
        {
            array<size_t, 3> x = { (width + col - 1) % width, col, (col + 1) % width };

            size_t index = to_index(row, col);

            for(size_t i = 0; i < 3; ++i)
            {
                for(size_t j = 0; j < 3; ++j)
                {
                    if(i == 1 && j == 1)
                        continue;

                    set_bit(y[i], x[j], table[index]);
                }
            }
        }
    }
    return table;
}() };

constexpr static bool alive_lookup[2][8] =
{
    { false, false, false, true, false, false, false, false },
    { false, false, true , true, false, false, false, false }
};

static size_t sFrame = 0;
static uint64_t sBoard;
static unordered_map<size_t, size_t> sVisited;
static unordered_map<size_t, size_t> sFrequencies;

void print_board(uint64_t board);
constexpr uint64_t next_gen(uint64_t u0);
size_t find_cycle();
void find_cycles();

int main(int argc, char** argv)
{
    auto start = chrono::steady_clock::now();
    find_cycles();
    auto end = chrono::steady_clock::now();

    auto ms = chrono::duration <double, milli> (end - start).count();
    
    cout << "Program took: " << ms << " ms" << endl;
    cout << "Board size: " << width << " x " << height << '\n'; 
    cout << "Total configurations searched: " << (1 << (total)) << endl;

    for (const auto& [period, count] : sFrequencies)
        cout << count << " configurations converge to a cycle of " << period << " frames\n";
}

void find_cycles()
{
    for(uint64_t i = 0; i < (1 << total); ++i)
    {
        sBoard = i;
        size_t period = find_cycle();

        if(sFrequencies.count(period) > 0)
            ++sFrequencies[period];
        else
            sFrequencies[period] = 1;
    }
}

void print_board(uint64_t board)
{
    cout << "frame: " << sFrame << '\n';
    for(int i = 0; i < width * height; ++i)
    {
        char c = (board & (1 << i)) == 0 ? '.' : '#';
        cout << c << ' ';
        if((1 + i) % width == 0)
            cout << '\n';
    }
    cout << '\n';
}

size_t find_cycle()
{
    sVisited.clear();
    sFrame = 0;
    for(;;)
    {
        sBoard = next_gen(sBoard);

        if(sVisited.count(sBoard) > 0)
            return sFrame - sVisited[sBoard];
    
        sVisited[sBoard] = sFrame++;
    }
}

constexpr uint64_t next_gen(uint64_t u0)
{
    uint64_t u1 = 0;
    for(size_t i = 0; i < width * height; ++i)
    {
        uint64_t mask = neighbour_mask_lookup[i];
        size_t n = __popcount(mask & u0);
        bool alive = (u0 & (1 << i)) != 0;
        u1 |= alive_lookup[alive][n] << i;
    }
    return u1;
}