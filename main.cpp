#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <climits>
#include <chrono>

using namespace std;

void print_board(
    uint8_t *board,
    size_t width,
    size_t height);

int neighbour_count(
    size_t row,
    size_t col,
    uint8_t *board,
    size_t width,
    size_t height);

uint64_t next_gen(
    uint8_t *current,
    uint8_t *next,
    size_t width,
    size_t height);

void find_cycle(
    uint8_t *buffer1,
    uint8_t *buffer2,
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
    // uint8_t *buffer1 = new uint8_t[width * height];
    // uint8_t *buffer2 = new uint8_t[width * height];
    // initialize_buffer(buffer1, width, height, 64512);

    // cout << "frame: " << 0 << endl;
    // print_board(buffer1, width, height);
    // cout << endl;

    // find_cycle(buffer1, buffer2, width, height, start, end);
}

void find_cycles(
    size_t width,
    size_t height,
    unordered_map<size_t, size_t> &frequencies)
{
    uint8_t *buffer1 = new uint8_t[width * height];
    uint8_t *buffer2 = new uint8_t[width * height];
    
    for(uint64_t i = 0; i < 1 << (width * height); ++i)
    {
        size_t start = 0, end = 0;

        initialize_buffer(buffer1, width, height, i);
        find_cycle(buffer1, buffer2, width, height, start, end);
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

    delete [] buffer1;
    delete [] buffer2;
}

void print_board(
    uint8_t *board,
    size_t width,
    size_t height)
{
    for(int i = 0; i < height; ++i)
    {
        for(int j = 0; j < width; ++j)
        {
            printf("%c ", board[j + i * width] == 1 ? '#' : '.'); 
        }
        cout << endl;
    }  
}

void find_cycle(
    uint8_t *buffer1,
    uint8_t *buffer2,
    size_t width,
    size_t height,
    size_t &cycle_start,
    size_t &cycle_end)
{
    bool turn = true;
    size_t frame = 0;
    std::unordered_map<uint64_t, uint64_t> state_dict;
    for(;;)
    {
        uint8_t *from = turn ? buffer1 : buffer2;
        uint8_t *to   = turn ? buffer2 : buffer1;
        uint64_t s    = next_gen(from, to, width, height);

        if(state_dict.count(s) > 0)
        {
            cycle_start = state_dict[s];
            cycle_end = frame;
            return;
        }
        else
        {
            state_dict[s] = frame;
        }

        turn = !turn;
        ++frame;

        // cout << "frame: " << frame << endl;
        // print_board(to, width, height);
        // cout << endl;
    }
}

uint64_t next_gen(
    uint8_t *current,
    uint8_t *next,
    size_t width,
    size_t height)
{
    uint64_t state = 0;
    for(size_t i = 0; i < height; ++i)
    {
        for(size_t j = 0; j < width; ++j)
        {
            size_t n = neighbour_count(i % height, j % width, current, width, height);
            size_t index = j + i * width; 
            next[index] = (n == 3 || (n == 2 && current[index]));

            if(next[index] == 1)
                state |= 1 << index;
        }
    }
    return state;
}

int neighbour_count(
    size_t row,
    size_t col,
    uint8_t *board,
    size_t width,
    size_t height)
{

    int xl = (width + col - 1) % width;
    int xm = (width + col + 0) % width;
    int xr = (width + col + 1) % width;

    int yt = (height + row - 1) % height;
    int ym = (height + row + 0) % height;
    int yb = (height + row + 1) % height;

    return 
        board[xl + yt * width] + board[xm + yt * width] + board[xr + yt * width] +
        board[xl + ym * width]                          + board[xr + ym * width] +
        board[xl + yb * width] + board[xm + yb * width] + board[xr + yb * width];
}