#include <chrono>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <frame.hpp>
#include <cycle.hpp>
#include <game_of_life.hpp>

using namespace std;
using namespace chrono;

int main(int argc, char** argv)
{
    constexpr size_t s = 4;
    GameOfLife<s> game;
    unordered_set<Cycle<s>, Cycle<s>::Hash, Cycle<s>::Equal> cycles;
    
    auto start = steady_clock::now();

    for(uint64_t i = 0; i < (1 << Frame<s>::CellCount); ++i)
    {
        game.set(i);
        cycles.insert(game.find_cycle());   
    }

    auto end = steady_clock::now();
    auto ms = duration<double, milli>(end - start).count();
    

    unordered_map<Cycle<s>, size_t, Cycle<s>::Hash, Cycle<s>::Equal> cycle_indices;


    size_t cycle_index_asignment_index = 0;
    for(const auto &cycle : cycles)
    {
        cycle_indices[cycle] = cycle_index_asignment_index++;
    }

    cout << "Program took " << ms << "ms, unique cycles: " << cycles.size() << "\n";

    for (const auto &cycle : cycles)
    {
        cout << "[T = " << cycle.frames().size() << ", id: " << cycle_indices[cycle] <<  "]\n";
        cout << cycle << '\n';
    }

    vector<double> p_matrix(cycles.size() * cycles.size(), 0);

    for(const auto& cycle : cycles)
    {
        unordered_map<Cycle<s>, size_t, Cycle<s>::Hash, Cycle<s>::Equal> dest_cycles;

        for(const auto& org_frame : cycle.frames())
        {
            for(size_t i = 0; i < Frame<s>::CellCount; ++i)
            {
                Frame<s> frame = org_frame;
                frame.toggle(i);
                game.set(frame);
                auto c = game.find_cycle();

                if(dest_cycles.contains(c))
                {
                    dest_cycles[c]++;
                }
                else
                {
                    dest_cycles[c] = 1;
                }

            }
        }

        for(const auto& dest : dest_cycles)
        {
            size_t index_in_p_matrix = cycle_indices[dest.first] + cycle_indices[cycle] * cycles.size();

            // dest.second is the number of perturbated configurations that converged
            // to the cycle dest.first
            double m = dest.second;
            double n = cycle.frames().size() * Frame<s>::CellCount;
            p_matrix[index_in_p_matrix] += m / n;
        }
    }

    for(size_t i = 0; i < cycles.size(); ++i)
    {
        for(size_t j = 0; j < cycles.size(); ++j)
        {
            printf("%4.8f ", p_matrix[j + i * cycles.size()]);
        }
        cout << '\n';
    }
    cout << '\n';
}