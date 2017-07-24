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
    novelty_table_basic.clear();
    int cnt = 0;
    while(1){
        novelty_table_basic.push_back(std::vector< std::vector<std::vector<float> > >(c_number + 5, std::vector< std::vector<float> >(r_number + 5, std::vector<float>(k_different_colors, -1000.0))));
        cnt++;
        //if(features_type == 5 && cnt < 10) continue;
        if(features_type >= 6 && cnt < 4) continue;
        //if(features_type == 7 && cnt < 14) continue;
        if(features_type >= 5 || cnt * displacement >= tile_row_size || !displacement) break;
    }

    if(neighbor_comparison())
        init_similarity_talbe(c_number + 5, r_number + 5);


    number_of_tables = cnt;
    number_of_displacements = cnt;

    pixels = get_meaninful_pixels(env);

    if(features_type >= 5) number_of_displacements = 1;
    std::cout << "Screen splitted in " << novelty_table_basic[0][0].size() - 5<< " rows and " << novelty_table_basic[0].size() - 5<< " columns and there are " << number_of_displacements << " displacements\n" ;
    std::cout << "Steps for patches comparison: " << k_time_steps_comparison / this ->fs << "\n";
    //std::cout <<novelty_table.size() << "\n";
}

/*void IWRGB::restore_state(Node *nod){
    Node * par = nod->get_parent();
    if(par != NULL){
        env->restoreState(par->get_state());
        env->act(nod->get_action());
     } else{
        env->restoreState(nod->get_state());
     }
}*/

void IWRGB::reset(){
    if(root == NULL) {
        ActionVect v  = env->getMinimalActionSet();
        std::vector<byte_t> feat =  get_feat(env, true);
        root = new Node(NULL, v[0], env->cloneState(), 1, 0, 1, feat);
        best_node = new Node(NULL, v[0], env->cloneState(), -5000000, -5000000, -5000000, feat);
    }

    for(int d =0 ; d < number_of_tables; d++)
        for(int i = 0; i<c_number + 2;i++) 
            for(int j = 0; j < r_number + 2; j++) 
                for(int k = 0; k<k_different_colors;k++){ 
                    novelty_table_basic[d][i][j][k]=-1000.0;
                    if(features_type == 4) novelty_table_basic[d][i][j][k]=-1000000;
                }

    if(compute_BPROS()){
         
        for(int i = 0; i<2 * k_novelty_columns;i++) 
            for(int j = 0; j < 2 * k_novelty_rows; j++) 
                for(int k1 = 0; k1<k_different_colors;k1++)  
                    for(int k2 = 0 ; k2<k_different_colors;k2++) novelty_table_bpros[k1][k2][i][j]= novelty_table_bprot[k1][k2][i][j] = -1000.0;

    }

    if(neighbor_comparison()) init_similarity_talbe(c_number + 5, r_number + 5);

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
                while(curr_node != nod){
                    if (maximum_depth < curr_node->get_depth()) maximum_depth = curr_node->get_depth();
                    if(best_node->get_reward_so_far() < curr_node->get_reward_so_far() && curr_node != root)
                        best_node = curr_node;


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
			    if (check_and_update_novelty(succs[i]) != 1 && dynamic_frame_skipping(succs[i])){
				    succs[i]->set_is_terminal(true);
                    succs[i]->tried++;
				    //pruned++;
			    } else {
                    succs[i]->tried = 0;
                    succs[i]->set_is_terminal(false);
                }
           } else{
                if(succs[i] -> get_is_terminal()){
                      if(check_and_update_novelty(succs[i]) == 1 || !dynamic_frame_skipping(succs[i])){
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
        if(ch[i] != best_node) 
            remove_tree(ch[i]);
        else {
            update_tree(ch[i], rw);
        }
    }
    root = best_node;
    best_node = tmp_node;
    if(root == best_node){ 
        best_node = new Node(NULL, v[rand() % v.size()], env->cloneState(), -5000000, -5000000, -5000000, get_feat(env, true));
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

    //restore_state(nod, env);
    basic_table_t fs = get_features(nod); 

    return novelty(nod, fs);
}
basic_table_t IWRGB::get_features(Node * nod){
    std::vector<byte_t> &screen = nod->features;
    //env->getScreenGrayscale(screen);
    //std::cout << "entre get_feat\n"; 
    basic_table_t v(number_of_displacements ,std::vector< std::vector<std::map<int, int> > >(c_number + 5, std::vector<std::map<int, int > >(r_number + 5, std::map<int, int>())));
    for(int d =0 ; d < number_of_displacements; d++)
        for(int i=0; i<pixels.size(); i++){
            int pixel_c = pixels[i].second;
            int pixel_r = pixels[i].first;

            int c = (pixel_c) / tile_column_size;
            int r = (pixel_r + d * displacement) / tile_row_size;
            
            if(c > c_number || r > r_number){
                //std::cout <<"WTF\n";
                continue;
            }
            //std::cout << c << " " << r << " -- " << c_number << " " << r_number << " " << screen.size() << "\n";
            

            if(!v[d][c][r].count((int)screen[i]))
                v[d][c][r][(int)screen[i]] = 1;
            else 
                v[d][c][r][(int)screen[i]]++;
            //v[d][c][r].insert((int)screen[i]);

        } 
    //std::cout << "sali get_feat\n"; 
    return v;
}

int IWRGB::novelty(Node * nod, basic_table_t &fs){
    int nov = 1e9;
    std::map<int, int>::iterator it, it2, it3;
//    std::cout << sz1 << " " << sz2 << "\n";
    basic_table_t fs_parent;
    Node *par;
    //std::cout <<"Entre\n";
    if(features_type == 3 || neighbor_comparison()){
        par = nod;
        int cnt = 0;
        while(par != NULL && cnt++ < 4){
           par = par -> get_parent(); 
        }
        if(par != NULL){
            //restore_state(par, env);
            fs_parent = get_features(par);
        }
    }

    float rw_so_far = nod->get_reward_so_far();
    int sz1 = fs[0].size(); int sz2 = fs[0][0].size();
    for(int d =0 ; d < number_of_displacements; d++){
        for(int i =0 ; i< sz1;i++){
            for(int j = 0 ; j < sz2; j++){
                it = fs[d][i][j].begin();
                while(it != fs[d][i][j].end()){
                    if((features_type < 4 || (features_type == 5 && !d)) && novelty_table_basic[d][i][j][it->first] == -1000.0){
                        nov = 1;
                        total_features++;
                        novelty_table_basic[d][i][j][it->first] = 1.0;
                    }
                    /*else if(features_type == 4 && novelty_table_basic[d][i][j][it -> first] < (int) ceil(nod->get_reward_so_far())){
                        //std::cout << "Era: " << novelty_table_basic[d][i][j][*it] << " y ahora: " << (int) ceil(nod->get_reward_so_far()) ;
                        novelty_table_basic[d][i][j][it -> first] = (int)ceil(nod->get_reward_so_far());
                        nov = 1;
                        total_features++;
                    } */else if( (features_type == 6 || features_type == 7) && !d){
                        int begin = 0;

                        int cnt = it -> second;
                        int color_relevance  = k_non_existent_color;
                        int total_pixels = tile_row_size * tile_column_size;
                        if(cnt && cnt * (100/k_several_existent_color_percentage) < total_pixels) color_relevance = k_few_existent_color;
                        else if(cnt && cnt * (100/k_relevant_color_percentage) < total_pixels) color_relevance = k_several_existent_color;
                        else if(cnt) color_relevance = k_relevant_color;

                        //std::cout <<"Entre2\n";
                        if(novelty_table_basic[begin + color_relevance][i][j][it->first] == 0){
                            nov = 1;
                            novelty_table_basic[begin + color_relevance][i][j][it->first]=1;
                            total_features++;
                        }
                        //std::cout <<"Sali2\n";
                    }
                    
                    //std::cout << i << " ---  " <<  j << "\n";
                    if(!d && compute_BPROS()){
                        //std::cout << "BPROS" << "\n";
                        for(int i2 =0 ; i2< sz1;i2++){
                            for(int j2 = 0 ; j2 < sz2; j2++){
                                if(features_type == 3 && par != NULL){
                                    it3 = fs_parent[0][i2][j2].begin();
                                    while(it3 != fs_parent[0][i2][j2].end()){
                                        int k1 = it -> first;
                                        int k2 = it3 -> first;
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
                                    int k1 = it -> first;
                                    int k2 = it2 -> first;
                                    int cc = i - i2 + k_novelty_columns;
                                    int rr = j - j2 + k_novelty_rows;
                                    if(novelty_table_bpros[k1][k2][cc][rr]  == -1000.0){
                                        //std::cout << "BPROS_NEW" << "\n";
                                        nov = 1;
                                        novelty_table_bpros[k1][k2][cc][rr] = 1.0;
                                    }
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

    if(neighbor_comparison()){
        par = nod;

        for(int t = 0; t < k_time_steps_comparison / this->fs; t++){
            par = par->get_parent();
            
            if(par == NULL) break;
            //restore_state(par, env);
            fs_parent = get_features(par);

            for(int i = 0 ; i< sz1; i++)
                for(int j = 0 ; j < sz2; j++) {
                    int most_similar = 0;
                    double similarity = 100000000.0;

                    for(int k = 0 ; k < 9; k++){
                        double dist = 0.;
                        int dx = k/3 - 1;
                        int dy = k%3 - 1;
                        it = fs[0][i][j].begin();
                        if( i + dx < 0 || j + dy < 0 || i + dx > sz1 || j + dy > sz2) continue;
                        while(it!=fs[0][i][j].end()){

                           //std::cout << "Entrex2\n";
                           dist += pow(it->second - fs_parent[0][i+dx][j+dy][it->first], 2.0);

                           it++;
                       }                                         
                       
                            //std::cout <<"BLA2\n";
                      if(dist < similarity){
                       //std::cout << dist << "\n";
                           similarity = dist;
                           most_similar = k;
                      }
                       
                    }
                    //std::cout << "Most similar: " << most_similar << " " << similarity << "\n";
                    if(similarity_table[most_similar][t][i][j] == 0){
                            similarity_table[most_similar][t][i][j] = 1;
                            //std::cout << most_similar << " " << t << " " << i << " " << j << "\n";
                            nov = 1;
                            total_features++;
                            //std::cout <<"Entre aca\n";
                    } 

                  
                }
            }

    }

    //if(nov==1)
//    std::cout << "Sali\n";
    return nov;
}
