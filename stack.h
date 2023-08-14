#define SIZE 16

typedef struct Stack {
  unsigned int data[SIZE];
  int top;
} Stack;

int Stack_Push(Stack *stack, unsigned int *data);
unsigned short Stack_Pop(Stack *stack);
void Stack_Print(Stack *stack);
