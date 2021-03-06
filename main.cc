#include <iostream>
#include <vector>
#include <regex>
#include <list>
#include <map>
#include <condition_variable>
#include <string>
#include <utility>
#include <mutex>
#include <fstream>
#include <thread>

#include "include/actor.h"
#include "include/instruction.h"

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

    map<pair<int, int>, vector<actor*>> occupancy_graph;
    vector<actor*> actors;

    for (int i = 1; i < argc; i++) {
        fstream input(argv[i]);

        if (!input.is_open()) {
            cout << "FIRE DOESN'T EXIST: " << argv[i] << endl;
            return 1;
        }

        // Instruction parser
        string line;
        regex parser("(\\S+)");
        smatch results;

        actor* curr_actor = new actor(i);

        while (getline(input, line)) {
            auto words_begin = sregex_iterator(line.begin(), line.end(), parser);
            auto words_end = sregex_iterator();

            // Pull out the movement direction
            pair<int, int> dir;
            auto direction_word = words_begin->str();

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
                return 1;
            }

            words_begin++;

            // Pull out the instruction name
            string ins_name = words_begin->str();
            function<void(actor&, vector<string>)> fn = instruction_map[ins_name];

            if (!fn) {
                cout << ins_name << " IS NOT A VALID INSTRUCTION" << endl;
                return 1;
            }

            words_begin++;

            // Pull out the arguments
            vector<string> args;
            for (sregex_iterator i = words_begin; i != words_end; ++i) {
                args.push_back(i->str());
            }

            // Register the instruction with the actor associated with
            // the file.
            curr_actor->add_instruction(dir, fn, args, ins_name);
        }

        // Keep track of the constructed actor.
        actors.push_back(curr_actor);
    }

    while (true) {
        // Move the actors and mark their cells they land in.
        for (actor* curr_actor : actors) {
            curr_actor->move();
            auto cell_actors = &occupancy_graph[curr_actor->get_coords()];
            cell_actors->push_back(curr_actor);
        }

        for (auto cell_actor_pair = occupancy_graph.begin(); cell_actor_pair != occupancy_graph.end(); ++cell_actor_pair) {
            for (actor* cell_actor : cell_actor_pair->second) {
                cell_actor->execute();
                cell_actor->print();
            }
        }

        occupancy_graph.clear();
    }

    return 0;
}
