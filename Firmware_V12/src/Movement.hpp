#include<Settings.hpp>
#ifndef Movement_Hpp
#define Movement_Hpp



void initialize_movement_struct(struct PositionStruct *pos, struct ControlStruct *control);
void initialize_movement_struct_no_control(struct PositionStruct *pos);

// Set by LuaRuntime::requestAbort(); polled inside move_function to enable mid-move abort.
extern volatile bool gMotionAbortFlag;
// Set true by move_function when a move is halted mid-way; checked in MovementLua.cpp.
extern volatile bool gMotionWasAborted;
#ifndef Has_rs485_ecoders
void home_all(struct PositionStruct *current_pos, struct Error *error);
void move_function(struct PositionStruct *current_pos, struct PositionStruct *input_data, struct Error *error);
#else
void home_all(struct PositionStruct *current_pos, struct Error *error,int NODE_ADDR_1,int NODE_ADDR_2);
void move_function(struct PositionStruct *current_pos, struct PositionStruct *input_data, struct Error *error,int NODE_ADDR_1,int NODE_ADDR_2);
#endif


#endif
