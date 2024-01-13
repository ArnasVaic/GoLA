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

template <
    class result_t   = std::chrono::milliseconds,
    class clock_t    = std::chrono::steady_clock,
    class duration_t = std::chrono::milliseconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

template<size_t N>
using CycleSet = std::unordered_set<Cycle<N>, typename Cycle<N>::Hash, typename Cycle<N>::Equal>;

template<size_t N>
void search_orbit_recursive(
        Cycle<N> const& parent_cycle,
        CycleSet<N> &visited,
        std::unordered_map<Frame<N>, size_t, typename Frame<N>::Hash> &visited_frames,
        std::vector<Frame<N>> &cycle_frames,
        GameOfLife<N> &game,
        Frame<N> &frame)
{
    for(const auto& org_frame : parent_cycle.frames())
    {
        for(size_t i = 0; i < Frame<N>::CellCount; ++i)
        {
            frame = org_frame;
            frame.toggle(i);
            game.set(frame);
            auto cycle = game.find_cycle(visited_frames, cycle_frames);

            if(visited.contains(cycle))
                continue;

            visited.insert(cycle);

            search_orbit_recursive<N>(
                cycle,
                visited,
                visited_frames,
                cycle_frames,
                game,
                frame);
        }
    }
}

template<size_t N>
CycleSet<N> search_square_orbit()
{
    CycleSet<N> cache;
    Frame<N> square_frame((0b11ull << N) | 0b11ull);
    Cycle<N> square_cycle(vector<Frame<N>>{ square_frame });

    std::unordered_map<Frame<N>, size_t, typename Frame<N>::Hash> visited_frames;
    std::vector<Frame<N>> cycle_frames;
    GameOfLife<N> game;
    Frame<N> frame;

    search_orbit_recursive<N>(
            square_cycle,
            cache,
            visited_frames,
            cycle_frames,
            game,
            frame);

    cache.insert(square_cycle);
    return cache;
}

int main(int argc, char** argv) {

    constexpr size_t N = 8;

    auto start = std::chrono::steady_clock::now();
    auto cycles = search_square_orbit<N>();
    cout << "Elapsed(ms)=" << since(start).count() << '\n';
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

    if (!os.is_open()) {
        cout << "Could not output input file: " << filename << '\n';
        return 0;
    }

    // place empty space in the front
    auto null_cycle = find_if(cycles.begin(), cycles.end(), [](Cycle<N> const &cycle) {
        return cycle.frames().contains(0);
    });

    unordered_map<Cycle<N>, size_t, Cycle<N>::Hash, Cycle<N>::Equal> cycle_indices;

    cycle_indices[*null_cycle] = 0;

    size_t cycle_index_assignment_index = 1;
    for (const auto &cycle: cycles) {
        if (Cycle<N>::Equal()(cycle, *null_cycle))
            continue;

        cycle_indices[cycle] = cycle_index_assignment_index++;
    }

    /// Cycles are stored in a vector in an order specified by cycle_indices
    vector<Cycle<N>> cycles_vec(cycles.size(), *null_cycle);
    for (auto const &c: cycles) {
        cycles_vec[cycle_indices[c]] = c;
    }


    for (const auto &cycle: cycles) {
        os << "[T = " << cycle.frames().size() << ", id: " << cycle_indices[cycle] << "]\n";
        for (const auto &frame: cycle.frames())
            os << frame.get() << ' ';
        os << '\n' << cycle << '\n';
    }

    unordered_map<Frame<N>, size_t, Frame<N>::Hash> visited_frames;
    vector<Frame<N>> cycle_frames;

    MatrixXd matrix_divided(cycles.size(), cycles.size());
    matrix_divided.setZero();

    MatrixXi matrix(cycles.size(), cycles.size());
    matrix.setZero();

    for (const auto &cycle: cycles) {
        unordered_map<Cycle<N>, size_t, Cycle<N>::Hash, Cycle<N>::Equal> dest_cycles;

        for (const auto &org_frame: cycle.frames()) {
            for (size_t i = 0; i < Frame<N>::CellCount; ++i) {
                Frame<N> frame = org_frame;
                frame.toggle(i);
                game.set(frame);
                auto dest_cycle = game.find_cycle(visited_frames, cycle_frames);

                if (dest_cycles.contains(dest_cycle))
                    dest_cycles[dest_cycle]++;
                else
                    dest_cycles[dest_cycle] = 1;
            }
        }

        for (const auto &[dest_cycle, dest_freq]: dest_cycles) {
            const Index row = cycle_indices[dest_cycle];
            const Index col = cycle_indices[cycle];
            const auto n = static_cast<double>(cycle.frames().size() * Frame<N>::CellCount);
            matrix(row, col) += dest_freq;// / n;
            matrix_divided(row, col) += dest_freq / n;
        }
    }

//    for (int i = 0; i < matrix.rows(); ++i)
//    {
//        for (int j = 0; j < matrix.cols(); ++j)
//        {
//            const auto n = cycles_vec[j].frames().size() * Frame<N>::CellCount;
//            matrix(i, j) /= n;
//        }
//    }

    //os << "Divide immediately\n";
    //os << N * N * (matrix_divided - MatrixXd::Identity(matrix_divided.rows(), matrix_divided.cols())) << '\n';
    //os << "Divide after\n";
    //os << N * N * (matrix - MatrixXi::Identity(matrix_divided.rows(), matrix_divided.cols()))<< '\n';;

    //matrix -= MatrixXi::Identity(matrix.rows(), matrix.cols());
    //matrix *= N * N;
    //EigenSolver<MatrixXd> solver(matrix);

    //os << solver.eigenvalues() << endl;

    for(Index i = 0; i < matrix.rows(); ++i)
    {
        for(Index j = 0; j < matrix.cols(); ++j)
        {
            const int n = cycles_vec[j].frames().size() * Frame<N>::CellCount;

            // we're on the diagonal
            if(i == j)
                matrix(i, j) -= n;

            // scale each entry
            matrix(i, j) *= N * N;

            if(0 == matrix(i, j) || 0 == matrix(i, j) % n)
            {
                os << setw(10) <<  matrix(i, j) / n;
            }
            else
            {
                int d = gcd(matrix(i, j), n);
                char buf[20];
                sprintf(buf, "%d/%d", matrix(i,j) / d, n / d);
                os << setw(10) << buf;
            }
        }
        os << '\n';
    }
    os << "Col Sums: " << matrix.colwise().sum() << '\n';
    os << "Row Sums: " << matrix.rowwise().sum().transpose() << '\n';
    os.close();
}