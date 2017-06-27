#include<queue>
#include <ale_interface.hpp>
#include"Node.hpp"
#ifdef __USE_SDL
    #include<SDL.h>
#endif
#include<utility>
#include<set>

#define RAM_FEATURES 1
#ifndef DEF_IW
#define DEF_IW
class IW{
    public:
        IW() { 
            features_type = RAM_FEATURES; 
            novelty_table = std::set<std::pair<int, int> >();
            q = std::queue<Node *>();
        } 

        IW(int ft);
       
        Action run(ALEState state, ALEInterface *env);


    private:
        std::vector<std::pair<int,int> > get_features(ALEInterface *ale);
        int novelty(std::vector<std::pair<int,int> > fs);
        void add_to_novelty_table( std::vector<std::pair<int,int> > fs );
        
        std::set<std::pair<int,int> > novelty_table;
        std::queue<Node *> q;
        int features_type;

};
#endif
