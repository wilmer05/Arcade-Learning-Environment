#include<queue>
#include <ale_interface.hpp>
#include"Node.hpp"
#ifdef __USE_SDL
    #include<SDL.h>
#endif
#include<utility>
#include<set>


typedef unsigned char byte_t;

#define RAM_FEATURES 1
#ifndef DEF_IW
#define DEF_IW
class IW{
    public:
        IW() { 
            features_type = RAM_FEATURES; 
            q = std::queue<Node *>();
            maximum_depth = 0;
        } 

        IW(int ft, ALEInterface *env);
       
        float run();

        int check_and_update_novelty(Node *);

    private:
        std::vector<std::pair<int,byte_t> > get_features();
        int novelty(std::vector<std::pair<int,byte_t> > fs);
        void add_to_novelty_table( std::vector<std::pair<int,byte_t> > fs );
        void remove_tree(Node *); 
        void reset();
        void restore_state(Node *nod);
        int novelty_table[300][300];
        void update_tree(Node *, float);
        std::queue<Node *> q;
        int features_type;
        int maximum_depth;
        ALEInterface *env;
        Node * root;
        Node *best_node;

};
#endif
