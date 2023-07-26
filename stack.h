#define SIZE 16

typedef struct Stack {
  unsigned short data[SIZE];
  int top;
} Stack;

int Stack_Push(Stack *stack, unsigned short *data);
unsigned short Stack_Pop(Stack *stack);
void Stack_Print(Stack *stack);
