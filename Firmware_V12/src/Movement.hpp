
#ifndef Movement_Hpp
#define Movement_Hpp


void initialize_Movement_Struct(struct PositionStruct *pos, struct ControlStruct *control);
void initialize_Movement_Struct_NC(struct PositionStruct *pos);
void HomeAll(struct PositionStruct *current_pos, struct Error *error);
void MOVE_FUNCTION(struct PositionStruct *current_pos, struct PositionStruct *input_data, struct Error *error);


#endif
