#include<queue>
#include <ale_interface.hpp>
#include"Node.hpp"
#ifdef __USE_SDL
    #include<SDL.h>
#endif
#include<utility>
#include<set>
#include"constants.hpp"

typedef unsigned char byte_t;
typedef std::vector<std::vector< std::set<int> > > basic_table_t;
#define RAM_FEATURES 1
#ifndef DEF_IW_RGB
#define DEF_IW_RGB
static bool novelty_table_bpros[k_different_colors][k_different_colors][k_novelty_columns * 2][k_novelty_rows * 2];
static bool novelty_table_bprot[k_different_colors][k_different_colors][k_novelty_columns * 2][k_novelty_rows * 2];
class IWRGB{
    public:
        IWRGB() { 
            features_type = RAM_FEATURES; 
            q = std::queue<Node *>();
            maximum_depth = 0;
        } 

        IWRGB(int ft, ALEInterface *env, int fs);
       
        float run();

        int check_and_update_novelty(Node *);
        bool compute_BPROS(){
            return features_type == 2 || features_type == 3;
        }

    private:
        basic_table_t get_features();
        int novelty(Node *, basic_table_t &fs);
        void remove_tree(Node *); 
        void reset();
        void restore_state(Node *nod);
        bool novelty_table_basic[k_novelty_columns][k_novelty_rows][k_different_colors];
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

};
#endif
