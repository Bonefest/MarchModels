#define MAX_STACK_SIZE 8

struct DistancesStack
{
  uint32 size;
  float32 distances[MAX_STACK_SIZE];
  uint32 geometry[MAX_STACK_SIZE]; // stores index of geometry corresponding to the distances
};
