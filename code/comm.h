
typedef struct _COPY_MEMORY
{
	pid_t pid;
	uintptr_t addr;
	void *buffer;
	size_t size;
} COPY_MEMORY, *PCOPY_MEMORY;

typedef struct _MODULE_BASE
{
	pid_t pid;
	char *name;
	uintptr_t base;
} MODULE_BASE, *PMODULE_BASE;

enum OPERATIONS
{
	OP_INIT_KEY = 0xE00,
	OP_READ_MEM = 0xE01,
	OP_WRITE_MEM = 0xE02,
	OP_MODULE_BASE = 0xE03,
};