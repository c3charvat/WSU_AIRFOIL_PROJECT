#include<Settings.hpp>
#ifndef Movement_Hpp
#define Movement_Hpp



void initialize_Movement_Struct(struct PositionStruct *pos, struct ControlStruct *control);
void initialize_Movement_Struct_NC(struct PositionStruct *pos);
#ifndef Has_rs485_ecoders
void HomeAll(struct PositionStruct *current_pos, struct Error *error);
void MOVE_FUNCTION(struct PositionStruct *current_pos, struct PositionStruct *input_data, struct Error *error);
#endif
#ifdef Has_rs485_ecoders
void HomeAll(struct PositionStruct *current_pos, struct Error *error,int NODE_ADDR_1,int NODE_ADDR_2);
void MOVE_FUNCTION(struct PositionStruct *current_pos, struct PositionStruct *input_data, struct Error *error,int NODE_ADDR_1,int NODE_ADDR_2);
#endif


#endif
