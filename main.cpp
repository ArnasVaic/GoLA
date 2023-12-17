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
    constexpr size_t s = 6;
    GameOfLife<s> game;

//    game.set(Frame<s>(6831));
//
//    // Reuse containers to avoid instantiation.
//
//    // Used as a lookup to check if frame has been visited
//    std::unordered_map<Frame<s>, size_t, Frame<s>::Hash> visited_frames;
//
//    // Cycle frames are accumulated.
//    std::vector<Frame<s>> cycle_frames;
//
//    auto c = game.find_cycle(visited_frames, cycle_frames);
//
//    cout << c << '\n';

    auto cycles = game.find_cycles(512, 10);
    //cycles = game.search_perturbed(cycles);

    cout << "Cycles found: " << cycles.size() << '\n';

    auto start = steady_clock::now();
    auto end = steady_clock::now();
    auto ms = duration<double, milli>(end - start).count();

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

    unordered_map<Cycle<s>, size_t, Cycle<s>::Hash, Cycle<s>::Equal> cycle_indices;
    size_t cycle_index_assignment_index = 0;
    for(const auto &cycle : cycles)
    {
        cycle_indices[cycle] = cycle_index_assignment_index++;
    }

    for (const auto &cycle : cycles)
    {
        os << "[T = " << cycle.frames().size() << ", id: " << cycle_indices[cycle] << "]\n";

        for(const auto &frame : cycle.frames())
        {
            os << frame.get() << ' ';
        }

        os << '\n' << cycle << '\n';
    }

//    vector<double> p_matrix(cycles.size() * cycles.size(), 0);
//    // Reuse containers to avoid instantiation.
//    // Used as a lookup to check if frame has been visited
//    std::unordered_map<Frame<s>, size_t, Frame<s>::Hash> visited_frames;
//    // Cycle frames are accumulated.
//    std::vector<Frame<s>> cycle_frames;
//
//    for(const auto& cycle : cycles)
//    {
//        unordered_map<Cycle<s>, size_t, Cycle<s>::Hash, Cycle<s>::Equal> dest_cycles;
//
//        for(const auto& org_frame : cycle.frames())
//        {
//            for(size_t i = 0; i < Frame<s>::CellCount; ++i)
//            {
//                Frame<s> frame = org_frame;
//                frame.toggle(i);
//                game.set(frame);
//                auto c = game.find_cycle(visited_frames, cycle_frames);
//
//                if(dest_cycles.contains(c))
//                    dest_cycles[c]++;
//                else
//                    dest_cycles[c] = 1;
//            }
//        }
//
//        for(const auto& [dest_cycle, dest_frequency] : dest_cycles)
//        {
//            if(cycle_indices.contains())
//            size_t index_in_p_matrix = cycle_indices[cycle] + cycle_indices[cycle] * cycles.size();
//            double m = dest_frequency;
//            double n = dest_cycle.frames().size() * Frame<s>::CellCount;
//            p_matrix[index_in_p_matrix] += m / n;
//        }
//    }
//
//    for(size_t i = 0; i < cycles.size(); ++i)
//    {
//        for(size_t j = 0; j < cycles.size(); ++j)
//        {
//            os << setw(9) << setprecision(8) << fixed << p_matrix[j + i * cycles.size()] << ' ';
//        }
//        os << '\n';
//    }
//    os << '\n';
//
//    MatrixXd matrix(cycles.size(), cycles.size());
//
//    for(size_t i = 0; i < p_matrix.size(); ++i)
//    {
//        matrix(i / cycles.size(), i % cycles.size()) = p_matrix[i];
//    }
//
//    EigenSolver<MatrixXd> solver(matrix);
//    os << solver.eigenvalues() << endl;
//
//    os.close();
}