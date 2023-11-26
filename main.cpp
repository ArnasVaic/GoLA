#include <chrono>
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
    constexpr size_t total = 1 << s * s;

    GameOfLife<s> game;

    unordered_set<Cycle<s>, Cycle<s>::Hash, Cycle<s>::Equal> cycles;
    
    auto start = steady_clock::now();

    std::unordered_map<Frame<s>, size_t, Frame<s>::Hash> visited_frames;

    for(uint64_t i = 0; i < total; ++i)
    {
        game.set(i);

        visited_frames.insert({ game.frame(), game.generation() });
    
        for(;;)
        {
            game.evolve();
            Frame<s> current_frame = game.frame();

            if(visited_frames.contains(current_frame))
            {
                size_t cycle_begin_generation = visited_frames[current_frame];
                std::vector<Frame<s>> cycle_frames;

                for (const auto& [frame, generation] : visited_frames) {
                    //cout << "generation: " << generation << '\n' << frame << '\n';
                    if(generation >= cycle_begin_generation)
                    {
                        cycle_frames.push_back(frame);
                    }
                }

                Cycle<s> cycle(cycle_frames);
                cycles.insert(cycle);

                visited_frames.clear();

                break;
            }
            else
            {
                visited_frames[current_frame] = game.generation();
            }
        }    
    }

    auto end = steady_clock::now();
    auto ms = duration<double, milli>(end - start).count();
    
    cout << "Program took " << ms << "ms, unique cycles: " << cycles.size() << "\n";

    for (const auto &cycle : cycles)
    {
        cout << "T = " << cycle.frames().size() << '\n';
        for(const auto &frame : cycle.frames())
        {
            cout << frame << '\n';
        }

        cout << '\n';
    }

}