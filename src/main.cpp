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

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> dis(1000, 9999);

#include <stack>

template<size_t N>
void search_orbit_iterative(
        Cycle<N> const& parent_cycle,
        std::unordered_set<Cycle<N>, typename Cycle<N>::Hash, typename Cycle<N>::Equal> &visited,
        std::unordered_map<Frame<N>, size_t, typename Frame<N>::Hash> &visited_frames,
        std::vector<Frame<N>> &cycle_frames,
        GameOfLife<N> &game,
        Frame<N> &frame)
{
    std::stack<Cycle<N>> cycleStack;
    cycleStack.push(parent_cycle);

    while (!cycleStack.empty()) {
        auto currentCycle = cycleStack.top();
        cycleStack.pop();

        for (const auto& org_frame : currentCycle.frames()) {
            for (size_t i = 0; i < Frame<N>::CellCount; ++i) {
                frame = org_frame;
                frame.toggle(i);
                game.set(frame);
                auto cycle = game.find_cycle(visited_frames, cycle_frames);

                if (visited.find(cycle) != visited.end())
                    continue;

                visited.insert(cycle);
                cycleStack.push(cycle);
            }
        }

        cout << "Cycle stack size:" << cycleStack.size() << '\n';
    }
}

template<size_t N>
std::unordered_set<Cycle<N>, typename Cycle<N>::Hash, typename Cycle<N>::Equal> search_square_orbit()
{
    std::unordered_set<Cycle<N>, typename Cycle<N>::Hash, typename Cycle<N>::Equal> cache;
    Frame<N> square_frame((0b11ull << N) | 0b11ull);
    Cycle<N> square_cycle(std::vector<Frame<N>>{ square_frame });

    std::unordered_map<Frame<N>, size_t, typename Frame<N>::Hash> visited_frames;
    std::vector<Frame<N>> cycle_frames;
    GameOfLife<N> game;

    std::stack<Cycle<N>> cycleStack;
    cycleStack.push(square_cycle);

    cache.reserve(1000); // Adjust size accordingly

    while (!cycleStack.empty()) {
        auto currentCycle = cycleStack.top();
        cycleStack.pop();

        for (const auto& org_frame : currentCycle.frames()) {
            for (size_t i = 0; i < Frame<N>::CellCount; ++i) {
                Frame<N> frame = org_frame; // Local copy within loop scope
                frame.toggle(i);
                game.set(frame);
                auto cycle = game.find_cycle(visited_frames, cycle_frames);

                auto [iter, inserted] = cache.insert(cycle); // Insert and check insertion

                if (!inserted)
                    continue;

                cycleStack.push(cycle);
            }
        }

        //cout << "Cycle stack size:" << cycleStack.size() << '\n';
    }

    cache.insert(square_cycle);
    return cache;
}

int generate_random_id()
{
    return dis(gen);
}

template <size_t N>
unordered_map<Cycle<N>, size_t, typename Cycle<N>::Hash, typename Cycle<N>::Equal> assign_indices(
    unordered_set<Cycle<N>, typename Cycle<N>::Hash, typename Cycle<N>::Equal> const& cycles)
{
    auto null_cycle = find_if(cycles.begin(), cycles.end(), [](Cycle<N> const &cycle) {
        Frame<N> zero(0);
        auto const& frames = cycle.frames();
        return frames.contains(zero);
    });

    unordered_map<Cycle<N>, size_t, typename Cycle<N>::Hash, typename Cycle<N>::Equal> indices;

    indices[*null_cycle] = 0;

    size_t cycle_index_assignment_index = 1;
    for (const auto &cycle: cycles) {
        if (typename Cycle<N>::Equal()(cycle, *null_cycle))
            continue;

        indices[cycle] = cycle_index_assignment_index++;
    }

    return indices;
}

template <size_t N>
void write_cycle_data(
    unordered_set<Cycle<N>, typename Cycle<N>::Hash, typename Cycle<N>::Equal> const& cycles,
    unordered_map<Cycle<N>, size_t, typename Cycle<N>::Hash, typename Cycle<N>::Equal> const& indices)
{
    auto file_name = std::format("{}x{}-configurations-{}.txt", N, N, generate_random_id());
    ofstream os(file_name);

    if (!os.is_open())
    {
        cout << "Could not open file: " << file_name << '\n';
        return;
    }

    for (const auto &cycle: cycles)
    {
        os << "[T = " << cycle.frames().size() << ", id: " << indices.at(cycle) << "]\n";

        for (const auto &frame: cycle.frames())
        {
            os << frame.get() << ' ';
        }

        os << '\n' << cycle << '\n';
    }

    os.close();
}

template <size_t N>
void write_matrix_data(
    unordered_set<Cycle<N>, typename Cycle<N>::Hash, typename Cycle<N>::Equal> const& cycles,
    unordered_map<Cycle<N>, size_t, typename Cycle<N>::Hash, typename Cycle<N>::Equal> const& indices
)
{
    auto file_name = std::format("{}x{}-matrix-{}.txt", N, N, generate_random_id());
    ofstream os(file_name);

    GameOfLife<N> game;
    unordered_map<Frame<N>, size_t, typename Frame<N>::Hash> visited_frames;
    vector<Frame<N>> cycle_frames;

    Cycle<N> const null_cycle;
    vector<Cycle<N>> cycles_as_vector(cycles.size(), null_cycle);

    for(auto const& cycle : cycles)
    {
        const auto id = indices.at(cycle);
        cycles_as_vector[id] = cycle;
    }

    // This loop iterates by index, which means, we don't need to mark what cycle
    // each row will correspond to as the row number - 1 and the index are the same
    for (auto const& cycle: cycles_as_vector) {

        unordered_map<Cycle<N>, size_t, typename Cycle<N>::Hash, typename Cycle<N>::Equal> dest_cycles;

        for (auto const& org_frame: cycle.frames()) {
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

        size_t freq = 0;

        const Index row = indices.at(cycle);

        // i'th element is the frequency at which current cycle ends up in cycle with index i.
        vector<int> destination_frequencies(cycles.size(), 0);

        auto const n = static_cast<int64_t>(cycle.frames().size() * Frame<N>::CellCount);

        // subtract n from diagonal
        destination_frequencies[row] -= n;

        // Here we iterate in a shuffled manner, but we want that element i in this row
        // would correspond to frequency where destination is cycle with index i
        for (const auto &[dest_cycle, dest_freq]: dest_cycles) {
            const Index col = indices.at(dest_cycle);
            destination_frequencies[col] += dest_freq;
        }

        size_t zero_counter = 0;

        for(auto const& frequency : destination_frequencies)
        {
            auto const scaled = static_cast<int64_t>(frequency * N * N);

            if(0 == scaled)
            {
                ++zero_counter;
            }
            else
            {
                // reset the zero counter and print zeros

                if(zero_counter > 0)
                {
                    if(1 == zero_counter)
                    {
                        os << 0 << ' ';
                    }
                    else
                    {
                        os << 0 << '$' << zero_counter << ' ';
                    }

                    zero_counter = 0;
                }

                if(0 == scaled % n)
                {
                    os << scaled / n << ' ';
                }
                else
                {
                    int d = gcd(scaled, n);
                    char buf[20];
                    sprintf(buf, "%lli/%lli", scaled / d, n / d);
                    os << buf << ' ';
                }
            }
        }

        if(zero_counter > 0)
        {
            if(1 == zero_counter)
            {
                os << 0 << ' ';
            }
            else
            {
                os << 0 << '$' << zero_counter << ' ';
            }

            zero_counter = 0;
        }

        //os << "[sum=" << std::accumulate(destination_frequencies.begin(), destination_frequencies.end(), 0) << "]\n";
        os << '\n';
    }

    os.close();
}

void main_flow()
{
    constexpr size_t N = 5;

    auto start = std::chrono::steady_clock::now();
    auto cycles = search_square_orbit<N>();
    cout << "Elapsed(ms)=" << since(start).count() << ", cycles found: " << cycles.size() << '\n';
    GameOfLife<N> game;
    const auto indices = assign_indices(cycles);
    write_cycle_data(cycles, indices);
    write_matrix_data(cycles, indices);

//    random_device rd;
//    mt19937 gen(rd());
//    uniform_int_distribution<> dis(1000, 9999);
//    string filename = "s" + to_string(N) + "-" + to_string(dis(gen)) + ".txt";
//    ofstream os(filename);
//
//    if (!os.is_open()) {
//        cout << "Could not output input file: " << filename << '\n';
//        return;
//    }
//
//    // place empty space in the front
//    auto null_cycle = find_if(cycles.begin(), cycles.end(), [](Cycle<N> const &cycle) {
//        Frame<N> zero(0);
//        auto const& frames = cycle.frames();
//        return frames.contains(zero);
//    });
//
//    unordered_map<Cycle<N>, size_t, Cycle<N>::Hash, Cycle<N>::Equal> cycle_indices;
//
//    cycle_indices[*null_cycle] = 0;
//
//    size_t cycle_index_assignment_index = 1;
//    for (const auto &cycle: cycles) {
//        if (Cycle<N>::Equal()(cycle, *null_cycle))
//            continue;
//
//        cycle_indices[cycle] = cycle_index_assignment_index++;
//    }
//
//    /// Cycles are stored in a vector in an order specified by cycle_indices
//    vector<Cycle<N>> cycles_vec(cycles.size(), *null_cycle);
//    for (auto const &c: cycles) {
//        cycles_vec[cycle_indices[c]] = c;
//    }
//
//
//    for (const auto &cycle: cycles) {
//        os << "[T = " << cycle.frames().size() << ", id: " << cycle_indices[cycle] << "]\n";
//        for (const auto &frame: cycle.frames())
//            os << frame.get() << ' ';
//        os << '\n' << cycle << '\n';
//    }
//
//    unordered_map<Frame<N>, size_t, Frame<N>::Hash> visited_frames;
//    vector<Frame<N>> cycle_frames;
//
//    MatrixXd matrix_divided(cycles.size(), cycles.size());
//    matrix_divided.setZero();
//
//    MatrixXi matrix(cycles.size(), cycles.size());
//    matrix.setZero();
//
//    for (const auto &cycle: cycles) {
//        unordered_map<Cycle<N>, size_t, Cycle<N>::Hash, Cycle<N>::Equal> dest_cycles;
//
//        for (const auto &org_frame: cycle.frames()) {
//            for (size_t i = 0; i < Frame<N>::CellCount; ++i) {
//                Frame<N> frame = org_frame;
//                frame.toggle(i);
//                game.set(frame);
//                auto dest_cycle = game.find_cycle(visited_frames, cycle_frames);
//
//                if (dest_cycles.contains(dest_cycle))
//                    dest_cycles[dest_cycle]++;
//                else
//                    dest_cycles[dest_cycle] = 1;
//            }
//        }
//
//        for (const auto &[dest_cycle, dest_freq]: dest_cycles) {
//            const Index row = cycle_indices[dest_cycle];
//            const Index col = cycle_indices[cycle];
//            const auto n = static_cast<double>(cycle.frames().size() * Frame<N>::CellCount);
//            matrix(row, col) += dest_freq;// / n;
//            matrix_divided(row, col) += dest_freq / n;
//        }
//    }
//
////    for (int i = 0; i < matrix.rows(); ++i)
////    {
////        for (int j = 0; j < matrix.cols(); ++j)
////        {
////            const auto n = cycles_vec[j].frames().size() * Frame<N>::CellCount;
////            matrix(i, j) /= n;
////        }
////    }
//
//    //os << "Divide immediately\n";
//    //os << N * N * (matrix_divided - MatrixXd::Identity(matrix_divided.rows(), matrix_divided.cols())) << '\n';
//    //os << "Divide after\n";
//    //os << N * N * (matrix - MatrixXi::Identity(matrix_divided.rows(), matrix_divided.cols()))<< '\n';;
//
//    //matrix -= MatrixXi::Identity(matrix.rows(), matrix.cols());
//    //matrix *= N * N;
//    //EigenSolver<MatrixXd> solver(matrix);
//
//    //os << solver.eigenvalues() << endl;
//
//    for(Index i = 0; i < matrix.rows(); ++i)
//    {
//        for(Index j = 0; j < matrix.cols(); ++j)
//        {
//            const int n = cycles_vec[j].frames().size() * Frame<N>::CellCount;
//
//            // we're on the diagonal
//            if(i == j)
//                matrix(i, j) -= n;
//
//            // scale each entry
//            matrix(i, j) *= N * N;
//
//            if(0 == matrix(i, j) || 0 == matrix(i, j) % n)
//            {
//                os << setw(10) <<  matrix(i, j) / n;
//            }
//            else
//            {
//                int d = gcd(matrix(i, j), n);
//                char buf[20];
//                sprintf(buf, "%d/%d", matrix(i,j) / d, n / d);
//                os << setw(10) << buf;
//            }
//        }
//        os << '\n';
//    }
//    os << "Col Sums: " << matrix.colwise().sum() << '\n';
//    os << "Row Sums: " << matrix.rowwise().sum().transpose() << '\n';
//    os.close();
}

int main(int argc, char** argv) {
    main_flow();
    return 0;
}