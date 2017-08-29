/* *****************************************************************************
 * A.L.E (Arcade Learning Environment)
 * Copyright (c) 2009-2013 by Yavar Naddaf, Joel Veness, Marc G. Bellemare,
 *  Matthew Hausknecht, and the Reinforcement Learning and Artificial Intelligence 
 *  Laboratory
 * Released under the GNU General Public License; see License.txt for details. 
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  sharedLibraryInterfaceExample.cpp 
 *
 *  Sample code for running an agent with the shared library interface. 
 **************************************************************************** */

#include <iostream>
#include <ale_interface.hpp>
#include"iwAlgorithmRGB.hpp"
#ifdef __USE_SDL
  #include <SDL.h>
#endif
#include "constants.hpp"
#include "iwAlgorithmRGB.hpp"
#include <ctime>
#include <cstdlib>
#include "butils.h"
#include "iwCarmel.hpp"
using namespace std;
double CNT::d_rw;

int main(int argc, char** argv) {
    if (argc < 7) {
        std::cerr << "Usage: " << argv[0] << " rom_file <feature_types (1,2,3)> <frame_skip> <tile_row_size> <tile_column_size> <delta_displacement> <optional_rate>\nFeatures types:\n\t1 For basic features\n\t2 for basic features + BPROS\n\t3 for basic + BPROS + BPROT\n\n The displacement is the displacement of basic features on the rows (a number between 0 and tile_row_size - 1)" << std::endl;
        return 1;
    }

    /*if(argc > 7){
        //std::cout << "Changing discount reward to " << atof(argv[7]) << "\n";
        CNT::d_rw = atof(argv[7]);
//        discount_reward = atof(argv[7]);
    } else {
   */    // std::cout << "WTF?\n";
        CNT::d_rw = 0.995;
   // }



    ALEInterface ale;
    std::srand(unsigned(std::time(0)));
    // Get & Set the desired settings
    std::cout << "FrameSkip set to: " << argv[3] << "\n";
    ale.setInt("frame_skip", atoi(argv[3]));
    ale.setInt("random_seed", 123);
    //The default is already 0.25, this is just an example
    ale.setFloat("repeat_action_probability", 0.0);

#ifdef __USE_SDL
    ale.setBool("display_screen", true);
    ale.setBool("sound", true);
#endif

    // Load the ROM file. (Also resets the system for new settings to
    // take effect.)

    int fs = atoi(argv[3]);
    ale.loadROM(argv[1]);
    ale.reset_game();
    //ALEState curr_state = ale.cloneState();
    float totalReward = 0;
    int episode;
    //ActionVect legal_actions = ale.getMinimalActionSet();
    IWRGB *iw;
    if(atoi(argv[2]) <= 4 )
        iw = new IWRGB(atoi(argv[2]), &ale, fs, atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
    else { 
        std::cout << "Using Carmels trick\n";
        iw = new IWCARMEL(atoi(argv[2]) - 4, &ale, fs, atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
    }
    cout << "Starting Episode\n";
    if(argc > 7) {
        std::cout << "Removing cache\n";
        iw -> cache = false;
    }
    ActionVect v  = ale.getMinimalActionSet();
    int nr = rand() % 10;
    if (!nr) nr = 1;

    for(int i =0 ; i < nr;i++){
        totalReward += ale.act(v[0]);
    }


    for (episode=0; !ale.game_over() && episode<max_steps / fs; episode++) {

        float start,end;
        //time (&start);
        start = Utils::read_time_in_seconds();
        
        totalReward += iw->run();
        
        end = Utils::read_time_in_seconds();

        cout << "Step number " << episode  + 1<< " ended with score: " << totalReward << endl;
        cout <<"Elapsed time: " << end-start << endl;
        cout << "Total features in lookahead = " << iw->get_total_features() << "\n";
    }
    if(episode == max_steps) cout << "Ended by number of steps\n";
    else cout <<"Ended by game over \n";
    return 0;
}
