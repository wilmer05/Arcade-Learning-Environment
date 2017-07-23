#include"Node.hpp"
#include<ale_interface.hpp>
#include"utils.hpp"

int value[210][160]; 
bool background[210][160];
void restore_state(Node *nod, ALEInterface *env){
     Node * par = nod->get_parent();
     if(par != NULL){
         env->restoreState(par->get_state());
         env->act(nod->get_action());
      } else{
         env->restoreState(nod->get_state());
      }
}

std::vector<std::pair<int,int> > get_meaninful_pixels(ALEInterface *ale){
    std::vector<std::pair<int,int> > meaninful_pixels;
    std::vector<byte_t> screen;
    bool initialized = false;
    int current_fs = ale->getInt("frame_skip");
    //std::cout << "Were using frame_skip of " << current_fs << "\n";
    //ale->setInt("frame_skip", 5);
    //ale->reset_game();
    ActionVect legal_actions = ale -> getLegalActionSet();
    std::cout << "Computing meaningful pixels\n"; 
    for(int i =0 ; i < 18000; i++){
        //std::cout << ale->getFrameNumber() << "\n";
        if(ale->game_over()){
            ale->reset_game();
//            std::cout <<"Reseted at: " << i << "\n";
        }
        ale->act(legal_actions[rand() % legal_actions.size()]); 
        ale->getScreenGrayscale(screen);
        if(!initialized){
            initialized = true;
            for(int i =0 ; i< 210; i++)
                for(int j = 0 ; j < 160; j++){
                   value[i][j] = (int)screen[i * 160 + j]; 
                   background[i][j] = true;
                }
        
        }
        for(int i =0 ; i< 210; i++)
            for(int j = 0 ; j < 160; j++){
               if((int)screen[i * 160 + j] != value[i][j])
                    background[i][j] = false;
            }
    }

    for(int i =0 ; i <210 ; i++)
        for(int j =0 ; j < 160; j++)
            if(!background[i][j])
                meaninful_pixels.push_back(std::make_pair(i, j));
     
    //ale->setInt("frame_skip", current_fs);
    ale->reset_game();
    std::cout << meaninful_pixels.size() << " out of 33600 are going to be used \n";
    return meaninful_pixels;
}
