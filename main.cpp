#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <climits>
#include <chrono>
#include <bit>

using namespace std;

void print_board(
    uint64_t board,
    size_t width,
    size_t height);

constexpr int neighbour_count(
    size_t row,
    size_t col,
    uint64_t board,
    size_t width,
    size_t height);

constexpr uint64_t next_gen(
    uint64_t current,
    size_t width,
    size_t height);

void find_cycle(
    uint64_t board,
    size_t width,
    size_t height,
    size_t &cycle_start,
    size_t &cycle_end);

void initialize_buffer(
    uint8_t *buffer1,
    size_t width,
    size_t height,
    uint64_t configuration)
{
    for(uint64_t i = 0; i < width * height; ++i)
    {
        buffer1[i] = configuration & (1 << i) ? 1 : 0;
    }
}

void find_cycles(
    size_t width,
    size_t height,
    unordered_map<size_t, size_t> &frequencies
);

int main(int argc, char** argv)
{
    unordered_map<size_t, size_t> frequencies;
    size_t w = 4, h = 4;

    auto start = chrono::steady_clock::now();
    find_cycles(w, h, frequencies);
    auto end = chrono::steady_clock::now();

    auto ms = chrono::duration <double, milli> (end - start).count();
    
    cout << "Program took: " << ms << " ms" << endl;
    cout << "Board size: " << w << " x " << h << '\n'; 
    cout << "Total configurations searched: " << (1 << (w * h)) << endl;

    for (const auto& [period, count] : frequencies)
        cout << count << " configurations converge to a cycle of " << period << " frames\n";

    // size_t width = 4, height = 4, start = 0, end = 0;
    
    // cout << "frame: " << 0 << endl;
    // print_board(64512, width, height);
    // cout << endl;

    // find_cycle(64512, width, height, start, end);
}

void find_cycles(
    size_t width,
    size_t height,
    unordered_map<size_t, size_t> &frequencies)
{
    for(uint64_t i = 0; i < 1 << (width * height); ++i)
    {
        size_t start = 0, end = 0;
        find_cycle(i, width, height, start, end);
        size_t period = end - start;

        // if(period == 4)
        // {
        //     initialize_buffer(buffer1, width, height, i);
        //     cout << "Configuration: " << i << endl;
        //     print_board(buffer1, width, height);
        //     cout << endl;
        // }

        if(frequencies.count(period) > 0)
        {
            ++frequencies[period];
        }
        else
        {
            frequencies[period] = 1;
        }
    }
}

void print_board(
    uint64_t board,
    size_t width,
    size_t height)
{
    for(int i = 0; i < height; ++i)
    {
        for(int j = 0; j < width; ++j)
        {
            size_t index = j + i * width;
            uint64_t alive = board & (1 << index);
            printf("%c ", alive ? '#' : '.'); 
        }
        cout << endl;
    }  
}

void find_cycle(
    uint64_t initial_state,
    size_t width,
    size_t height,
    size_t &cycle_start,
    size_t &cycle_end)
{
    size_t frame = 0;
    std::unordered_map<uint64_t, uint64_t> state_dict;
    uint64_t current_state = initial_state;
    for(;;)
    {
        current_state = next_gen(current_state, width, height);

        if(state_dict.count(current_state) > 0)
        {
            cycle_start = state_dict[current_state];
            cycle_end = frame;
            return;
        }
        else
        {
            state_dict[current_state] = frame;
        }

        ++frame;

        // cout << "frame: " << frame << endl;
        // print_board(current_state, width, height);
        // cout << endl;
    }
}

constexpr bool alive_lookup[2][8] =
{
    { false, false, false, true, false, false, false, false },
    { false, false, true , true, false, false, false, false }
};

constexpr uint64_t next_gen(
    uint64_t current,
    size_t width,
    size_t height)
{
    uint64_t state = 0;

    for(size_t i = 0; i < height; ++i)
    {
        for(size_t j = 0; j < width; ++j)
        {
            size_t n = neighbour_count(i, j, current, width, height);

            // printf("%lu ", n);

            size_t index = j + i * width; 
            size_t old_alive = (current & (1 << index)) != 0;
            size_t new_alive = alive_lookup[old_alive][n];
            state |= new_alive << index;
        }
        // cout << '\n';
    }

    // cout << '\n';

    return state;
}

constexpr int neighbour_count(
    size_t row,
    size_t col,
    uint64_t board,
    size_t width,
    size_t height)
{
    size_t shifted_col = width + col;
    size_t xl = (shifted_col - 1) % width;
    size_t xm = (shifted_col + 0) % width;
    size_t xr = (shifted_col + 1) % width;

    size_t shifted_row = height + row;
    size_t yt = ((shifted_row - 1) % height) * width;
    size_t ym = ((shifted_row + 0) % height) * width;
    size_t yb = ((shifted_row + 1) % height) * width;

    uint64_t mask = 0;
    mask |= (1 << (xl + yt));
    mask |= (1 << (xm + yt));
    mask |= (1 << (xr + yt));
    mask |= (1 << (xl + ym));

    mask |= (1 << (xr + ym));
    mask |= (1 << (xl + yb));
    mask |= (1 << (xm + yb));
    mask |= (1 << (xr + yb));

    return __popcount(board & mask);
}