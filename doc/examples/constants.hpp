#ifndef CONSTANTS
#define CONSTANTS
const int frameskip = 5;
const int max_depth = 1500;
const int max_lookahead = 150000;
const double discount_reward = 0.995;
const int max_steps = 18000;


const int k_novelty_columns = 17;
const int k_novelty_rows = 15;

const int k_box_columns_size = 10;
const int k_box_rows_size = 15;


const int k_different_colors = 260;


const int k_non_existent_color = 0;
const int k_few_existent_color = 1;
const int k_several_existent_color = 2;
const int k_relevant_color = 3;

const int k_several_existent_color_percentage = 25;
const int k_relevant_color_percentage = 50;
#endif
