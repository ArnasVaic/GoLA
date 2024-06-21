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
#include <stack>
#include "process_rates_and_decays.hpp"

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
    unordered_map<Cycle<N>, size_t, typename Cycle<N>::Hash, typename Cycle<N>::Equal> dest_cycles;
    vector<Frame<N>> cycle_frames;

    Cycle<N> const null_cycle;
    vector<Cycle<N>> cycles_as_vector(cycles.size(), null_cycle);
    vector<int> destination_frequencies(cycles.size(), 0);

    for(auto const& cycle : cycles)
    {
        const auto id = indices.at(cycle);
        cycles_as_vector[id] = cycle;
    }

    // This loop iterates by index, which means, we don't need to mark what cycle
    // each row will correspond to as the row number - 1 and the index are the same
    for (auto const& cycle: cycles_as_vector) {
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

        os << "Cycle[" << indices.at(cycle) << "] has " << dest_cycles.size() << "unique destination cycles.\n";

        for (auto const& [dest_cycle, dest_freq] : dest_cycles)
        {
            os << indices.at(dest_cycle) << ' ';
        }

        size_t freq = 0;

        const Index row = indices.at(cycle);

        // i'th element is the frequency at which current cycle ends up in cycle with index i.
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

            os << scaled << ' ';

//            if(0 == scaled)
//            {
//                ++zero_counter;
//            }
//            else
//            {
//                // reset the zero counter and print zeros
//
//                if(zero_counter > 0)
//                {
//                    if(1 == zero_counter)
//                    {
//                        os << 0 << ' ';
//                    }
//                    else
//                    {
//                        os << 0 << '$' << zero_counter << ' ';
//                    }
//
//                    zero_counter = 0;
//                }
//
//                if(0 == scaled % n)
//                {
//                    os << scaled / n << ' ';
//                }
//                else
//                {
//                    int d = gcd(scaled, n);
//                    char buf[20];
//                    sprintf(buf, "%lli/%lli", scaled / d, n / d);
//                    os << buf << ' ';
//                }
//            }
        }

//        if(zero_counter > 0)
//        {
//            if(1 == zero_counter)
//            {
//                os << 0 << ' ';
//            }
//            else
//            {
//                os << 0 << '$' << zero_counter << ' ';
//            }
//
//            zero_counter = 0;
//        }
        os << '\n';

        destination_frequencies.clear();
        dest_cycles.clear();
    }

    os.close();
}

/// @brief This write is only relevant for 5x5 torus. Together with cycle animations there
/// should be displayed frames where each cell shows the id of a cycle that will be reached
/// if said cell was to be perturbed for the respective frame configuration of the cycle.
/// @param cycles
/// @param indices
void write_5x5(
    unordered_set<Cycle<5>, typename Cycle<5>::Hash, typename Cycle<5>::Equal> const& cycles,
    unordered_map<Cycle<5>, size_t, typename Cycle<5>::Hash, typename Cycle<5>::Equal> const& indices)
{

    const string filename = "5x5-destination-frames.txt";
    ofstream os(filename);

    unordered_map<Frame<5>, size_t, typename Frame<5>::Hash> visited_frames;
    vector<Frame<5>> cycle_frames;

    GameOfLife<5> game;

    if (!os.is_open())
    {
        cout << "Could not open file: " << filename << '\n';
        return;
    }

    for (const auto &cycle: cycles)
    {
        os << "[T = " << cycle.frames().size() << ", id: " << indices.at(cycle) << "]\n";

        for (const auto &frame: cycle.frames())
        {
            os << frame.get() << ' ';
        }

        os << '\n';

        // perturbed destination indices
        size_t pdi[cycle.frames().size()][5][5];

        // Find indices of perturbed cycle frames
        size_t frame_index = 0;
        for (const auto &frame: cycle.frames())
        {
            for (size_t i = 0; i < Frame<5>::CellCount; ++i) {
                Frame<5> perturbed_frame = frame;
                perturbed_frame.toggle(i);
                game.set(perturbed_frame);
                auto dest_cycle = game.find_cycle(visited_frames, cycle_frames);

                //cout << dest_cycle << '\n';

                const size_t perturbed_cycle_index = indices.at(dest_cycle);
                //cout << "cycle id: " << perturbed_cycle_index << '\n';
                const size_t row = i / 5, col = i % 5;

                //cout << "(row, col) = (" << row << ", " << col << ")\n";

                pdi[frame_index][col][row] = perturbed_cycle_index;
            }
            ++frame_index;
        }

        // print pretty pdi
        for(size_t row = 0; row < 5; ++row)
        {
            for(size_t frame = 0; frame < cycle.frames().size(); ++frame)
            {
                for(size_t col = 0; col < 5; ++col)
                {
                    os << pdi[frame][col][row] << ' ';
                }
                os << "  ";
            }
            os << '\n';
        }

        os << '\n' << cycle << '\n';
    }

    os.close();
}

void main_flow()
{
    constexpr size_t N = 5;
    auto start = std::chrono::steady_clock::now();
    auto cycles = search_square_orbit<N>();
    cout << "Elapsed(ms)=" << since(start).count() << ", cycles found: " << cycles.size() << '\n';
    const auto indices = assign_indices(cycles);
    write_cycle_data(cycles, indices);
    write_matrix_data(cycles, indices);
}

void special_5x5_flow()
{
    auto start = std::chrono::steady_clock::now();
    unordered_set<Cycle<5>, typename Cycle<5>::Hash, typename Cycle<5>::Equal> cycles;
    unordered_map<Frame<5>, size_t, typename Frame<5>::Hash> visited_frames;
    vector<Frame<5>> cycle_frames;
    GameOfLife<5> game;
    for(size_t i = 0; i < (1 << 24); ++i)
    {
        game.set(Frame<5>(i));
        const auto cycle = game.find_cycle(visited_frames, cycle_frames);
        cycles.insert(cycle);

        constexpr double step = 1.f / 20.f;
        constexpr size_t items_per_step = (1 << 24) * step;
        if(i % items_per_step == 0) {
            cout << "Searched " << 100 * static_cast<double>(i) / static_cast<double>(1 << 24) << "% of all states\n";
        }
    }

    cout << "Elapsed(ms)=" << since(start).count() << ", cycles found: " << cycles.size() << '\n';
    const auto indices = assign_indices(cycles);
    write_5x5(cycles, indices);
}


int main(int argc, char** argv) {
    process_rates_and_decays<9>(
        "9-altered.txt",
        "ranks-9.txt",
        "eigen-9.txt");
    return 0;
}