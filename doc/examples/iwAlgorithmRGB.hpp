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
#include<functional>


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

struct ClassComparer{
    bool operator()(Node *n1, Node* n2){
        return n1->get_depth() > n2->get_depth() || (n1->get_depth() == n2->get_depth() && n1->get_reward_so_far() > n2->get_reward_so_far());
    }
};

class IWRGB{
    public:
        /*IWRGB() { 
            features_type = RAM_FEATURES; 
            q = std::queue<Node *>();
            maximum_depth = 0;
        } */

        IWRGB(int ft, ALEInterface *env, int fs, int tile_row_size, int tile_column_size, int delta);
       
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
        bool cache;
    private:

        //void reset_table(std::vector<int> &);
        void reset_table(std::vector<bool> &);

    protected:
        int generated;
        virtual void reset_tables();
        int news;
        double depth_sum;
        double depth_percentage;
        int pruned;
        int expanded;

        virtual void compute_features(Node *);
        virtual int novelty(Node *);
        void remove_tree(Node *); 
        virtual void reset();
        void free_the_memory(Node*);
        //void restore_state(Node *nod);
        //bool novelty_table_basic[k_novelty_columns][k_novelty_rows][k_different_colors];
        //std::vector< std::vector<std::vector< std::vector<float> > > > novelty_table_basic;
        std::vector<bool> novelty_table;
        void compute_cross_features(std::vector<int> &screen_state_atoms, Node *);
        //std::vector<bool> novelty_table_bpros;
        //std::vector<bool> novelty_table_bprot;

        std::vector<bool> table;
        //std::vector<bool> table_bpros;
        //std::vector<bool> table_bprot;
        similar_patches_t similarity_table;
        //bool novelty_table_bpros[k_different_colors][k_different_colors][k_novelty_columns * 2][k_novelty_rows * 2];
        //bool novelty_table_bpros[1][1][1][1];
        void update_tree(Node *, float);
        //std::queue<Node *> q;
        std::priority_queue<Node *, std::vector<Node* >, ClassComparer> q;
        std::stack<Action> my_stack;
        int features_type;
        int maximum_depth;
        int fs;
        ALEInterface *env;
        Node * root;
        Node *best_node;
        int tile_row_size;
        int step_number;
        int tile_column_size;
        int c_number;
        int r_number;
        int displacement;
        int number_of_tables;
        int number_of_displacements;
        int new_nodes;
        int total_features;
        int look_number;
        std::vector<std::pair<int,int> > pixels;

};
#endif
