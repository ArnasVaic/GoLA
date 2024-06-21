#pragma once

#include <cstddef>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <numeric>
#include "cycle.hpp"
#include "game_of_life.hpp"

std::map<size_t, size_t> read_rankings(std::string const& filename)
{
    using std::ifstream, std::string;
    ifstream stream(filename);
    string line;
    std::map<size_t, size_t> rankings;
    size_t line_number = 0;
    while(std::getline(stream, line))
        rankings[std::stoi(line)] = line_number++;
    return rankings;
}

inline std::string trim(const std::string &s)
{
    auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
    auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
    return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}

// We have to take original file (in folder old)
// and process it to add additional information:
// - unique ids of cycles it decays to (except 0)
// - rates of decay to each of those ids (decaying to itself has negative sign?)
// the ordering in final file has to be according to rankings provided in file
// decays have to be ordered according to rankings so before calculation the
// cycles should be indexed according to rankings.
// also filter only those cycles that are present in rankings.
template<size_t N>
void process_rates_and_decays(
        std::string const& filename,
        std::string const& rankings_filename,
        std::string const& eigenvalues_filename)
{
    using std::ifstream, std::string, std::cout, std::vector, std::unordered_set, std::unordered_map;

    string line;
    ifstream input_file(filename);
    ifstream eigenvalue_file(filename);
    string eigenvalue;
    std::ofstream output("Torus9x9.txt");
    size_t line_number = 0;

    const size_t LINES_PER_CONFIGURATION = 2; // only for our special 9x9 altered file
    size_t current_cycle_period = -1, current_cycle_id = -1;
    vector<Cycle<N>> cycles;
    unordered_set<Cycle<N>, typename Cycle<N>::Hash, typename Cycle<N>::Equal> cycles_set;
    unordered_map<Cycle<N>, size_t, typename Cycle<N>::Hash, typename Cycle<N>::Equal> cycles_indices;

    std::map<size_t, size_t> indices = read_rankings(rankings_filename);

    if(9 == N)
    {
        assert(indices.size() == 7363);
    }

    const string prefix = "[T = ", middle = ", id: ";
    while (std::getline(input_file, line))
    {
        std::istringstream iss(line);
        // We can discard all lines that pretty print the configurations
        if(line_number % LINES_PER_CONFIGURATION == 0)
        {
            // Handle configuration info (period and id)
            size_t period, id;

            //cout << "parsing line : '" << line << "'\n";

            iss.ignore(prefix.length());
            iss >> period;
            iss.ignore(middle.length());
            iss >> id;
            //cout << prefix << period << middle << id << "]\n";
            current_cycle_period = period;
            current_cycle_id = id;
        }
        else if(line_number % LINES_PER_CONFIGURATION == 1)
        {
            // Handle configuration state
            uint64_t hi, lo;
            vector<Frame<N>> frames(current_cycle_period);
            for(size_t i = 0; i < current_cycle_period; ++i)
            {
                iss >> hi >> lo;
                frames[i] = Frame<N>(absl::MakeUint128(hi, lo));
            }
            Cycle<N> cycle(frames);
            // cycle set will only contain the required cycles
            if(indices.contains(current_cycle_id))
            {
                cycles_set.insert(cycle);
                cycles_indices[cycle] = indices[current_cycle_id];
            }
        }

        if(line_number % 10000 == 0)
        {
            cout << "read " << line_number << " lines\n";
            cout << "cycle set size: " << cycles_set.size() << '\n';
        }

        line_number++;
    }


    std::vector<Cycle<N>> cycle_vector(cycles_indices.size());

    cout << "cycle indices elements:  " << cycles_indices.size() << "\n";
    cout << "allocating cycle vector [" << cycle_vector.size() << " elements]...\n";

    cout << "populating cycle vector...\n";
    for(const auto& [ cycle, id ] : cycles_indices) {
        cycle_vector.at(id) = cycle;
    }


    cout << "cycle vector size: " << cycle_vector.size() << '\n';

    GameOfLife<N> game;
    unordered_map<Frame<N>, size_t, typename Frame<N>::Hash> visited_frames;
    vector<Frame<N>> cycle_frames;
    unordered_map<Cycle<N>, size_t, typename Cycle<N>::Hash, typename Cycle<N>::Equal> decay_rates;

    for(const auto& cycle : cycle_vector)
    {
        int cycle_index = cycles_indices.at(cycle);
        int cycle_length = cycle.frames().size();

        if(cycle_index == 0)
            continue;

        std::getline(eigenvalue_file, eigenvalue);
        cout << "Cycle id: " << cycle_index << ", period: " << cycle_length << '\n';
        output << "#" << cycle_index << ": c" << cycle_length << ": " << trim(eigenvalue) << "\n";

        // toggle each cell, add
        for (const auto &frame: cycle.frames())
        {
            for (size_t i = 0; i < Frame<N>::CellCount; ++i) {
                Frame<N> perturbed_frame = frame;
                perturbed_frame.toggle(i);
                game.set(perturbed_frame);
                auto dest_cycle = game.find_cycle(visited_frames, cycle_frames);

                if (decay_rates.contains(dest_cycle))
                    decay_rates[dest_cycle]++;
                else
                    decay_rates[dest_cycle] = 1;
            }
        }

        // artificially add itself if not yet existing (with freq 0)
        if (!decay_rates.contains(cycle))
            decay_rates[cycle] = 0;

        // Pretty print rates and decays (decays must be in ascending order)

        vector<std::pair<Cycle<N>, size_t>> ordered_rates_and_decays(decay_rates.begin(), decay_rates.end());

        std::sort(ordered_rates_and_decays.begin(), ordered_rates_and_decays.end(), [&](
            std::pair<Cycle<N>, size_t> const& left,
            std::pair<Cycle<N>, size_t> const& right)
        {
            return cycles_indices.at(left.first) < cycles_indices.at(right.first);
        });

        output << "decays: ";
        for(size_t i = 0; i < ordered_rates_and_decays.size(); ++i)
        {
            const auto& [decay_cycle, rate] = ordered_rates_and_decays[i];
            if(cycles_indices.at(decay_cycle) == 0)
            {
                continue;
            }

            output << cycles_indices.at(decay_cycle);

            if(i != ordered_rates_and_decays.size() - 1)
            {
                output << ',';
            }
        }
        output << '\n';

        output << "rates: ";
        for(size_t i = 0; i < ordered_rates_and_decays.size(); ++i)
        {
            const auto& [decay_cycle, calculated_rate] = ordered_rates_and_decays[i];

            int rate = static_cast<int>(calculated_rate);

            if(cycles_indices.at(decay_cycle) == 0)
            {
                continue;
            }
            if(cycles_indices.at(decay_cycle) == cycles_indices.at(cycle))
            {
                rate -= static_cast<int>(N * N * cycle_length);
            }

            if(0 == rate % cycle_length)
            {
                output << rate / cycle_length;
            }
            else
            {
                int d = std::gcd(rate, cycle_length);
                output << rate / d << "/" << cycle_length / d;
            }

            if(i != ordered_rates_and_decays.size() - 1)
            {
                output << ',';
            }
        }
        output << '\n';

        output << *cycle.frames().begin() << '\n';
        decay_rates.clear();
    }

    input_file.close();
    eigenvalue_file.close();
    output.close();
}