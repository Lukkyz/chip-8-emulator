#include "./stack.h"
#include <stdio.h>

/* Function : Stack_Push
 *
 * struct Stack, data
 *
 * Put data at the top of the stack
 *
 * return :
 * -1 if stack is full
 *  0 if data has been added
 */
int Stack_Push(Stack *stack, unsigned int *data) {
  if (stack->top == SIZE - 1) {
    return -1;
  }

  stack->top++;
  stack->data[stack->top] = *data;

  return 0;
}

/* Function Stack_Pop
 *
 * struct Stack
 *
 * return :
 * value
 * -1 if stack is empty
 */
unsigned short Stack_Pop(Stack *stack) {
  if (stack->top > -1) {
    return stack->data[stack->top--];
  }
  return -1;
}

void Stack_Print(Stack *stack) {
  int idx = 0;
  while (idx <= stack->top) {
    printf("%d\n", stack->data[idx]);
    idx += 1;
  }
}
