#include<queue>
#include <ale_interface.hpp>
#include"Node.hpp"
#ifdef __USE_SDL
    #include<SDL.h>
#endif
#include<utility>
#include "iwAlgorithm.hpp"
#include "constants.hpp"


IW::IW(int ft) {
    features_type = ft;
    novelty_table = std::set<std::pair<int, int> >();
    q = std::queue<Node *>();
}

Action IW::run(ALEState state, ALEInterface *env) {
    env->restoreState(state);
    ActionVect v  = env->getMinimalActionSet();
    Node *curr_node  = new Node(NULL, v[0], state, 1, 0, 1);
    q.push(curr_node);
    Node *best_node = curr_node;
    int generated = 1;
    std::vector<Node *> clear;
    clear.push_back(curr_node);
    while(!q.empty()){
       curr_node = q.front();
       q.pop();
       if(best_node->get_reward_so_far() < curr_node->get_reward_so_far())
            best_node = curr_node;
       env->restoreState(state);
       std::vector<std::pair<int,int> > fs = get_features(env); 
       if(novelty(fs) == 1){
            add_to_novelty_table(fs);
            if(generated < max_lookahead) {
                std::vector<Node *> succs = curr_node->get_successors(env); 
                for(int i =0 ; i < succs.size() && generated < max_lookahead; i++){
                    q.push(succs[i]); 
                    clear.push_back(succs[i]);
                    generated ++;
                }
            }
       } 

    }
    std::cout<< "Best node at depth: " << best_node->get_depth() << ", reward:" << best_node -> get_reward_so_far() << std::endl;
    std::cout<< "Generated nodes: " << generated << std::endl;
    while(best_node->get_depth() > 2) best_node = best_node->get_parent();
    Action best_act = best_node -> get_action();
    for(int i =0 ; i<clear.size(); i++) delete clear[i];
    std::cout <<"Best action: " << best_act << std::endl;
    return best_act;
}

std::vector<std::pair<int,int> > IW::get_features(ALEInterface *ale){
    std::vector<std::pair<int,int> > fs;
    if (features_type == RAM_FEATURES){

        const ALERAM &ram = ale->getRAM();
        fs = std::vector<std::pair<int,int> >();
        for(int i = 0 ; i < RAM_SIZE; i++)
            fs.push_back(std::make_pair (i, (int)ram.get(i)));
    
    }

    return fs;
}

int IW::novelty(std::vector<std::pair<int,int> > fs){
    int nov = 1e9;

    for(int i =0 ; i< fs.size();i++){
        if(novelty_table.count(fs[i]) == 0)
            nov = 1;
    }

    return nov;
}

void IW::add_to_novelty_table(std::vector<std::pair<int,int> > fs){
    for(int i =0 ; i< fs.size();i++){
        novelty_table.insert(fs[i]);
    }
}
