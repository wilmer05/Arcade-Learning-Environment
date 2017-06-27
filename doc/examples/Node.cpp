#include <ale_interface.hpp>
#include <vector>
#ifdef __USE_SDL
    #include <SDL.h>
#endif
#include"Node.hpp"
#include "constants.hpp"
Node::Node(Node* par, Action act, ALEState ale_state, int d, double rew, double disc) {
    parent = par;
    state = ale_state;
    depth = d;
    reward_so_far = rew;
    discount = disc;
    action = act;
    
    /*std::cout << parent << std::endl;
    std::cout << depth << std::endl;
    std::cout << rew << std::endl;
    std::cout << disc << std::endl;
    std::cout << act << std::endl;*/
}

ALEState Node::get_state(){
    return this->state;
}

std::vector<Node *> Node::get_successors(ALEInterface *env){
    std::vector<Node *> succs;

    if(depth >= max_depth) return succs;

    env->restoreState(state);

    if(env->game_over()) 
        return succs;

    ActionVect acts = env->getMinimalActionSet();
    int cur_d = depth + 1;
    double cur_disc = discount * discount_reward;

    for(int i = 0; i < acts.size(); i++) {
        env->restoreState(state);
        float reward = env->act(acts[i]) * cur_disc;
        ALEState nextState = env->cloneState();
        succs.push_back(new Node(this, acts[i], nextState, cur_d, reward_so_far + reward, cur_disc));
    }
    
    return succs;
}
