#include"Node.hpp"
#include<ale_interface.hpp>
#include"utils.hpp"
#include"constants.hpp"
#include<cassert>
int value[210][160]; 
bool background[210][160];

int pack_basic_feature(int c, int r, int color){
    //std::cout << color << "\n";
    //assert(!(color & 1));
    //assert(color < 128);
    color >>= 1;
    assert(c >= 0 && c < 16);
    assert(r >= 0 && r < 14);
    return color + (( 14 * c + r ) << 7);
}

 void unpack_basic_feature(int pack, basic_t &ret){
    assert(pack < k_total_basic_features);

    int color = 127 & pack;
    pack >>= 7;
    int c,r;
    r = pack % 14;
    c = pack / 14;

    ret.first.first = c;
    ret.first.second = r;
    ret.second = color;
}

 bool is_cross_feature(int pack) {
    return (pack >= k_total_basic_features) && (pack < k_total_basic_features + num_cross_features_);
}


/*int pack_cross_feature(int dr, int dc, int p1, int p2){
    assert((-15 <= dc) && (dc <=15));
    assert((-13 <= dr) && (dr <= 13));
    assert((p1 >= 0) && (p1 < 128));
    assert((p2 >= 0) && (p2 < 128));
    assert(p1 <= p2);
    int pack = 0;
    if(p1 < p2) {
       pack = ((15 + dc) * 27 + (13 + dr)) * 64 * 127;
    }

    pack += k_total_basic_features;
    assert(is_cross_feature(pack));
    return pack;
}*/

 int pack_cross_feature(int dc, int dr, int p1, int p2) {
    assert((-15 <= dc) && (dc <= 15));
    assert((-13 <= dr) && (dr <= 13));
    assert((p1 >= 0) && (p1 < 128));
    assert((p2 >= 0) && (p2 < 128));
    assert(p1 <= p2);
    int pack = 0;
    if( p1 < p2 ) {
        pack = ((15 + dc) * 27 + (13 + dr)) * 64 * 127;
        pack += p1 * 127 - p1 * (1 + p1) / 2 + p2 - 1;
        assert((pack >= 0) && (pack < num_cross_features_type0_));
    } else if( (dc != 0) || (dr != 0) ) {
        assert(p1 == p2);
        if( (dc < 0) || ((dc == 0) && (dr < 0)) ) {
            dc = -dc;
            dr = -dr;
        }
        assert((dc > 0) || ((dc == 0) && (dr > 0)));

        if( dc > 0 )
            pack = ((dc - 1) * 27 + (13 + dr)) * 128;
        else
            pack = 15 * 27 * 128 + (dr - 1) * 128;

        pack += p1;
        assert((pack >= 0) && (pack < num_cross_features_type1_));
        pack += num_cross_features_type0_;
    } else {
        assert((p1 == p2) && (dc == 0) && (dr == 0));
        pack = p1;
        assert((pack >= 0) && (pack < num_cross_features_type2_));
        pack += num_cross_features_type0_ + num_cross_features_type1_;
    }
    pack += k_total_basic_features;
    assert(is_cross_feature(pack));
    return pack;
}


int pack_cross_feature(int b1, int b2){
    basic_t f1,f2;
    unpack_basic_feature(b1, f1);
    unpack_basic_feature(b2, f2);
    int dr = f1.first.second - f2.first.second; 
    int dc = f1.first.first - f2.first.first; 
    if(f1.second <= f2.second)
        return pack_cross_feature(dc,dr,f1.second, f2.second);
    else return pack_cross_feature(-dc, -dr, f2.second, f1.second);
}

bool is_temporal_feature(int pack){
    return pack >= k_total_basic_features + num_cross_features_ && pack < k_total_basic_features + num_cross_features_ + num_temporal_features_;;
}

int pack_temporal_feature(int dc, int dr, int k1, int k2){
    
   dc += 15;
   dr += 13;

   int pack = k2 + (1<<7) * k1;
   pack += dr * (1<<14);
   pack += dc * 27 * (1 << 14);

   pack += k_total_basic_features + num_cross_features_;
   assert(is_temporal_feature(pack));
   return pack;

}

int pack_temporal_feature(int b1, int b2){
    basic_t f1,f2;
    unpack_basic_feature(b1, f1);
    unpack_basic_feature(b2, f2);
    int dr = f1.first.second - f2.first.second; 
    int dc = f1.first.first - f2.first.first; 
    
    
    return pack_temporal_feature(dc,dr,f1.second, f2.second);
}


 int is_background(int c, int r, int p){

    p >>= 1;

    if(p != value[r][c]){
        if (value[r][c] == -1){
            value[r][c] = p;
            return false;
        }
        else
            background[r][c] = false;
    }

    return background[r][c];
}

bool is_basic_feature(int pack) {
        return (pack >= 0) && (pack < k_total_basic_features);
}

 void restore_state(Node *nod, ALEInterface *env){
     Node * par = nod->get_parent();
     if(par != NULL){
         env->restoreState(par->get_state());
         env->act(nod->get_action());
      } else {
         env->restoreState(nod->get_state());
      }
}

 void compute_meaninful_pixels(ALEInterface *ale){
    /*std::vector<byte_t> screen;
    bool initialized = false;
    int current_fs = ale->getInt("frame_skip");
    ActionVect legal_actions = ale -> getLegalActionSet();
    std::cout << "Computing meaningful pixels\n"; 
*/
    for(int i=0 ; i < 210; i++) for (int j =0 ; j < 160; j++)
        value[i][j] = -1, background[i][j] = true;
   /* for(int i =0 ; i < 18000; i++){
        if(ale->game_over()){
            ale->reset_game();
        }
        ale->act(legal_actions[rand() % legal_actions.size()]); 
        ale->getScreenGrayscale(screen);
        if(!initialized){
            initialized = true;
            for(int i =0 ; i< 210; i++)
                for(int j = 0 ; j < 160; j++){
                   value[i][j] = ((int)screen[i * 160 + j]) >> 1; 
                   background[i][j] = true;
                }
        
        }
        for(int i =0 ; i< 210; i++)
            for(int j = 0 ; j < 160; j++){
               if(((int)screen[i * 160 + j] >> 1) != value[i][j])
                    background[i][j] = false;
            }
    }
    int cnt = 0;
    for(int i =0 ; i <210 ; i++)
        for(int j =0 ; j < 160; j++)
            if(!background[i][j])
                cnt++;
     */       
  
    //ale->reset_game();
    //std::cout << cnt << " out of 33600 are going to be used \n";
}


 std::vector<byte_t> get_feat(ALEInterface *env, bool take_screen){
    std::vector<byte_t> v;
    //restore_state(nod, env);
    if(!take_screen){
        const ALERAM &ram = env->getRAM();
        for(int i = 0 ; i < RAM_SIZE; i++){
            v.push_back(ram.get(i));
        }
    } else{
        env->getScreenGrayscale(v);
    }

    return v;
}


void get_bit_index(int pack, std::pair<int,int> &p){
    int i_size = sizeof(int);
    
    p.first  =  pack / i_size;
    p.second =  pack % i_size;
     
}
