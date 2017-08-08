#ifndef CONSTANTS
#define CONSTANTS
const int frameskip = 5;
const int max_depth = 1500;
const int max_lookahead = 150000;
const double discount_reward = 0.995;
const int max_steps = 18000;


const int k_novelty_columns = 16;
const int k_novelty_rows = 15;

const int k_box_columns_size = 10;
const int k_box_rows_size = 15;


const int total_col = 160 / k_box_columns_size;
const int total_row = 210 / k_box_rows_size;
const int k_different_colors = 256;


const int k_non_existent_color = 0;
const int k_few_existent_color = 1;
const int k_several_existent_color = 2;
const int k_relevant_color = 3;

const int k_several_existent_color_percentage = 25;
const int k_relevant_color_percentage = 50;


const int k_total_basic_features = 14 * 16 * 128;
const int k_time_steps_comparison = 60;


 const size_t num_cross_features_type0_ = 6803136; // (dc,dr,k1,    k2) where k1 < k2
 const size_t num_cross_features_type1_ = 53504; // (dc,dr,k    ,k) where dc != 0 or dr != 0, equal to (31 * 27 - 1) * 128 / 2
 const size_t num_cross_features_type2_ = 128; // (dc,dr,k,k    ) where dc = dr = 0
 const size_t num_cross_features_ = num_cross_features_type0_ + num_cross_features_type1_ + num_cross_features_type2_; // 6856768

 const size_t num_temporal_features_ = 31 * 27 * 128 * 128;


 const int k_log_scores = 5;

#endif

