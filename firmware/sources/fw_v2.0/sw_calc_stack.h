
typedef struct 
{
	enum { X, Y, Z, T, M };
	union
	{
		struct { float X, Y, Z, T, M; } reg;
		float arr[5];
	};
} Stack;

#define STACK_SIZE (sizeof(Stack) / sizeof(float))

static Stack stack;  // Float stack (XYZT) and memory
static Stack shadow; // Shadow memory (buffer) for stack

static void StackPush()
{
	// M -> not changed
	// Z -> T
	// Y -> Z
	// X -> Y
	memmove(&stack.reg.Y, &stack.reg.X, (STACK_SIZE - 2) * sizeof(float));
}

static void StackPull()
{
	// Y -> X
	// Z -> Y
	// T -> Z
	// M -> not changed
	memcpy(&stack.reg.X, &stack.reg.Y, (STACK_SIZE - 2) * sizeof(float));
}

static void Store()
{
	stack.reg.M = stack.reg.X;
}

static void Recall()
{
	StackPush();
	stack.reg.X = stack.reg.M;
}