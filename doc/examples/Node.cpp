#include <ale_interface.hpp>
#include <vector>
#ifdef __USE_SDL
    #include <SDL.h>
#endif
#include"Node.hpp"
#include "constants.hpp"
#include<algorithm>
#include<cstdlib>
#include<ctime>
Node::Node(Node* par, Action act, ALEState ale_state, int d, double rew, double disc, std::vector<byte_t> feat) {
    parent = par;
    tested_duplicate = false;
    features_computed = false;
    state = ale_state;
    depth = d;
    reward_so_far = rew;
    discount = disc;
    action = act;
    this -> features = feat;
    childs = std::vector<Node *>();
    count_in_novelty = true;
    is_terminal = false;
    is_duplicate = false;
    reused_nodes = 0;
    tried = 0;
    in_tree = false;
    must_be_prunned = false;
    processed_screen.clear();
    differential_screen.clear();
    differential_patch_index.clear();
    patch_index.clear();
    generated_by_df = false; 
    /*std::cout << parent << std::endl;
    std::cout << depth << std::endl;
    std::cout << rew << std::endl;
    std::cout << disc << std::endl;
    std::cout << act << std::endl;*/
}

Node::Node(Node* par, Action act, int d, double rew, double disc) {
    parent = par;
    //state = ale_state;
    depth = d;
    reward_so_far = rew;
    discount = disc;
    action = act;
    childs = std::vector<Node *>();
    count_in_novelty = true;
    is_terminal = false;
    is_duplicate = false;
    reused_nodes = 0;
    tried = 0;
    solved = false;
    in_tree = false;
}

Node * Node::generate_child_with_same_action(ALEInterface * env, bool take_screen){
    Action a = this -> action;
    Node *nod = NULL;

    if(this->childs.size()==1) return this->childs[0];
    if(this->childs.size() > 0) return nod;

    env->restoreState(this->state);
    
    int cur_d = depth + 1;
    double cur_disc = discount * discount_reward;
    if(!env->game_over()){
        float reward = env->act(a) * cur_disc;
        ALEState nextState = env->cloneState();
        if(env->game_over()){
            reward = -10000000;
        }

        std::vector<byte_t> v;
        if(!take_screen){
            const ALERAM &ram = env->getRAM();
            for(int i = 0 ; i < RAM_SIZE; i++){
                v.push_back(ram.get(i));
            }
        } else{
            env->getScreenGrayscale(v);    
        }
        try{
            nod = new Node(this, a, nextState, cur_d, reward_so_far + reward, cur_disc, v);
            nod->generated_by_df = true;
            this -> childs.push_back(nod);
        }catch(std::bad_alloc &ba){
            std::cout << "Bad allocation on dfs\n";
            nod = NULL;
        }
    }
    

    return nod;
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

bool Node::test_duplicate(){
    Node *node = this;
    if (node->get_parent() == NULL) return false;
    else if(tested_duplicate) {return is_duplicate;}

    tested_duplicate = true;
    Node *parent = node->get_parent();

    for (int c = 0; c < parent->childs.size(); c++) {
        Node * sibling = parent->childs[c];
        if (sibling->get_is_duplicate() || sibling == node || sibling->childs.size() == 0) continue;

         if (sibling->state.equals(node->state)) {
             node->set_is_duplicate(true);
             return true;
         }
    }
    // None of the siblings match, unique node
    node->set_is_duplicate(false);
    return false;
    
}

int my_random(int i) { return std::rand() % i ;}

std::vector<Node *> Node::get_successors(ALEInterface *env, bool take_screen){
    std::vector<Node *> succs;
    std::srand(unsigned (std::time(0)));
    if(childs.size() > 0) return childs;

    env->restoreState(this->get_state());
    ActionVect acts = env->getMinimalActionSet();

    int cur_d = depth + 1;
    double cur_disc = discount * discount_reward;

    
    if(env->game_over()){ 
        return succs;
    }
    ALEState node_state = this->get_state();
    for(int i = 0; i < acts.size(); i++) {
        if(i)
            env->restoreState(node_state);
        
        float reward = env->act(acts[i]) * cur_disc;
        ALEState nextState = env->cloneState();
        if(env->game_over()) reward = -10000000;

        std::vector<byte_t> v;
        if(!take_screen){
            const ALERAM &ram = env->getRAM();
            for(int i = 0 ; i < RAM_SIZE; i++){
                v.push_back(ram.get(i));
            }
        } else{
            env->getScreenGrayscale(v);    
        }
        try{
            succs.push_back(new Node(this, acts[i], nextState, cur_d, reward_so_far + reward, cur_disc, v));
        } catch(std::bad_alloc& ba){
           std::cout << "Bad alloc in get_succs\n";
           break; 
        }
    }
    //random_shuffle(succs.begin(), succs.end(), my_random);
    random_shuffle(succs.begin(), succs.end());
    childs = succs;
    return childs;
}

std::vector<Node *> Node::get_stateless_successors(ALEInterface *env){
    std::vector<Node *> succs;

    if(childs.size() > 0) return childs;

    /*if(depth >= max_depth) {
        return succs;
    }*/

    //restore_state(this, env);

    ActionVect acts = env->getMinimalActionSet();
    this -> childActs = acts;
    int cur_d = depth + 1;
    double cur_disc = discount * discount_reward;

    if(env->game_over()){ 
        return succs;
    }
    for(int i = 0; i < acts.size(); i++) {
        //restore_state(this, env);
        //std::cout << acts[i] << " -> \n";
        
        //std::cout << env->getFrameNumber() << " -.- \n";
        
        //float reward = env->act(acts[i]) * cur_disc;
        //ALEState nextState;
        //if(nextState == env->cloneState) std::cout <<"WHAT" << "\n";
        //if(reward != 0.0) std::cout << "algo hay" << "\n";
        //if(env->game_over()) reward = -10000000;
        //std::cout << env->getFrameNumber() << "\n";
        //std::cout << acts[i] <<"\n";
        Node * n_node = new Node(this, acts[i], cur_d, 0.0, cur_disc);
        succs.push_back(n_node);
    }
    random_shuffle(succs.begin(), succs.end());
    childs = succs;
    return childs;
}

int Node::count_nodes(){
    int cnt  = 0;
    for(int i =0 ;i < childs.size();i++)
        //if(childs[i]->get_childs().size()> 0 )
        cnt += childs[i]->count_nodes();
    reused_nodes = 1 + cnt;
    return reused_nodes;
}
