#ifndef IDLE_H
#define IDLE_H

void SetupIdle();
void RunIdle();
void ResetTotal();
void RemoveLastBoard();
void AddBoardToList(float lengthInches);
float GetBoardLengthInches();
bool HasValidSensorReading();
float GetTotalInches();
int GetBoardCount();
float GetBoardLengthAtIndex(int i);

#endif
