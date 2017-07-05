#include<queue>
#include <ale_interface.hpp>
#include"Node.hpp"
#ifdef __USE_SDL
    #include<SDL.h>
#endif
#include<utility>
#include "iwAlgorithmRGB.hpp"
#include "constants.hpp"


IWRGB::IWRGB(int ft, ALEInterface *ale, int fs) {
    features_type = ft;
    q = std::queue<Node *>();
    env = ale;
    root = NULL;
    this -> fs = fs;

    //std::cout <<novelty_table.size() << "\n";
}

void IWRGB::restore_state(Node *nod){
    Node * par = nod->get_parent();
    if(par != NULL){
        env->restoreState(par->get_state());
        env->act(nod->get_action());
     } else{
        env->restoreState(nod->get_state());
     }
}

void IWRGB::reset(){
    if(root == NULL) {
        ActionVect v  = env->getMinimalActionSet();
        root = new Node(NULL, v[0], env->cloneState(), 1, 0, 1);
        best_node = new Node(NULL, v[0], env->cloneState(), -5000000, -5000000, -5000000);
    }
    for(int i = 0; i<k_novelty_columns;i++) for(int j = 0; j < k_novelty_rows; j++) for(int k = 0; k<k_different_colors;k++) novelty_table_basic[i][j][k]=0;

    if(compute_BPROS()){
         
        for(int i = 0; i<2 * k_novelty_columns;i++) 
            for(int j = 0; j < 2 * k_novelty_rows; j++) 
                for(int k1 = 0; k1<k_different_colors;k1++)  
                    for(int k2 = 0 ; k2<k_different_colors;k2++) novelty_table_bpros[k1][k2][i][j]=0;

    }
}

float IWRGB::run() {
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
       if(curr_node->get_depth() < max_depth / this -> fs){
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
           if(!succs[i]->get_is_terminal() && !succs[i]->test_duplicate() && succs[i]->reused_nodes < max_lookahead  / this->fs) q.push(succs[i]);
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
        best_node = new Node(NULL, v[rand() % v.size()], env->cloneState(), -5000000, -5000000, -5000000);
        std::cout <<"Best node restarted\n";
    }
    std::cout <<"Best action: " << best_act << std::endl;
    return rw;
}

void IWRGB::remove_tree(Node * nod){
    std::vector<Node *> ch = nod->get_childs();
    for(int i =0 ; i< ch.size(); i++) remove_tree(ch[i]);
    delete nod;
}

void IWRGB::update_tree(Node *nod, float reward){
    std::vector<Node *> ch = nod->get_childs();
    for(int i =0 ; i< ch.size(); i++) 
        if(!ch[i]->get_is_duplicate())update_tree(ch[i], reward);
    nod->set_depth(nod->get_depth() - 1);
    nod->set_reward_so_far(reward);
}
int IWRGB::check_and_update_novelty( Node * nod){

    restore_state(nod);
    basic_table_t fs = get_features(); 

    int nov = novelty(fs);
    //std::cout << nov <<" " ;
    return nov;
}
basic_table_t IWRGB::get_features(){
    std::vector<byte_t> screen;
    env->getScreenGrayscale(screen);

    basic_table_t v(k_novelty_columns, std::vector<std::set<int> >(k_novelty_rows, std::set<int>()));
    for(int i=0; i<screen.size(); i++){
        int c = (i % 160) / k_box_columns_size;
        int r = (i / 160) / k_box_rows_size;
        //std::cout << r << " " << c << "\n";
        //std::cout << i << " " << c << " " << r << "\n";
        v[c][r].insert((int)screen[i]);
        //if((int) screen[i] > 200)
        //std::cout << (int) screen[i] << "\n";

    } 
    return v;
}

int IWRGB::novelty(basic_table_t &fs){
    int nov = 1e9;
    std::set<int>::iterator it, it2;
    int sz1 = fs.size(); int sz2 = fs[0].size();
//    std::cout << sz1 << " " << sz2 << "\n";
    for(int i =0 ; i< sz1;i++){
        for(int j = 0 ; j < sz2; j++){
            it = fs[i][j].begin();
            while(it != fs[i][j].end()){
                
                if(novelty_table_basic[i][j][*it] == 0){
                    nov = 1;
                }
                
                novelty_table_basic[i][j][*it] = 1;
                if(compute_BPROS()){
                    //std::cout << "BPROS" << "\n";
                    for(int i2 =0 ; i2< sz1;i2++){
                        for(int j2 = j ; j2 < sz2; j2++){
                            if(j2 == j && i2 <= i) continue;
                            it2 = fs[i2][j2].begin();
                            while(it2 != fs[i2][j2].end()){
                                int k1 = *it;
                                int k2 = *it2;
                                int cc = i - i2 + k_novelty_columns;
                                int rr = j - j2 + k_novelty_rows;
                                //std::cout << cc << " " << rr << " " << k1 << " " << k2 << "\n";
                                //std::cout <<"pase" << "\n";
                                if(cc < 0 || rr < 0 || k1 < 0 || k2 < 0){
                                    //std::cout << cc << " " << rr << " " << i << "-"  << i2 << " " << j<< "-" << j2<< " " << k1 << " " << k2 << "\n";
                                    //continue;
                                }
                                if(novelty_table_bpros[k1][k2][cc][rr] == 0){
                                    //std::cout << "BPROS_NEW" << "\n";
                                    nov = 1;
                                }
                                novelty_table_bpros[k1][k2][cc][rr] = 1;
                                it2++;
                            }
                        }
                    }
                
                }
                it++;
            }
        }
    }

    //if(nov==1)
    //    std::cout << nov << " \n";
    return nov;
}
