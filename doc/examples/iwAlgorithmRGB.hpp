#include<queue>
#include <ale_interface.hpp>
#include"Node.hpp"
#ifdef __USE_SDL
    #include<SDL.h>
#endif
#include<utility>
#include<set>
#include"constants.hpp"
#include"utils.hpp"
#include <map>
#include <bitset>


#define RAM_FEATURES 1
#ifndef DEF_IW_RGB
#define DEF_IW_RGB

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
class IWRGB{
    public:
        /*IWRGB() { 
            features_type = RAM_FEATURES; 
            q = std::queue<Node *>();
            maximum_depth = 0;
        } */

        IWRGB(int ft, ALEInterface *env, int fs, int tile_row_size, int tile_column_size, int delta);
       
        float run();

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

    private:

        //void reset_table(std::vector<int> &);
        void reset_table(std::vector<bool> &);
        void reset_tables();
        int generated;
        int news;
        int pruned;
        int expanded;

        void compute_features(Node *);
        int novelty(Node *);
        void remove_tree(Node *); 
        void reset();
        //void restore_state(Node *nod);
        //bool novelty_table_basic[k_novelty_columns][k_novelty_rows][k_different_colors];
        //std::vector< std::vector<std::vector< std::vector<float> > > > novelty_table_basic;
        std::vector<bool> novelty_table;
        void compute_cross_features(std::vector<int> &screen_state_atoms);
        //std::vector<bool> novelty_table_bpros;
        //std::vector<bool> novelty_table_bprot;

        std::vector<bool> table;
        //std::vector<bool> table_bpros;
        //std::vector<bool> table_bprot;
        similar_patches_t similarity_table;
        //bool novelty_table_bpros[k_different_colors][k_different_colors][k_novelty_columns * 2][k_novelty_rows * 2];
        //bool novelty_table_bpros[1][1][1][1];
        void update_tree(Node *, float);
        std::queue<Node *> q;
        int features_type;
        int maximum_depth;
        int fs;
        ALEInterface *env;
        Node * root;
        Node *best_node;
        int tile_row_size;
        int tile_column_size;
        int c_number;
        int r_number;
        int displacement;
        int number_of_tables;
        int number_of_displacements;
        int total_features;
        std::vector<std::pair<int,int> > pixels;

};
#endif
