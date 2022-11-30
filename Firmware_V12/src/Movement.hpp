#include<Settings.hpp>
#ifndef Movement_Hpp
#define Movement_Hpp



void initialize_movement_struct(struct PositionStruct *pos, struct ControlStruct *control);
void initialize_movement_struct_no_control(struct PositionStruct *pos);
#ifndef Has_rs485_ecoders
void home_all(struct PositionStruct *current_pos, struct Error *error);
void move_function(struct PositionStruct *current_pos, struct PositionStruct *input_data, struct Error *error);
#else
void home_all(struct PositionStruct *current_pos, struct Error *error,int NODE_ADDR_1,int NODE_ADDR_2);
void move_function(struct PositionStruct *current_pos, struct PositionStruct *input_data, struct Error *error,int NODE_ADDR_1,int NODE_ADDR_2);
#endif


#endif
