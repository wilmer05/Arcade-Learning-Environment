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
    childs = std::vector<Node *>();
    count_in_novelty = true;
    /*std::cout << parent << std::endl;
    std::cout << depth << std::endl;
    std::cout << rew << std::endl;
    std::cout << disc << std::endl;
    std::cout << act << std::endl;*/
}

ALEState Node::get_state(){
    return this->state;
}


void Node::restore_state(Node *nod, ALEInterface *env){
    Node * par = nod->get_parent();
    if(par != NULL){
        env->restoreState(par->get_state());
        env->act(nod->get_action());
     } else{
        env->restoreState(nod->get_state());
     }
}

std::vector<Node *> Node::get_successors(ALEInterface *env){
    std::vector<Node *> succs;

    if(childs.size() > 0) return childs;

    if(depth >= max_depth) {
        return succs;
    }

    restore_state(this, env);

    if(env->game_over()){ 
        return succs;
    }

    ActionVect acts = env->getMinimalActionSet();
    int cur_d = depth + 1;
    double cur_disc = discount * discount_reward;

    for(int i = 0; i < acts.size(); i++) {
        restore_state(this, env);
        //std::cout << acts[i] << " -> \n";
        
        //std::cout << env->getFrameNumber() << " -.- \n";
        
        float reward = env->act(acts[i]) * cur_disc;
        ALEState nextState = env->cloneState();
        //if(nextState == env->cloneState) std::cout <<"WHAT" << "\n";
        //if(reward != 0.0) std::cout << "algo hay" << "\n";
        if(env->game_over()) reward = -10000000;
        //std::cout << env->getFrameNumber() << "\n";
        //std::cout << acts[i] <<"\n";
        succs.push_back(new Node(this, acts[i], nextState, cur_d, reward_so_far + reward, cur_disc));
    }

    childs = succs;
    return childs;
}
