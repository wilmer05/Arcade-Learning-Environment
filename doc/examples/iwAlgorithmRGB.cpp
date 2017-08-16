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
    look_number = 0;
    //q = std::queue<Node *>();
    //q = std::priority_queue<Node *, std::vector<Node* >, bool (*) (Node*, Node*) >(&my_comparer);
    env = ale;
    root = NULL;
    step_number = 0;
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

    int times = 0;
    if(this->features_type == 4) times = k_log_scores;

    if(novelty_table.size() > 0 )
        reset_table(novelty_table);
    else if(this->features_type == 1 || this->features_type == 4)
        novelty_table = std::vector<bool>(k_total_basic_features << times, false);
    else if(this -> features_type == 2){
        novelty_table = std::vector<bool>((k_total_basic_features + num_cross_features_) << times, false);
    }
    else if(this -> features_type == 3){
        novelty_table = std::vector<bool>((k_total_basic_features + num_cross_features_ + num_temporal_features_) << times, false);
    }

    if(root == NULL) {
        ActionVect v  = env->getMinimalActionSet();
        std::vector<byte_t> feat =  get_feat(env, true);
        root = new Node(NULL, v[rand() % v.size()], new ALEState(env->cloneState()), 1, 0, 1, feat);
        best_node = root;
        root->generated_at_step = look_number;
        //best_node = new Node(NULL, v[rand() % v.size()], env->cloneState(), 1, 0, 1, feat);
    }

    //if(neighbor_comparison()) init_similarity_talbe(c_number + 5, r_number + 5);

    total_features = 0;
    maximum_depth = 0;
    depth_sum = 0;
    depth_percentage = 0;
    //std::cout << novelty_table.size() << "\n";
}

void IWRGB::update_av_depth(Node *nod){
    
    depth_sum += nod->get_depth();

}

bool IWRGB::dynamic_frame_skipping(Node *nod){
    //std::cout << "DS CALLED\n" << "\n";
    if(nod->generated_by_df && nod->childs.size() == 1) return false;
    Action a = nod -> get_action();
    int steps = 30 / this->fs - 1;
    

    nod->must_be_prunned = true;
    //std::cout << "Entre\n";
    if(!nod->childs.size()){
         Node *curr_node = nod; 
         int st = 0;
         for(st =0 ; st < steps && st + generated < max_lookahead / this->fs; st++){
            
            Node *succ = curr_node->generate_child_with_same_action(env, true);
            if(succ == NULL) break;
            update_av_depth(succ);
            generated++ ;
            new_nodes++;
            //compute_features(succ);
            if(check_and_update_novelty(succ) == 1){

            //if(nod->basic_f != succ->basic_f){
                curr_node = succ;
                while(curr_node != nod){
                    if (maximum_depth < curr_node->get_depth()) maximum_depth = curr_node->get_depth();
                    if(best_node->get_reward_so_far() < curr_node->get_reward_so_far() && curr_node != root)
                        best_node = curr_node;

                   curr_node -> generated_by_df = true;
                   curr_node -> must_be_prunned = false;
                   curr_node = curr_node -> get_parent(); 

                }
                curr_node -> must_be_prunned = false;
                break;   
            }
            curr_node = succ;
         }

        if(nod->must_be_prunned) pruned += st;
    }
    return nod->must_be_prunned;
}

float IWRGB::execute_action(Action best_act){
    std::vector<Node *> ch = root->get_childs();
    float rw = env->act(best_act);
    Node *new_root;
    for(int i =0 ; i<ch.size(); i++) {
        if(ch[i] -> get_action() != best_act) 
            remove_tree(ch[i]);
        else {
            update_tree(ch[i], rw);
            ch[i] -> parent = NULL;
            new_root = ch[i];
        }
    }

    if(rw != 0.0)
        while(!my_stack.empty()) my_stack.pop();
    delete root;
   // std::cout << best_node -> get_reward_so_far() << "=reward of best_node\n";
    root = new_root;
    return rw;
}

float IWRGB::run() {
    //std::cout <<"Va\n";
    ALEState root_state = env->cloneState();
    look_number++;
    if(my_stack.size()){
        Action b_act = my_stack.top();
        my_stack.pop();
        std::cout << "Using action from stack: " << b_act <<"\n"; 
        return execute_action(b_act);
    }
    reset();
    Node *curr_node = root;

    std::vector<Node *> chs = curr_node->get_childs();
    int total_reused = 0;
    for(int i= 0 ; i< chs.size(); i++) {
        //std::cout <<"Entre " << chs.size() << " " << chs[i]->tried << "\n";
        chs[i]->count_nodes(look_number);
        //std::cout << "#########" ;
        //std::cout << chs[i] -> reused_nodes << "\n";
        total_reused += chs[i] -> reused_nodes;
    }
    q.push(curr_node);
    ActionVect v  = env->getMinimalActionSet();
    generated = 1;
    news = 0;
    pruned = 0 ;
    expanded = 0;
    new_nodes = 0;
    //std::cout <<"Vax2\n";
    //std::cout << max_lookahead / this->fs;
    while(!q.empty()){
        //curr_node = q.front();
        curr_node = q.top();
        //std::cout << curr_node -> get_reward_so_far() << " " << curr_node->get_depth() << "\n";
//std::cout << curr_node->get_reward_so_far() << "\n";
        q.pop();
        expanded ++;
        if (maximum_depth < curr_node->get_depth()) maximum_depth = curr_node->get_depth();
        if(best_node->get_reward_so_far() < curr_node->get_reward_so_far() && curr_node != root)
            best_node = curr_node;
        
        
        bool leaf = curr_node->get_childs().size() == 0;
        if(leaf && generated >= max_lookahead / this->fs) continue;
        std::vector<Node *> succs;
        if(curr_node->get_depth() < max_depth / this -> fs){
            succs = curr_node->get_successors(env, true, look_number);
            if(curr_node->get_state_address() != NULL){
                curr_node->null_state_address();
            }
        }
        
        if(leaf) new_nodes += succs.size();
        curr_node -> unset_count_in_novelty();
        
        for(int i =0 ; i < succs.size() && generated + succs.size() < max_lookahead / this->fs; i++){
           if(leaf) {
                generated++;
                update_av_depth(succs[i]);
                if(succs[i] -> test_duplicate()){
                    pruned++;
        
                    succs[i] -> set_is_terminal(true);
                    continue;
                }
        
                if(!succs[i] -> generated_by_df && check_and_update_novelty(succs[i]) != 1 && dynamic_frame_skipping(succs[i])){
                    succs[i]->set_is_terminal(true);
                    succs[i]->tried++;
                    //pruned++;
                } else {
                    succs[i]->tried = 0;
                    succs[i]->set_is_terminal(false);
                }
           } else{
                
                if(succs[i] -> get_is_terminal()) {
            
                        if(succs[i] -> test_duplicate()){
                            pruned++;
                            succs[i] -> set_is_terminal(true);
                            continue;
                        }
        
        
        
                      if(succs[i] -> generated_by_df || check_and_update_novelty(succs[i]) == 1 || !dynamic_frame_skipping(succs[i])){
                        //add_to_novelty_table(fs);
                        succs[i]->set_is_terminal(false);
                        free_the_memory(succs[i]);
                      } else{
                        //pruned++;
                        succs[i]->set_is_terminal(true);
                      }
                } else if(!succs[i]->freed){
                    free_the_memory(succs[i]);
                }
            }
           if((!succs[i]->get_is_terminal() /*|| (leaf && succs[i]->tried * this->fs < 30)*/) && !succs[i]->test_duplicate() && succs[i]->reused_nodes < max_lookahead  / this->fs)  q.push(succs[i]);
           else pruned++;
         }
        
    }
    std::cout<< "Best node at depth: " << best_node->get_depth() << ", reward:" << best_node -> get_reward_so_far() /*<< " " << best_node*/<< std::endl;
    std::cout<< "Generated nodes: " << generated << std::endl;
    std::cout <<"Expanded nodes:" << expanded << "\n";
    std::cout<< "Pruned nodes: " << pruned << std::endl;
    std::cout<< "Maximum depth: " << maximum_depth << std::endl;
    std::cout << "New generated nodes: " << new_nodes << std::endl;
    if(generated == 0) generated = 1;
    std::cout << "Average depth: " << depth_sum / generated << "\n";
    std::cout << "Reused nodes: " << total_reused << "\n";
    Node *tmp_node = best_node;
    bool push_in_stack = best_node->get_reward_so_far() != 0.0 && this->features_type != 1 && this->features_type != 4;
    while(best_node->get_depth() > 2) {
        //if(push_in_stack) my_stack.push(best_node->get_action());
        best_node = best_node->get_parent();
    }
    Action best_act = best_node -> get_action();

    std::vector<Node *> ch = root->get_childs();
    //restore_state(root, env);
    env->restoreState(root_state);
    float rw = env->act(best_act);
    for(int i =0 ; i<ch.size(); i++) {
        if(ch[i] -> get_action() != best_act) 
            remove_tree(ch[i]);
        else {
            update_tree(ch[i], rw);
            ch[i] -> parent = NULL;
        }
    }
    Node *tmp2 = root;
    root = best_node;
    best_node = tmp_node;
    if(root == best_node){ 
        best_node = new Node(NULL, v[rand() % v.size()], new ALEState(env->cloneState()), 1, 0, 1, get_feat(env, true));
        root = best_node;
        root -> generated_at_step = look_number;
        std::cout <<"Best node restarted\n";
    }
//    std::cout << root << " " << tmp2 << "\n";
    delete tmp2;

    std::cout <<"Best action: " << best_act << std::endl;
    return rw;
}

void IWRGB::free_the_memory(Node *nod){
    int sz = nod->basic_f.size();
    if(!sz) return;
    int idx=0;

    //std::cout << "freeing " << nod->basic_f.size() <<"\n";
    //std::cout << nod->basic_f[sz];
    while(idx < sz && is_basic_feature(nod->basic_f[idx])){
        idx++; 
    }
    nod->basic_f.resize(idx);
    //std::cout << nod->basic_f.size() << "\n";
    nod ->freed = true;
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

void IWRGB::reset_table(std::vector<bool> &tab){
        std::fill(tab.begin(), tab.end(), false);
        //for(int i =0 ; i < table.size(); i++) table[i] = false;
}

inline void IWRGB::reset_tables(){
    if(table.size()) 
        reset_table(table); 
    else if(this->features_type == 1 || this->features_type == 4)
        table = std::vector<bool> (k_total_basic_features, false);
    else if(this->features_type == 2)
        table = std::vector<bool> (k_total_basic_features + num_cross_features_, false);
    else if(this -> features_type == 3)
        table = std::vector<bool> (k_total_basic_features + num_cross_features_ + num_temporal_features_, false);
}

void IWRGB::compute_cross_features(std::vector<int> &screen_state_atoms, Node *nod) {
    Node *par = NULL;
    if(this -> features_type == 3){
        int cnt = 3;
        par = nod;
        while(par != NULL && cnt >0){
            par = par->get_parent();
            cnt--;
        }
    }

    std::vector<int> basic_features(screen_state_atoms);
    std::pair<std::pair<int, int>, int> f1, f2;
    for( size_t j = 0; j < basic_features.size(); ++j ) {
        //unpack_basic_feature(basic_features[j], f1);
        for( size_t k = j + 1; k < basic_features.size(); ++k ) {
            //unpack_basic_feature(basic_features[k], f2);

            int pack = pack_cross_feature(basic_features[j], basic_features[k]);
            if( !table[pack] ) {
                table[pack] = true;
                screen_state_atoms.push_back(pack);
            }
        } 

        if(par != NULL)
            for(int k = 0; k < par->basic_f.size() && is_basic_feature(par->basic_f[k]); k++){
                int pack = pack_temporal_feature(basic_features[j], par->basic_f[k]);
                if( !table[pack] ) {
                    table[pack] = true;
                    screen_state_atoms.push_back(pack);
                } else break;
            }
        
    }
}

void IWRGB::compute_features(Node * nod){
    int depth = nod->get_depth();
    if(nod->features_computed)
        return;
    
    nod -> features_computed = true;
    nod -> basic_f.clear();
    //if(nod == root)
    reset_tables();
    std::vector<byte_t> &screen = nod->features;
    //std::cout <<"VA\n"; 
   int pixel_c = 0;
   int pixel_r = 0;

    for(int i=0; i<screen.size() && i < 33600; i++){
        //int pixel_c = i % 160;
        //int pixel_r = i / 160;
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
        pixel_c ++;
        if(pixel_c >= 160 ){
            pixel_c = 0;
            pixel_r++;
        }
    }

    if(this -> features_type >= 2 && this->features_type < 4){
        compute_cross_features(nod->basic_f, nod);
    }
    screen.clear();
    //std::cout << nod->basic_f.size() << "\n";
}

int IWRGB::novelty(Node * nod/*, vp &patches*/){
    int nov = 1e9;
    std::map<int, int>::iterator it, it2, it3;
    basic_table_t fs_parent;
    Node *par;
    std::vector<int> &fs = nod -> basic_f;

    int log_score = this->features_type == 4 ? 1 : 0;
    float rw = nod->get_reward_so_far();

    int times = this->features_type == 4 ? k_log_scores : 0;

    int cnt = 0;
    while(times > 0 && log_score < rw) log_score <<= 1, cnt++;
    
    if(cnt >= (1 << k_log_scores)) cnt = (1<<k_log_scores) - 1;


    //std::cout << log_score << " " << cnt <<"\n";
    //std::cout << novelty_table.size() << "\n";
    for(int i=0 ;i < fs.size();i++){
        int tmp = fs[i];
        tmp <<= times;
        tmp += cnt;
        //std::cout << log_score << " " << cnt <<"\n";
        assert(tmp < novelty_table.size());
        if(!novelty_table[tmp]){
            nov = 1;
            total_features++; 
            novelty_table[tmp] = true;
        }
    }
    return nov;
}
