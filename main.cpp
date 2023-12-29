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

using namespace std;
using namespace chrono;
using namespace Eigen;

template<size_t N>
using CycleSet = std::unordered_set<Cycle<N>, typename Cycle<N>::Hash, typename Cycle<N>::Equal>;

template<size_t N>
void search_orbit_recursive(Cycle<N> const& parent_cycle, CycleSet<N> &visited)
{
    auto cycles = GameOfLife<N>::search_perturbed(parent_cycle);

    for(auto const& cycle : cycles)
    {
        if(visited.contains(cycle))
            continue;

        visited.insert(cycle);
        search_orbit_recursive(cycle, visited);
    }
}

template<size_t N>
CycleSet<N> search_square_orbit()
{
    CycleSet<N> cache;
    Frame<N> square_frame((0b11ull << N) | 0b11ull);
    Cycle<N> square_cycle(vector<Frame<N>>{ square_frame });
    search_orbit_recursive<N>(square_cycle, cache);
    cache.insert(square_cycle);
    return cache;
}

int main(int argc, char** argv)
{

    constexpr size_t N = 4;
    auto cycles = search_square_orbit<N>();
//    cout << "Size of closed orbit: " << cycles.size() << '\n';
//    for(auto const& cycle : cycles)
//    {
//        cout << cycle << '\n';
//    }


//    constexpr size_t s = 6;
    GameOfLife<N> game;
//    auto cycles = game.find_cycles(65536, 100);
//    cycles = game.search_perturbed(cycles);
//
    cout << "Cycles found: " << cycles.size() << '\n';

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1000, 9999);
    string filename = "s" + to_string(N) + "-" + to_string(dis(gen)) + ".txt";
    ofstream os(filename);

    if(!os.is_open())
    {
        cout << "Could not output input file: " << filename << '\n';
        return 0;
    }

    unordered_map<Cycle<N>, size_t, Cycle<N>::Hash, Cycle<N>::Equal> cycle_indices;
    size_t cycle_index_assignment_index = 0;
    for(const auto &cycle : cycles)
        cycle_indices[cycle] = cycle_index_assignment_index++;

    for (const auto &cycle : cycles)
    {
        os << "[T = " << cycle.frames().size() << ", id: " << cycle_indices[cycle] << "]\n";
        for(const auto &frame : cycle.frames())
            os << frame.get() << ' ';
        os << '\n' << cycle << '\n';
    }

    unordered_map<Frame<N>, size_t, Frame<N>::Hash> visited_frames;
    vector<Frame<N>> cycle_frames;

    MatrixXd matrix(cycles.size(), cycles.size());
    matrix.setZero();

    for(const auto& cycle : cycles)
    {
        unordered_map<Cycle<N>, size_t, Cycle<N>::Hash, Cycle<N>::Equal> dest_cycles;

        for(const auto& org_frame : cycle.frames())
        {
            for(size_t i = 0; i < Frame<N>::CellCount; ++i)
            {
                Frame<N> frame = org_frame;
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
            auto n = static_cast<double>(cycle.frames().size() * Frame<N>::CellCount);
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