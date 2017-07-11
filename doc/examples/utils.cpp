#include"Node.hpp"
#include<ale_interface.hpp>
#include"utils.hpp"

void restore_state(Node *nod, ALEInterface *env){
     Node * par = nod->get_parent();
     if(par != NULL){
         env->restoreState(par->get_state());
         env->act(nod->get_action());
      } else{
         env->restoreState(nod->get_state());
      }
}

