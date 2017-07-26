#include"Node.hpp"
#include<ale_interface.hpp>
#include<vector>
#include<utility>
#include<map>
#ifndef UTILS
#define UTILS

typedef std::pair< std::pair<int,int> , int> basic_t;

 int is_background(int, int, int);

 int pack_basic_feature(int c, int r, int col);

 void unpack_basic_feature(int p, basic_t &);

 bool is_cross_feature(int);

 bool is_temporal_feature(int);

 int pack_cross_feature(int, int, int, int);

 int pack_temporal_feature(int, int, int, int);

 int pack_temporal_feature(int, int);

 int pack_cross_feature(int, int);

 void restore_state(Node *nod, ALEInterface *env);

 void compute_meaninful_pixels(ALEInterface *env);

 std::vector<byte_t> get_feat(ALEInterface *env, bool take_screen);

void get_bit_index(int, std::pair<int,int> &);

 bool is_basic_feature(int);
#endif
