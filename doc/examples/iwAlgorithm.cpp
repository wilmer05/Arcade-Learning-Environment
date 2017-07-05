#include<queue>
#include <ale_interface.hpp>
#include"Node.hpp"
#ifdef __USE_SDL
    #include<SDL.h>
#endif
#include<utility>
#include "iwAlgorithm.hpp"
#include "constants.hpp"


IW::IW(int ft, ALEInterface *ale) {
    features_type = ft;
    q = std::queue<Node *>();
    env = ale;
    root = NULL;
    fs = frameskip;
    //std::cout <<"BLA";
    //std::cout <<novelty_table.size() << "\n";
}

void IW::restore_state(Node *nod){
    Node * par = nod->get_parent();
    if(par != NULL){
        env->restoreState(par->get_state());
        env->act(nod->get_action());
     } else{
        env->restoreState(nod->get_state());
     }
}

void IW::reset(){
    if(root == NULL) {
        ActionVect v  = env->getMinimalActionSet();
        root = new Node(NULL, v[0], env->cloneState(), 1, 0, 1);
        best_node = new Node(NULL, v[0], env->cloneState(), -5000000, -5000000, -5000000);
    }
    for(int i = 0; i<300;i++) for(int j = 0; j < 300; j++) novelty_table[i][j]=0;
}

float IW::run() {
    //std::cout <<env << "\n";
    reset();
    //env->restoreState(state);
    //std::vector<unsigned char> vvv;
    //env->getScreenGrayscale(vvv);
    /*for(int i =0 ; i < vvv.size(); i++){
        std::cout << (int) vvv[i]  << " " ;
    }*/
    Node *curr_node  = root;

    std::vector<Node *> chs = curr_node->get_childs();
    for(int i= 0 ; i< chs.size(); i++) {
        chs[i]->count_nodes();
   //    if(chs[i]->reused_nodes > 1)
   //         std::cout <<chs[i]->reused_nodes << "\n";
    }
    //std::cout <<root <<"\n";
    q.push(curr_node);
    ActionVect v  = env->getMinimalActionSet();
    int generated = 1;
    int news = 0;
    int pruned = 0 ;
    int expanded = 0;
    while(!q.empty()){
       curr_node = q.front();
       q.pop();
       expanded ++;
       if (maximum_depth < curr_node->get_depth()) maximum_depth = curr_node->get_depth();
       if(best_node->get_reward_so_far() < curr_node->get_reward_so_far() && curr_node != root)
            best_node = curr_node;
       //for(int i =0 ; i < fs.size(); i++){
      //  std::cout << fs[i].second << " " ;
       //}
       
       //std::cout << "VA\n" ;
       bool leaf = curr_node->get_childs().size() == 0;
       //std::cout <<leaf<< "\n";
       std::vector<Node *> succs;
       if(curr_node -> get_depth() < max_depth / this->fs) {
            succs = curr_node->get_successors(env);
       }
       curr_node -> unset_count_in_novelty();

       for(int i =0 ; i < succs.size() && generated < max_lookahead / this->fs; i++){
           //if(succs[i]->get_is_duplicate()) continue;
           if(leaf) {
                generated ++;
			    if (check_and_update_novelty(succs[i]) != 1){
				    succs[i]->set_is_terminal(true);
				    pruned++;
				//continue;
			    }
           } else{
                if(succs[i] -> get_is_terminal()){
                      if(check_and_update_novelty(succs[i]) == 1){
                        //add_to_novelty_table(fs);
                        succs[i]->set_is_terminal(false);
                      } else{
                        pruned++;
                        succs[i]->set_is_terminal(true);
                      }
                }
           //restore_state(succs[i]);
           ////std::cout << env->getFrameNumber() << "\n";
           //std::vector<std::pair<int,byte_t> > fs = get_features(); 
           //if( succs[i]-> get_count_in_novelty() && novelty(fs) == 1){
           //     //std::cout << curr_node->get_depth() << "\n";
           //     add_to_novelty_table(fs);
           //     //std::cout << "VA2\n" ;
           //     q.push(succs[i]); 
           //     generated ++;
           //     news ++;
           // }
           // else if(! succs[i]->get_count_in_novelty()) {
           //     q.push(succs[i]); 
           //     generated ++;
           // }
           // else pruned ++;
            }
           if(!succs[i]->get_is_terminal() && !succs[i]->test_duplicate() && succs[i]->reused_nodes < max_lookahead / this->fs) q.push(succs[i]);
     //      else if(succs[i]->reused_nodes >= max_lookahead) std::cout <<"Obviado para busqueda\n";
         }
            
    }
    std::cout<< "Best node at depth: " << best_node->get_depth() << ", reward:" << best_node -> get_reward_so_far() << std::endl;
    std::cout<< "Generated nodes: " << generated << std::endl;
    std::cout <<"Expanded nodes:" << expanded << "\n";
    std::cout<< "Pruned nodes: " << pruned << std::endl;
    std::cout<< "Maximum depth: " << maximum_depth << std::endl;
    Node *tmp_node = best_node;
    while(best_node->get_depth() > 2) best_node = best_node->get_parent();
    Action best_act = best_node -> get_action();

    std::vector<Node *> ch = root->get_childs();
    restore_state(root);
    float rw = env->act(best_act);
    for(int i =0 ; i<ch.size(); i++) {
        if(ch[i] != best_node) 
            remove_tree(ch[i]);
        else {
            //std::cout <<"Si hay un nuevo buen root\n";
            update_tree(ch[i], rw);
        }
    }
    root = best_node;
    best_node = tmp_node;
    if(root == best_node){ 
        best_node = new Node(NULL, v[0], env->cloneState(), -5000000, -5000000, -5000000);
        std::cout <<"Best node restarted\n";
    }
    std::cout <<"Best action: " << best_act << std::endl;
    return rw;
}

void IW::remove_tree(Node * nod){
    std::vector<Node *> ch = nod->get_childs();
    for(int i =0 ; i< ch.size(); i++) remove_tree(ch[i]);
    delete nod;
}

void IW::update_tree(Node *nod, float reward){
    std::vector<Node *> ch = nod->get_childs();
    for(int i =0 ; i< ch.size(); i++) 
        if(!ch[i]->get_is_duplicate())update_tree(ch[i], reward);
    nod->set_depth(nod->get_depth() - 1);
    nod->set_reward_so_far(reward);
}
int IW::check_and_update_novelty( Node * nod){

    restore_state(nod);
    std::vector<std::pair<int,byte_t> > fs = get_features(); 
    int nov = novelty(fs);
    if ( nov == 1 ) {
	    add_to_novelty_table(fs);
	}

    return nov;
}
std::vector<std::pair<int,byte_t> > IW::get_features(){
    std::vector<std::pair<int,byte_t> > fs;
    if (features_type == RAM_FEATURES){

        const ALERAM &ram = env->getRAM();
        fs = std::vector<std::pair<int,byte_t> >();
        for(int i = 0 ; i < RAM_SIZE; i++){
            fs.push_back(std::make_pair (i, ram.get(i)));
        }
    
    }

    return fs;
}

int IW::novelty(std::vector<std::pair<int, byte_t> > fs){
    int nov = 1e9;

    for(int i =0 ; i< fs.size() && nov > 1;i++){
        if(novelty_table[fs[i].first][(int)fs[i].second] == 0){
            nov = 1;
        }
    }

    //if(nov==1)
    //    std::cout << nov << " \n";
    return nov;
}

void IW::add_to_novelty_table(std::vector<std::pair<int,byte_t> > fs){
    for(int i =0 ; i< fs.size();i++){
        novelty_table[fs[i].first][(int)fs[i].second] = 1;
    }
}
