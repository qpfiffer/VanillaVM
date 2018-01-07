#include <iostream>
#include <vector>
#include <regex>
#include <map>
#include <string>
#include <utility>
#include <fstream>
#include <cctype>
#include <thread>

#include "actor.h"
#include "instruction.h"

using namespace std;

map<pair<int, int>, map<string, int>> memory;

/* TEMPLATE
 * {
 *     "select", [](actor& ins_actor, vector<string> args) {
 *     }
 * }
 */

map<string, instruction_fn> instruction_map {
    {
        "drop", [](actor& ins_actor, vector<string> args){
            memory[ins_actor.get_coords()] = ins_actor.getContext();
            ins_actor.getContext().clear();
        }
    },

    {
        "grab", [](actor& ins_actor, vector<string> args) {
            ins_actor.setContext(memory[ins_actor.get_coords()]);
            memory[ins_actor.get_coords()]; 
        }
    },

    {
        "select", [](actor& ins_actor, vector<string> args) {
            auto context = memory[ins_actor.get_coords()];
            ins_actor.getContext()[args[0]] = context[args[0]];
        }
    },

    {
        "write", [](actor& ins_actor, vector<string> args) {
            auto context = memory[ins_actor.get_coords()];
            context[args[0]] = ins_actor.getContext()[args[0]];
        }
    },

    {
        "write", [](actor& ins_actor, vector<string> args) {
            auto context = memory[ins_actor.get_coords()];
            context[args[0]] = ins_actor.getContext()[args[0]];
        }
    },

    {
        "print", [](actor& ins_actor, vector<string> args) {
            ins_actor.print();
        }
    },

    {
        "noop", [](actor& ins_actor, vector<string> args) {
        }
    },

    {
        "sync", [](actor& ins_actor, vector<string> args) {
            // TODO: notify actor to pause
        }
    },
};

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "FIRE REQUIRED" << endl;
        return 1;
    }

    map<pair<int, int>, vector<actor>> occupancy_graph;
    vector<actor> actors;

    for (int i = 1; i < argc; i++) {
        fstream input(argv[i]);
        string line;
        regex parser("(\\S+)");
        smatch results;

        actor curr_actor(i);

        while (getline(input, line)) {
            auto words_begin = sregex_iterator(line.begin(), line.end(), parser);
            auto words_end = sregex_iterator();

            pair<int, int> dir;
            auto direction_word = words_begin->str();
            words_begin++;

            if (direction_word == "up") {
                dir = {0, 1};
            } else if (direction_word == "down") {
                dir = {0, -1};
            } else if (direction_word == "left") {
                dir = {-1, 0};
            } else if (direction_word == "right") {
                dir = {1, 0};
            } else {
                cout << direction_word << " IS NOT A VALID DIRECTION" << endl;
                return 0;
            }

            string ins_name = words_begin->str();
            function<void(actor&, vector<string>)> fn = instruction_map[ins_name];
            words_begin++;

            vector<string> args;
            for (sregex_iterator i = words_begin; i != words_end; ++i) {
                args.push_back(i->str());
            }

            curr_actor.add_instruction(dir, fn, args, ins_name);
        }

        actors.push_back(curr_actor);
    }

    while (true) {
        // Move the actors and mark their cells they are in
        for (actor curr_actor : actors) {
            curr_actor.move();
            auto cell_actors = &occupancy_graph[curr_actor.get_coords()];
            cell_actors->push_back(curr_actor);
        }


        vector<pair<vector<actor>::iterator, vector<actor>::iterator>> actor_iters;
        for (auto i = occupancy_graph.begin(); i != occupancy_graph.end(); i++) {
            actor_iters.push_back({i->second.begin(), i->second.end()});
        }

        // Iterate through the cell actor iterators till they are all
        // done executing.  Each cell should have a thread associated
        // with it that executes each of its actors instructions in
        // order that the actors appear in the actor vector.
        bool done = false;
        vector<thread> actor_threads;
        while (!done) {
            done = true;

            for (int i = 0; i < actor_iters.size(); i++) { 
                auto curr_actor_iter_pair = actor_iters[i];
                if (curr_actor_iter_pair.first != curr_actor_iter_pair.second) {
                    done = false;
                    actor_threads.emplace_back([&](){
                        curr_actor_iter_pair.first->print();
                    });
                    actor_iters[i].first++;
                }
            }

            for (int i = 0; i < actor_threads.size(); i++) {
                actor_threads[i].join();
            }

            actor_threads.clear();
        }

        occupancy_graph.clear();
    }

    return 0;
}