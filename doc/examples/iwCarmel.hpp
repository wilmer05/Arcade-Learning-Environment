#include<queue>
#include <ale_interface.hpp>
#include"Node.hpp"
#ifdef __USE_SDL
    #include<SDL.h>
#endif
#include<utility>
#include<set>
#include"constants.hpp"
#include"iwAlgorithmRGB.hpp"
#include"utils.hpp"
#include <map>
#include <bitset>
#include<functional>


#define RAM_FEATURES 1
#ifndef DEF_IW_CARMEL
#define DEF_IW_CARMEL

#define select_features(nod) (nod->get_depth() == 2 ? nod->processed_screen : nod->differential_screen)
#define select_patches(nod) (nod ->get_depth() == 2 ? nod->patch_index : nod->differential_patch_index)

typedef unsigned char byte_t;
//typedef std::vector<std::vector<std::vector< std::map<int, int> > > > basic_table_t;

typedef std::vector<int> vi;
typedef std::vector<vi > vix2;
typedef std::vector<vix2 > vix3;
typedef std::vector<vix3 > vix4;

typedef vix4 similar_patches_t;
typedef std::vector<std::pair<int,int> > vp;

//std::vector<int> table_basic;
//std::vector<int> novelty_table_basic;


class IWCARMEL : public IWRGB{
    public:
        /*IWRGB() { 
            features_type = RAM_FEATURES; 
            q = std::queue<Node *>();
            maximum_depth = 0;
        } */

        IWCARMEL(int ft, ALEInterface *env, int fs, int tile_row_size, int tile_column_size, int delta) : IWRGB(ft, env, fs,tile_row_size, tile_column_size, delta) {

        }
      /* 
        virtual float run();

        int check_and_update_novelty(Node *);
        bool compute_BPROS(){
            return features_type == 2 || features_type == 3;
        }

        int get_total_features() { return total_features;} 

        bool neighbor_comparison() {
            return features_type == 5 || features_type == 7;
        }

        
        void init_similarity_talbe(int c_number, int r_number) {
            similarity_table = vix4(9, vix3(k_time_steps_comparison / this -> fs, vix2(c_number, vi(r_number, 0))));
        }

        bool dynamic_frame_skipping(Node *);
        void update_av_depth(Node *);
        float execute_action(Action);
        */
    private:

        /*void reset_table(std::vector<bool> &);
        void reset_tables();
        int generated;
        int news;
        double depth_sum;
        double depth_percentage;
        int pruned;
        int expanded;
        */
       // virtual void compute_features(Node *);

        void reset_table(std::vector<float> &v){
            std::fill(v.begin(), v.end(), -100000.0);
        }

        virtual void reset(){
        
            int times = 0;
            
            if(novelty_table.size() > 0 )
                reset_table(carmel_novelty_table);
            else if(this->features_type == 1 || this->features_type == 4)
                carmel_novelty_table = std::vector<float>(k_total_basic_features << times, -100000.0);
            else if(this -> features_type == 2){
                carmel_novelty_table = std::vector<float>((k_total_basic_features + num_cross_features_) << times, -100000.0);
            }
            else if(this -> features_type == 3){
                carmel_novelty_table = std::vector<float>((k_total_basic_features + num_cross_features_ + num_temporal_features_) << times, -100000.0);
            }
            
            if(root == NULL) {
                ActionVect v  = env->getMinimalActionSet();
                std::vector<byte_t> feat =  get_feat(env, true);
                root = new Node(NULL, v[rand() % v.size()], env->cloneState(), 1, 0, 1, feat);
                best_node = root;
                //best_node = new Node(NULL, v[rand() % v.size()], env->cloneState(), 1, 0, 1, feat);
            }
            
            //if(neighbor_comparison()) init_similarity_talbe(c_number + 5, r_number + 5);
            
            total_features = 0;
            maximum_depth = 0;
            depth_sum = 0;
            depth_percentage = 0;
        
        
        }
        virtual int novelty(Node *nod){

            int nov = 1e9;
            std::map<int, int>::iterator it, it2, it3;
            basic_table_t fs_parent;
            Node *par;
            std::vector<int> &fs = nod -> basic_f;

            /*int log_score = this->features_type == 4 ? 1 : 0;
            float rw = nod->get_reward_so_far();

            int times = this->features_type == 4 ? k_log_scores : 0;

            int cnt = 0;
            while(times > 0 && log_score < rw) log_score <<= 1, cnt++;

            if(cnt >= (1 << k_log_scores)) cnt = (1<<k_log_scores) - 1;
*/

            //std::cout << log_score << " " << cnt <<"\n";
            //std::cout << novelty_table.size() << "\n";
            double rw_so_far = nod->get_reward_so_far();
            for(int i=0 ;i < fs.size();i++){
                int tmp = fs[i];
                //tmp <<= times;
                //tmp += cnt;
                //std::cout << log_score << " " << cnt <<"\n";
                assert(tmp < carmel_novelty_table.size());
                if(carmel_novelty_table[tmp] < rw_so_far){
                    nov = 1;
                    total_features++;
                    carmel_novelty_table[tmp] = rw_so_far;
                }
            }
            return nov;    
        
        }
        /*void remove_tree(Node *); 
        void reset();
        void free_the_memory(Node*);
        */

        

        std::vector<float> carmel_novelty_table;
        //std::vector<bool> novelty_table;
        //void compute_cross_features(std::vector<int> &screen_state_atoms, Node *);
        //std::vector<bool> novelty_table_bpros;
        //std::vector<bool> novelty_table_bprot;

        //std::vector<float> table_f;
        //std::vector<bool> table;
        //std::vector<bool> table_bpros;
        //std::vector<bool> table_bprot;
        //similar_patches_t similarity_table;
        //bool novelty_table_bpros[k_different_colors][k_different_colors][k_novelty_columns * 2][k_novelty_rows * 2];
        //bool novelty_table_bpros[1][1][1][1];
        //void update_tree(Node *, float);
        //std::queue<Node *> q;
        //std::priority_queue<Node *, std::vector<Node* >, ClassComparer> q;
        //std::stack<Action> my_stack;
        //int features_type;
        //int maximum_depth;
        //int fs;
        //ALEInterface *env;
        //Node * root;
        //Node *best_node;
        //int tile_row_size;
        //int step_number;
        //int tile_column_size;
        //int c_number;
        //int r_number;
        //int displacement;
        //int number_of_tables;
        //int number_of_displacements;
        //int new_nodes;
        //int total_features;
        //std::vector<std::pair<int,int> > pixels;

};
#endif
