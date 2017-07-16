#include <ale_interface.hpp>
#include <vector>
#ifdef __USE_SDL
    #include <SDL.h>
#endif
#include"constants.hpp"
#ifndef DEF_NODE
#define DEF_NODE
class Node{
    public:

        Node(Node* par, Action act, ALEState ale_state, int d, double rew, double discs);

        Node(Node* par, Action act, int d, double rew, double discs);

        std::vector<Node *> get_successors(ALEInterface *env);

        std::vector<Node *> get_stateless_successors(ALEInterface *env);
        ALEState get_state();
        
        double get_reward_so_far(){ return reward_so_far; }
        void set_reward_so_far(float substract){ 
            reward_so_far /= discount_reward;
            reward_so_far -= substract; 
            discount /= discount_reward;
        }
         void set_rw(float rw){ 
            reward_so_far = rw;
        }
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
        int count_nodes();
        int reused_nodes;
        int tried;
        ActionVect childActs;
        bool solved;
        bool in_tree;
    private:

        Node *parent;
        ALEState state;
        int depth;
        double reward_so_far;
        Action action;
        double discount;
        bool count_in_novelty;
        bool is_terminal;
        bool is_duplicate;
};
#endif
