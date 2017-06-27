#include <ale_interface.hpp>
#include <vector>
#ifdef __USE_SDL
    #include <SDL.h>
#endif
#include"constants.hpp"
#ifndef DEF_NODE
#define DEF_NODE
class Node{
    public:

        Node(Node* par, Action act, ALEState ale_state, int d, double rew, double discs);

        std::vector<Node *> get_successors(ALEInterface *env);

        ALEState get_state();
        
        double get_reward_so_far(){ return reward_so_far; }

        int get_depth() { return depth; } 

        Node * get_parent() { return parent;}

        Action get_action() { return action; } 

    private:

        Node *parent;
        ALEState state;
        int depth;
        double reward_so_far;
        Action action;
        double discount;
};
#endif
