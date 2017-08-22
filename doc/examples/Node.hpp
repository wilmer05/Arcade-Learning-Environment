#include <ale_interface.hpp>
#include <vector>
#ifdef __USE_SDL
    #include <SDL.h>
#endif
#include <bitset>
#include"constants.hpp"
#ifndef DEF_NODE
#define DEF_NODE

typedef std::vector<std::vector<std::vector< std::map<int, int> > > > basic_table_t;

class Node{
    public:

        Node(Node* par, Action act, ALEState *ale_state, int d, double rew, double discs, std::vector<byte_t> r);

        Node(Node* par, Action act, int d, double rew, double discs);

        std::vector<Node *> get_successors(ALEInterface *env, bool, int);

        std::vector<Node *> get_stateless_successors(ALEInterface *env);
        ALEState get_state();
        ALEState *get_state_address(){
            return this->state;
        }
        
        void null_state_address(){
            delete this->state;
            this->state = NULL;
        }
        double get_reward_so_far(){ return reward_so_far; }
        void set_reward_so_far(float substract){ 
            reward_so_far /= discount_reward;
            reward_so_far -= substract; 
            discount /= discount_reward;
        }
        void set_rw(float rw){ 
            reward_so_far = rw;
        }

        Node *generate_child_with_same_action(ALEInterface *, bool);

        void add_tried() {tried ++;} 
        int get_depth() { return depth; } 

        void set_depth(int val) { depth = val; } 
        Node * get_parent() { return parent;}

        Action get_action() { return action; } 
        bool get_count_in_novelty() {return count_in_novelty;} 
        void unset_count_in_novelty() { count_in_novelty = false; }
        std::vector<Node *> &get_childs(){ return childs; }
        void restore_state(Node *nod, ALEInterface *env);
        bool get_is_terminal() {return is_terminal;}
        void set_is_terminal(bool v) { is_terminal = v;}
        double get_discount() { return discount; }

        bool get_is_duplicate() { return is_duplicate;}
        void set_is_duplicate(bool v) { is_duplicate = v; }
        bool test_duplicate();
        std::vector<Node *> childs;
        int count_nodes(int);
        int count_nodes();
        bool freed;
        int reused_nodes;
        int tried;
        ActionVect childActs;
        bool solved;
        bool in_tree;
        bool must_be_prunned;
        bool tested_duplicate;
        std::vector<byte_t> features;
        basic_table_t processed_screen;
        basic_table_t differential_screen;
        bool features_computed;
        std::vector<std::pair<int, int> > patch_index, differential_patch_index;
        bool generated_by_df;
        std::vector<int> basic_f;
        //std::bitset<k_novelty_columns * k_novelty_rows * k_different_c    olors> patch_f;
        Node *parent;
        int generated_at_step;
    private:

        ALEState *state;
        int depth;
        double reward_so_far;
        //void free_memory();
        Action action;
        double discount;
        bool count_in_novelty;
        bool is_terminal;
        bool is_duplicate;
};
#endif
