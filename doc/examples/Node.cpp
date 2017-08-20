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
#include<cassert>
Node::Node(Node* par, Action act, ALEState *ale_state, int d, double rew, double disc, std::vector<byte_t> feat) {
    parent = par;
    tested_duplicate = false;
    features_computed = false;
    generated_at_step = 10000000;
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
    freed = false;
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
    freed = false;
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
    if(this->childs.size() > 0 || this->state==NULL) return nod;

    env->restoreState(this->get_state());
    
    int cur_d = depth + 1;
    double cur_disc = discount * discount_reward;
    if(!env->game_over()){
        float reward = env->act(a) * cur_disc;
        ALEState *nextState = new ALEState(env->cloneState());
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
            nod->generated_at_step = this->generated_at_step;
            this -> childs.push_back(nod);
        }catch(std::bad_alloc &ba){
        //    std::cout << "Bad allocation on dfs\n";
            nod = NULL;
        }
    }
    

    return nod;
}

ALEState Node::get_state(){
    return *(this->state);
}


void Node::restore_state(Node *nod, ALEInterface *env){
    Node * par = nod->get_parent();
    if(par != NULL){
        assert(par->state != NULL);
        env->restoreState(par->get_state());
        env->act(nod->get_action());
     } else{
        assert(nod->state != NULL);
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
    
  /*      if(sibling->state == NULL || node->state == NULL){
    //        std::cout << "#########SENDO BUGGGGGGGGGGG#########\n";
        }*/
         if (sibling->state != NULL && node->state != NULL && sibling->get_state().equals(*(node->state))) {
             node->set_is_duplicate(true);
             return true;
         }
    }
    // None of the siblings match, unique node
    node->set_is_duplicate(false);
    return false;
    
}

int my_random(int i) { return std::rand() % i ;}

std::vector<Node *> Node::get_successors(ALEInterface *env, bool take_screen, int l_number){
    std::vector<Node *> succs;
    std::srand(unsigned (std::time(0)));
    if(childs.size() > 0 || this->state == NULL) return childs;

    assert(this->state != NULL);
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
        ALEState *nextState = new ALEState(env->cloneState());
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
            Node *my_succ = new Node(this, acts[i], nextState, cur_d, reward_so_far + reward, cur_disc, v);
            my_succ->generated_at_step = l_number + 1;
            succs.push_back(my_succ);
        } catch(std::bad_alloc& ba){
        //   std::cout << "Bad alloc in get_succs\n";
           break; 
        }
    }
    random_shuffle(succs.begin(), succs.end(), my_random);
    //random_shuffle(succs.begin(), succs.end());
    childs = succs;
    return childs;
}

std::vector<Node *> Node::get_stateless_successors(ALEInterface *env){
    std::vector<Node *> succs;

    if(childs.size() > 0 || this->state==NULL) return childs;

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

/*void Node::free_memory(){
    
    for(int i =0 ;i < childs.size();i++)
       childs[i]->free_memory;
}*/

int Node::count_nodes(int look_number){
    int cnt  = 0;
/*    if(look_number - this->generated_at_step > 3) {
//        std::cout <<"Cleaning " << look_number << " " << this->get_depth()<<"\n";
        this->basic_f.clear();
        this->basic_f.push_back(0);
    }*/
    for(int i =0 ;i < childs.size();i++)
        //if(childs[i]->get_childs().size()> 0 )
        cnt += childs[i]->count_nodes(look_number);
    reused_nodes = 1 + cnt;
    return reused_nodes;
}
