#include<queue>
#include <ale_interface.hpp>
#include"Node.hpp"
#include <algorithm>
#ifdef __USE_SDL
    #include<SDL.h>
#endif
#include<utility>
#include <cmath>
#include "iwAlgorithmRGB.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include <set>

IWRGB::IWRGB(int ft, ALEInterface *ale, int fs, int tile_row_sz, int tile_column_sz, int delta) {
    features_type = ft;
    q = std::queue<Node *>();
    env = ale;
    root = NULL;
    this -> fs = fs;
    tile_row_size = tile_row_sz;
    tile_column_size = tile_column_sz;
     
    std::cout<<"Features type: " << ft << "\n";
    c_number = 160 / tile_column_sz;
    r_number = 210 / tile_row_sz;
    maximum_depth = 0;
    displacement = delta;
    //std::cout << "Basic table with size: " << novelty_table.size() << "\n";
    compute_meaninful_pixels(env);

}

void IWRGB::reset(){

    if(this->features_type == 1)
        novelty_table = std::vector<bool>(k_total_basic_features, false);
    else if(this -> features_type == 2){
        novelty_table = std::vector<bool>(k_total_basic_features + num_cross_features_, false);
    }
    else if(this -> features_type == 3){
        novelty_table = std::vector<bool>(k_total_basic_features + num_cross_features_ + num_temporal_features_, false);
    }

    if(root == NULL) {
        ActionVect v  = env->getMinimalActionSet();
        std::vector<byte_t> feat =  get_feat(env, true);
        root = new Node(NULL, v[rand() % v.size()], env->cloneState(), 1, 0, 1, feat);
        best_node = new Node(NULL, v[rand() % v.size()], env->cloneState(), 1, 0, 1, feat);
    }

    //if(neighbor_comparison()) init_similarity_talbe(c_number + 5, r_number + 5);

    total_features = 0;
    maximum_depth = 0;
}

bool IWRGB::dynamic_frame_skipping(Node *nod){
    Action a = nod -> get_action();
    int steps = 60 / this->fs - 1;

    nod->must_be_prunned = true;
    //std::cout << "Entre\n";
    if(nod->childs.size() <= 1){
         Node *curr_node = nod; 
         int st = 0;
         for(st =0 ; st < steps && st + generated < max_lookahead / this->fs; st++){
            
            //std::cout <<"Sali0\n";
            Node *succ = curr_node->generate_child_with_same_action(env, true);
            //std::cout <<"Sali0-2\n";
            if(succ == NULL) break;
            generated++ ;
            if(check_and_update_novelty(succ) == 1){
                curr_node = succ;
                //std::cout <<"Sali1\n";
                //curr_node -> set_is_terminal(true);
                while(curr_node != nod){
                    if (maximum_depth < curr_node->get_depth()) maximum_depth = curr_node->get_depth();
                    if(best_node->get_reward_so_far() < curr_node->get_reward_so_far() && curr_node != root)
                        best_node = curr_node;

                   curr_node -> generated_by_df = true;
                   curr_node -> must_be_prunned = false;
                   curr_node = curr_node -> get_parent(); 

                  //  std::cout <<"Sali3\n";
                }
                //std::cout <<"Sali2\n";
                curr_node -> must_be_prunned = false;
                //std::cout <<"Found some novelty\n" ;
                break;   
            }
            curr_node = succ;
         }

        if(nod->must_be_prunned) pruned += st;
    }
    return nod->must_be_prunned;
}

float IWRGB::run() {
    //std::cout <<"Va\n";
    reset();
    Node *curr_node  = root;

    std::vector<Node *> chs = curr_node->get_childs();
    for(int i= 0 ; i< chs.size(); i++) {
        //std::cout <<"Entre " << chs.size() << " " << chs[i]->tried << "\n";
        chs[i]->count_nodes();
    }
    q.push(curr_node);
    ActionVect v  = env->getMinimalActionSet();
    generated = 1;
    news = 0;
    pruned = 0 ;
    expanded = 0;
    //std::cout <<"Vax2\n";
    while(!q.empty()){
       curr_node = q.front();
       //std::cout << curr_node->get_reward_so_far() << "\n";
       q.pop();
       expanded ++;
       if (maximum_depth < curr_node->get_depth()) maximum_depth = curr_node->get_depth();
       if(best_node->get_reward_so_far() < curr_node->get_reward_so_far() && curr_node != root)
            best_node = curr_node;
       
       bool leaf = curr_node->get_childs().size() == 0;
       std::vector<Node *> succs;
       if(curr_node->get_depth() < max_depth / this -> fs){
            succs = curr_node->get_successors(env, true);
       }
       curr_node -> unset_count_in_novelty();

       for(int i =0 ; i < succs.size() && generated < max_lookahead / this->fs; i++){
           if(leaf) {
                generated ++;
			    if(!succs[i] -> generated_by_df && check_and_update_novelty(succs[i]) != 1 && dynamic_frame_skipping(succs[i])){
				    succs[i]->set_is_terminal(true);
                    succs[i]->tried++;
				    //pruned++;
			    } else {
                    succs[i]->tried = 0;
                    succs[i]->set_is_terminal(false);
                }
           } else{
                if(succs[i] -> get_is_terminal()){
                      if(succs[i] -> generated_by_df || check_and_update_novelty(succs[i]) == 1 || !dynamic_frame_skipping(succs[i])){
                        //add_to_novelty_table(fs);
                        succs[i]->set_is_terminal(false);
                      } else{
                        //pruned++;
                        succs[i]->set_is_terminal(true);
                      }
                }
            }
           if((!succs[i]->get_is_terminal() /*|| (leaf && succs[i]->tried * this->fs < 30)*/) && !succs[i]->test_duplicate() && succs[i]->reused_nodes < max_lookahead  / this->fs) q.push(succs[i]);
           else pruned++;
         }
            
    }
    std::cout<< "Best node at depth: " << best_node->get_depth() << ", reward:" << best_node -> get_reward_so_far() /*<< " " << best_node*/<< std::endl;
    std::cout<< "Generated nodes: " << generated << std::endl;
    std::cout <<"Expanded nodes:" << expanded << "\n";
    std::cout<< "Pruned nodes: " << pruned << std::endl;
    std::cout<< "Maximum depth: " << maximum_depth << std::endl;
    Node *tmp_node = best_node;
    while(best_node->get_depth() > 2) best_node = best_node->get_parent();
    Action best_act = best_node -> get_action();

    std::vector<Node *> ch = root->get_childs();
    restore_state(root, env);
    float rw = env->act(best_act);
    for(int i =0 ; i<ch.size(); i++) {
        if(ch[i] -> get_action() != best_act) 
            remove_tree(ch[i]);
        else {
            update_tree(ch[i], rw);
        }
    }
    root = best_node;
    best_node = tmp_node;
    if(root == best_node){ 
        best_node = new Node(NULL, v[rand() % v.size()], env->cloneState(), 1, 0, 1, get_feat(env, true));
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
        update_tree(ch[i], reward);
    nod->set_depth(nod->get_depth() - 1);
    nod->set_reward_so_far(reward);
}
int IWRGB::check_and_update_novelty( Node * nod){
    compute_features(nod);  
    return novelty(nod);
}

void IWRGB::reset_table(std::vector<bool> &table){
        for(int i =0 ; i < table.size(); i++) table[i] = false;
}

void IWRGB::reset_tables(){
    //if(table.size()) 
    //    reset_table(table); 
    /*else*/ if(this->features_type == 1)
        table = std::vector<bool> (k_total_basic_features, false);
    else if(this->features_type == 2)
        table = std::vector<bool> (k_total_basic_features + num_cross_features_, false);
    else if(this -> features_type == 3)
        table = std::vector<bool> (k_total_basic_features + num_cross_features_ + num_temporal_features_, false);
}

void IWRGB::compute_cross_features(std::vector<int> &screen_state_atoms) {
    std::vector<int> basic_features(screen_state_atoms);
    std::pair<std::pair<int, int>, int> f1, f2;
    for( size_t j = 0; j < basic_features.size(); ++j ) {
        //unpack_basic_feature(basic_features[j], f1);
        int start = j + 1;
        if(this -> features_type == 3) 
            start = 0;

        for( size_t k = start; k < basic_features.size(); ++k ) {
            //unpack_basic_feature(basic_features[k], f2);

            if(k > j){
                int pack = pack_cross_feature(basic_features[j], basic_features[k]);
                if( !table[pack] ) {
                    table[pack] = true;
                    screen_state_atoms.push_back(pack);
                }
            }
            
            if(this -> features_type == 3){
                int pack = pack_temporal_feature(basic_features[j], basic_features[k]);
                if( !table[pack] ) {
                    table[pack] = true;
                    screen_state_atoms.push_back(pack);
                }
            
            }
        }
    }
}

void IWRGB::compute_features(Node * nod){
    int depth = nod->get_depth();
    if(nod->features_computed)
        return;
    
    nod -> features_computed = true;
    nod -> basic_f.clear();
    reset_tables();
    std::vector<byte_t> &screen = nod->features;
    //std::cout <<"VA\n"; 
    for(int i=0; i<screen.size() && i < 33600; i++){
        int pixel_c = i % 160;
        int pixel_r = i / 160;
        
        int c = pixel_c / tile_column_size;
        int r = pixel_r / tile_row_size;
     //   std:: cout << tile_row_size << " " << tile_column_size << "\n";

        int pixel_val = (int) screen[i];
        //std::cout << pixel_val << "\n";
        
        int feature_idx = pack_basic_feature(c, r, pixel_val);

        //std::cout << feature_idx << " " << table.size() << "\n";
        assert(feature_idx < table.size());
        if(!is_background(pixel_c, pixel_r, pixel_val) && !table[feature_idx]){
            nod->basic_f.push_back(feature_idx);
            table[feature_idx] = true;
        }
    }

    if(this -> features_type >= 2){
        compute_cross_features(nod->basic_f);
    }
    //std::cout << nod->basic_f.size() << "\n";
}

int IWRGB::novelty(Node * nod/*, vp &patches*/){
    int nov = 1e9;
    std::map<int, int>::iterator it, it2, it3;
    basic_table_t fs_parent;
    Node *par;
    std::vector<int> &fs = nod -> basic_f;

    for(int i=0 ;i < fs.size();i++){
        assert(fs[i] < novelty_table.size());
        if(!novelty_table[fs[i]]){
            nov = 1;
            total_features++; 
            novelty_table[fs[i]] = true;
        }
    }
    return nov;
}
