#include <game.hpp>

#include <chrono>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <fstream>
#include <Eigen/Dense>
#include <random>

using namespace std;
using namespace chrono;
using namespace Eigen;

template <size_t N>
using cycle_ids = unordered_map<Cycle<N>, size_t, typename Cycle<N>::Hash, typename Cycle<N>::Equal>;

template <
    class result_t   = std::chrono::milliseconds,
    class clock_t    = std::chrono::steady_clock,
    class duration_t = std::chrono::milliseconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}


int main(int argc, char** argv) {
    constexpr size_t N = 5;
    Game<N> game;

    Frame<N> square_frame((0b11ull << N) | 0b11ull);
    Cycle<N> square_cycle(vector<Frame<N>>{ square_frame });

    auto start = std::chrono::steady_clock::now();
    auto cycles = game.search_orbit(square_cycle);
    cout << "Elapsed(ms)=" << since(start).count() << '\n';

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
    auto null_cycle = find_if(
        cycles.begin(),
        cycles.end(),
        [](Cycle<N> const &cycle) { return cycle.frames().contains(0); }
    );

    cycle_ids<N> cycle_indices;
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
            os << frame.state() << ' ';
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
            for (size_t i = 0; i < Frame<N>::cell_count; ++i) {
                Frame<N> frame = org_frame;
                frame.toggle(i);
                game.reset(frame);
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
            const auto n = static_cast<double>(cycle.frames().size() * Frame<N>::cell_count);
            matrix(row, col) += dest_freq;// / n;
            matrix_divided(row, col) += dest_freq / n;
        }
    }

    for(Index i = 0; i < matrix.rows(); ++i)
    {
        for(Index j = 0; j < matrix.cols(); ++j)
        {
            const int n = cycles_vec[j].frames().size() * Frame<N>::cell_count;

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