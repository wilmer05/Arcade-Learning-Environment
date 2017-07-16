#include<queue>
#include <ale_interface.hpp>
#include"Node.hpp"
#ifdef __USE_SDL
    #include<SDL.h>
#endif
#include<utility>
#include <algorithm>
#include "rolloutsAlgorithm.hpp"
#include "constants.hpp"
#include "utils.hpp"

IWR::IWR(int ft, ALEInterface *ale, int fs, int tile_row_sz, int tile_column_sz, int delta) {
    features_type = ft;
    //q = std::queue<Node *>();
    env = ale;
    root = NULL;
    this -> fs = fs;
    tile_row_size = tile_row_sz;
    tile_column_size = tile_column_sz;
    
    c_number = 160 / tile_column_sz;
    r_number = 210 / tile_row_sz;
    maximum_depth = 0;
    displacement = delta;
    novelty_table_basic.clear();
    int cnt = 0;
    while(1){
        novelty_table_basic.push_back(std::vector< std::vector<std::vector<int> > >(c_number + 5, std::vector< std::vector<int> >(r_number + 5, std::vector<int>(k_different_colors, 1000000))));
        cnt++;
        if(cnt * displacement >= tile_row_size || !displacement) break;
    }

    number_of_tables = cnt;
    std::cout << "Screen splitted in " << novelty_table_basic[0][0].size() - 5<< " rows and " << novelty_table_basic[0].size() - 5<< " columns and there are " << cnt << " displacements\n" ;

    //std::cout <<novelty_table.size() << "\n";
}

/*void IWR::restore_state(Node *nod){
    Node * par = nod->get_parent();
    if(par != NULL){
        env->restoreState(par->get_state());
        env->act(nod->get_action());
     } else{
        env->restoreState(nod->get_state());
     }
}*/

void IWR::reset(){
    if(root == NULL) {
        ActionVect v  = env->getMinimalActionSet();
        root = new Node(NULL, v[0], 1, 0, 1);
        best_node = new Node(NULL, v[0], -5000000, -5000000, -5000000);
    }

    for(int d =0 ; d < number_of_tables; d++)
        for(int i = 0; i<c_number + 2;i++) 
            for(int j = 0; j < r_number + 2; j++) 
                for(int k = 0; k<k_different_colors;k++) 
                    novelty_table_basic[d][i][j][k]=1000000;

    if(compute_BPROS()){
         
        for(int i = 0; i<2 * k_novelty_columns;i++) 
            for(int j = 0; j < 2 * k_novelty_rows; j++) 
                for(int k1 = 0; k1<k_different_colors;k1++)  
                    for(int k2 = 0 ; k2<k_different_colors;k2++) novelty_table_bpros[k1][k2][i][j]= novelty_table_bprot[k1][k2][i][j] = 0;

    }
    total_features = 0;
    maximum_depth = 0;
}

float IWR::run() {
    reset();
    pruned = 0;
    expanded = 0;
    total_steps = 0;
    ALEState root_state = env->cloneState();
    ActionVect v  = env->getMinimalActionSet();
    Node * bla;
    //int cnt = 0;
    while(rollout() != root) {
        //env->act(v[0]);
        env->restoreState(root_state);
    }
    std::cout<< "Best node at depth: " << best_node->get_depth() << ", reward:" << best_node -> get_reward_so_far() << std::endl;
    //std::cout<< "Generated nodes: " << generated << std::endl;
    std::cout <<"Expanded nodes:" << expanded << "\n";
    std::cout<< "Pruned nodes: " << pruned << std::endl;
    std::cout<< "Maximum depth: " << maximum_depth << std::endl;
    Node *tmp_node = best_node;
    while(best_node->get_depth() > 2) best_node = best_node->get_parent();
    Action best_act = best_node -> get_action();

    std::vector<Node *> ch = root->get_childs();
    //restore_state(root, env);
    env->restoreState(root_state);

    float rw = env->act(best_act);
    //for(int i =0 ; i<ch.size(); i++) {
        //if(ch[i] != best_node) 
            remove_tree(root);
        /*else {
            //std::cout <<"Si hay un nuevo buen root\n";
            update_tree(ch[i], rw);
        }*/
    //}

    

    //Node *tmp_node = best_node;
    root = NULL;
    //best_node = tmp_node;
    //if(root == best_node){ 
        best_node = new Node(NULL, v[rand() % v.size()], -5000000, -5000000, -5000000);
    //    std::cout <<"Best node restarted\n";
    //}
    std::cout <<"Best action: " << best_act << std::endl;
    return rw;
}

bool IWR::solved_node(Node * n) { 
        if(n->solved) return true;
        std::vector<Node *> chs = n -> get_stateless_successors(env);
        bool solved = true;
        for(int i =0 ; i < chs.size() && solved; i++){
           solved = solved && chs[i]->solved; 
        }
        //std::cout <<solved << "\n";  
        return solved;
}

Node * IWR::propagate_solved(Node *nod){
    nod -> solved = true; 
    Node * last_solved = nod;
    while(nod != NULL && nod -> get_depth() >= 1){
        if(solved_node(nod)){
        //std::cout <<nod->get_depth() << "\n";
           nod->solved = true; 
           last_solved = nod;
           nod = nod->get_parent();
        } else break;
    }
    //std::cout <<last_solved << "\n";
    return last_solved;
} 

Node * IWR::rollout() {
    
    Node *curr_node  = root;
    while(1){

        if(!curr_node -> in_tree){ 
            expanded++;
            total_steps++;
        }

        curr_node -> in_tree = true;
        if(curr_node -> get_depth() >=  max_depth / this -> fs || solved_node(curr_node) || total_steps * this->fs >= max_lookahead){
            return propagate_solved(curr_node);
        }
        std::vector<Node *> chs = curr_node -> get_stateless_successors(env);
        random_shuffle(chs.begin(), chs.end());
        int i;
        for(i =0 ; i < chs.size(); i++){
           if(!chs[i]->solved){
                break; 
           } 
        }
        //std::cout << i << " " << chs.size() << "\n";
        float rw_so_far = curr_node->get_reward_so_far();
        curr_node = chs[i];
        float tmp_rw = env->act(curr_node -> get_action());
        //std::cout << env->getFrameNumber() << "\n";
        //std::cout << curr_node -> get_action() << "\n";
        //tmp_rw = env->act(curr_node -> get_action());

        float rw = rw_so_far + curr_node->get_discount() * tmp_rw;
        //std::cout <<env->getFrameNumber() << "\n";
        curr_node->set_rw(rw); 
        
        //std::cout << "DUCK\n";
        basic_table_t fs = get_features(); 
        //std::cout << nv << "\n";
        if(curr_node != root && novelty(curr_node, fs) != 1 && (!curr_node->in_tree || !equal_atom_flag)){
                pruned++;
                return propagate_solved(curr_node);
        }
        if(curr_node -> get_reward_so_far() >= best_node->get_reward_so_far()) best_node = curr_node; 
        //std::cout << "DUCK x 2\n";
    }

}

void IWR::remove_tree(Node * nod){
    std::vector<Node *> ch = nod->get_childs();
    for(int i =0 ; i< ch.size(); i++) remove_tree(ch[i]);
    delete nod;
}

/*void IWR::update_tree(Node *nod, float reward){
    std::vector<Node *> ch = nod->get_childs();
    for(int i =0 ; i< ch.size(); i++) 
        if(!ch[i]->get_is_duplicate())update_tree(ch[i], reward);
    nod->set_depth(nod->get_depth() - 1);
    nod->set_reward_so_far(reward);
}*/
int IWR::check_and_update_novelty( Node * nod){

    restore_state(nod, env);
    basic_table_t fs = get_features(); 

    return novelty(nod, fs);
}
basic_table_t IWR::get_features(){
    std::vector<byte_t> screen;
    env->getScreenGrayscale(screen);
    /*for(int i =0 ;i < screen.size(); i++) {
        std::cout << (int)screen[i] << " " ;
    }*/
    basic_table_t v(number_of_tables ,std::vector< std::vector<std::set<int> > >(c_number + 2, std::vector<std::set<int> >(r_number + 2, std::set<int>())));
    for(int d =0 ; d < number_of_tables; d++)
        for(int i=0; i<screen.size(); i++){

            int c = (i % 160) / tile_column_size;
            int r = (i / 160 + d * displacement) / tile_row_size;

            v[d][c][r].insert((int)screen[i]);

        } 
    return v;
}

int IWR::novelty(Node * nod, basic_table_t &fs){
    int nov = 1e9;
    equal_atom_flag = false;
    std::set<int>::iterator it, it2, it3;
//    std::cout << sz1 << " " << sz2 << "\n";
    basic_table_t fs_parent;
    Node *par;
    //std::cout <<"Duck\n";
    /*if(features_type == 3){
        par = nod;
        int cnt = 0;
        while(par != NULL && cnt++ < 4){
           par = par -> get_parent(); 
        }
        if(par != NULL){
            restore_state(par, env);
            fs_parent = get_features();
        }
    }*/
    for(int d =0 ; d < number_of_tables; d++){
        int sz1 = fs[d].size(); int sz2 = fs[d][0].size();
        for(int i =0 ; i< sz1;i++){
            for(int j = 0 ; j < sz2; j++){
                it = fs[d][i][j].begin();
                while(it != fs[d][i][j].end()){
                    if(novelty_table_basic[d][i][j][*it] == nod->get_depth()){
                        equal_atom_flag = true;
                    }
                    //std::cout <<nod->get_depth() << " " <<novelty_table_basic[d][i][j][*it] <<"\n";
                    if(novelty_table_basic[d][i][j][*it] > nod->get_depth()){
                        nov = 1;
                        novelty_table_basic[d][i][j][*it] = nod->get_depth();
                        total_features++;
                    }
                    //std::cout << i << " ---  " <<  j << "\n";
                    if(!d && compute_BPROS()){
                        //std::cout << "BPROS" << "\n";
                        for(int i2 =0 ; i2< sz1;i2++){
                            for(int j2 = 0 ; j2 < sz2; j2++){
                                if(features_type == 3 && par != NULL){
                                    it3 = fs_parent[0][i2][j2].begin();
                                    while(it3 != fs_parent[0][i2][j2].end()){
                                        int k1 = *it;
                                        int k2 = *it3;
                                        int cc = i - i2 + k_novelty_columns;
                                        int rr = j - j2 + k_novelty_rows;
                                        //std::cout << k1 << " " << k2 << " " << cc << " " << rr << "\n";
                                        if(novelty_table_bprot[k1][k2][cc][rr] == 0){
                                            //std::cout << "BPROS_NEW" << "\n";
                                            nov = 1;
                                        }
                                        novelty_table_bprot[k1][k2][cc][rr] = 1;
                                        it3++;
                                    }
                                }

                                if(j2 < j || (j2 == j && i2 <= i)) continue;
                                it2 = fs[d][i2][j2].begin();
                                while(it2 != fs[d][i2][j2].end()){
                                    int k1 = *it;
                                    int k2 = *it2;
                                    int cc = i - i2 + k_novelty_columns;
                                    int rr = j - j2 + k_novelty_rows;
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
    }

    //if(nov==1)
    //    std::cout << nov << " \n";
    return nov;
}
