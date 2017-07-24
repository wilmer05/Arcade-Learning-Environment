#include"Node.hpp"
#include<ale_interface.hpp>
#include<vector>
#include<utility>
#include<map>
#ifndef UTILS
#define UTILS

void restore_state(Node *nod, ALEInterface *env);

std::vector<std::pair<int,int> > get_meaninful_pixels(ALEInterface *env);


std::vector<byte_t> get_feat(ALEInterface *env, bool take_screen);

#endif
