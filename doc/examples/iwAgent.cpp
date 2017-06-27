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
#include"iwAlgorithm.hpp"
#ifdef __USE_SDL
  #include <SDL.h>
#endif
#include "constants.hpp"
#include "iwAlgorithm.hpp"
using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " rom_file" << std::endl;
        return 1;
    }

    ALEInterface ale;

    // Get & Set the desired settings
    ale.setInt("frame_skip", 5);
    ale.setInt("random_seed", 123);
    //The default is already 0.25, this is just an example
    ale.setFloat("repeat_action_probability", 0.25);

#ifdef __USE_SDL
    ale.setBool("display_screen", true);
    ale.setBool("sound", true);
#endif

    // Load the ROM file. (Also resets the system for new settings to
    // take effect.)

    ale.loadROM(argv[1]);
    ALEState curr_state = ale.cloneState();

    float totalReward = 0;
    int episode;
    for (episode=0; !ale.game_over() && episode<max_steps; episode++) {
        IW iw = IW(1);
        Action a = iw.run(curr_state, &ale);
        
        ale.restoreState(curr_state);
        totalReward += ale.act(a);
        curr_state = ale.cloneState();
        cout << "Current frame: " << ale.getFrameNumber() << endl;

        cout << "Episode " << episode << " ended with score: " << totalReward << endl;
    }
    if(episode == max_steps) cout << "Ended by number of steps\n";
    else cout <<"Ended by game over \n";
    return 0;
}
