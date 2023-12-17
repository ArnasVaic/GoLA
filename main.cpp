#include <chrono>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <fstream>
#include <frame.hpp>
#include <cycle.hpp>
#include <game_of_life.hpp>
#include <Eigen/Dense>
#include <random>
#include <iomanip>

using namespace std;
using namespace chrono;
using namespace Eigen;

int main(int argc, char** argv)
{
    constexpr size_t s = 4;
    GameOfLife<s> game;
    unordered_set<Cycle<s>, Cycle<s>::Hash, Cycle<s>::Equal> cycles;
    unordered_map<Frame<s>, size_t, Frame<s>::Hash> visited_frames;
    vector<Frame<s>> cycle_frames;

    auto start = steady_clock::now();

    for(uint64_t i = 0; i < (1ull << Frame<s>::CellCount); ++i)
    {
        game.set(Frame<s>(i));
        const auto cycle = game.find_cycle(visited_frames, cycle_frames);
        cycles.insert(cycle);
    }

    auto end = steady_clock::now();
    auto ms = duration<double, milli>(end - start).count();
    

    unordered_map<Cycle<s>, size_t, Cycle<s>::Hash, Cycle<s>::Equal> cycle_indices;


    size_t cycle_index_assignment_index = 0;
    for(const auto &cycle : cycles)
    {
        cycle_indices[cycle] = cycle_index_assignment_index++;
    }

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1000, 9999);

    string filename = "s" + to_string(s) + "-" + to_string(dis(gen)) + ".txt";

    ofstream os(filename);

    if(!os.is_open())
    {
        cout << "Could not output input file: " << filename << '\n';
        return 0;
    }

    os << "Program took " << ms << "ms, unique cycles: " << cycles.size() << "\n";

    for (const auto &cycle : cycles)
    {
        os << "[T = " << cycle.frames().size() << ", id: " << cycle_indices[cycle] << "]\n";

        for(const auto &frame : cycle.frames())
        {
            os << frame.get() << ' ';
        }

        os << '\n' << cycle << '\n';
    }

    MatrixXd matrix(cycles.size(), cycles.size());
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
                auto dest_cycle = game.find_cycle(visited_frames, cycle_frames);

                if(dest_cycles.contains(dest_cycle))
                    dest_cycles[dest_cycle]++;
                else
                    dest_cycles[dest_cycle] = 1;
            }
        }

        for(const auto& [dest_cycle, dest_freq] : dest_cycles)
        {
            size_t row = cycle_indices[dest_cycle];
            size_t col = cycle_indices[cycle];
            auto m = static_cast<double>(dest_freq);
            auto n = static_cast<double>(cycle.frames().size() * Frame<s>::CellCount);
            matrix(row, col) += m / n;
        }
    }
    EigenSolver<MatrixXd> solver(matrix);
    os << solver.eigenvalues() << endl;
    os << matrix << "\n\n";
    os << "Col Sums:\n" << matrix.colwise().sum() << "\n\n";
    os << "Row Sums:\n" << matrix.rowwise().sum() << "\n";
    os.close();
}