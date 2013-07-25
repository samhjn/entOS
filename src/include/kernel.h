#define kpc (&keContextCpu0)
#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define KERNEL_TSS 0x18
#define KERNEL_LDT 0x20
#define USER_CS 0x08
#define USER_DS 0x10

#define IRQ_BASE_VECTOR 0x20
#define IRQ_COUNT 16

#define TASK_LEVEL 0
#define CLI_LEVEL  1

#define REQUIRE_CLI_LEVEL (assert(kpc->exLevel == CLI_LEVEL))
#define REQUIRE_TASK_LEVEL	(assert(kpc->exLevel == TASK_LEVEL))

#define TASK_NICE_MAX	15
#define TASK_NIEC_MIN	0
#define TASK_KERNEL_STACK_SIZE	40960
#define TASK_MAX_WAIT 	3

#define TASK_STATE_INIT		0
#define TASK_STATE_READY	1
#define TASK_STATE_RUNNING	2
#define TASK_STATE_SLEEPING	3
#define TASK_STATE_WAITING	4
#define TASK_STATE_KILLED	5
#define TASK_STATE_INVALID	-1

#define KWAITABLE_TYPE_SIGNAL	0
#define KWAITABLE_TYPE_EVENT	1
#define KWAITABLE_TYPE_MUTEX	2
#define KWAITABLE_TYPE_SEMAPHORE	3

#define TASK_MAGIC 0xaabbfbfcUL


#define TICK_MS		54

#define TASK_LIST_EMPTY(p)  ((p)->next == (p))


typedef void (*FnKernelThreadStartup)();

typedef struct _TASK_LIST_ENTRY {
	struct _KTASK *ptr;
	struct _TASK_LIST_ENTRY *next, *prev;
} TASK_LIST_ENTRY, *PTASK_LIST_ENTRY;

typedef struct _KTASK {
	u32 storedEsp;
	int state;
	int nice;
	int preemptableInKernel;
	u32 kernelStackBase;
	u32 kernelStackEnd;
	u32 magic1;
	u8 kernelStack[TASK_KERNEL_STACK_SIZE];
	u32 magic2;
	int waitCount;
	int waitAny;
	struct _KWAITABLE *waitObj[TASK_MAX_WAIT];
	TASK_LIST_ENTRY waitList[TASK_MAX_WAIT];
	TASK_LIST_ENTRY sleepList;
	TASK_LIST_ENTRY readyList;
	int sleepCount;
	FnKernelThreadStartup startupFunc;
} KTASK, *PKTASK;

typedef struct _TASK_INIT_CONTEXT{ 
	u32 eflags;
	REGS_PUSHAD regs;
	u32 retAddr;
} TASK_INIT_CONTEXT, *PTASK_INIT_CONTEXT;

typedef struct _KPROCESSOR_CONTEXT {
	int32_t cliCount;
	u32 exLevel;
	PKTASK currentTask;
} KPROCESSOR_CONTEXT, *PKPROCCESSOR_CONTEXT;

typedef struct _KWAITABLE {
	struct _KTASK *ownerTask;
	int type;
	int signalValue;
	TASK_LIST_ENTRY waitListHead;
} KWAITABLE, *PKWAITABLE, KMUTEX, *PKMUTEX, KSEMAPHORE, *PKSEMAPHORE, KSIGNAL, *PKSIGNAL, KEVENT, *PKEVENT; 

void conInitStage0();
void conPuts(char* str);
void conPutChar(char ch);
void conClearScreen();
int conSetCursor(int x, int y);

void dbgPrintRegs(REGS_PUSHAD r);
void dbgAssert(char* e, char* file, int line) ;
void keBugCheck(char* reason, u32 param1, u32 param2, u32 param3, u32 param4) ;

void keEnterCli();
void keLeaveCli();
inline u8 inportb(u16 port);
inline u16 inportw(u16 port);
inline void outportb(u16 port, u8 data);
inline void outportw(u16 port, u16 data) ;
void io_wait();

void kePICEnableIrq(int irq) ;
void kePICSetMask(u32 m) ;
u32 kePICGetMask() ;
void kePICDoEoi(int irq) ;
void kePICInit() ;
void keSetIDT() ;

extern KPROCESSOR_CONTEXT keContextCpu0;
extern u32 keSysTickCount;
extern u32 keSysTickCount;

STDC void keSwapTaskInternal(PKTASK currentTask, PKTASK swapToTask);
void keSleep(int ms) ;
PKTASK keCreateTask(FnKernelThreadStartup startupFunc) ;
void keRescheduleTask() ;
PKTASK keFindReadyTask() ;
void keEndSwapTask() ;
void keSwapTask(PKTASK currentTask, PKTASK swapToTask) ;
void keUnreadyTask(PKTASK task, int newState) ;
void keReadyTask(PKTASK task) ;
void keInitTaskMgr() ;
void keReadyTask(PKTASK task) ;
void keInitTaskList(PTASK_LIST_ENTRY head) ;
void keInsertTaskList(PTASK_LIST_ENTRY head, PTASK_LIST_ENTRY e) ;
void keRemoveTaskList(PTASK_LIST_ENTRY e);

int keTestUnwaitObject(PKWAITABLE obj) ;
void keWaitOneObject(PKWAITABLE p) ;
void keAcquireMutex(PKMUTEX p) ;
void keReleaseMutex(PKMUTEX p) ;
void keInitMutex(PKMUTEX p) ;
void keExitTask() ;
void keInitSysTick() ;
void keYield() ;

inline u8 inportb(u16 port) {
    u8 ret;
    __asm__ __volatile__ ("inb %1, %0":"=a"(ret):"dN"(port));
    return ret;
}

inline u16 inportw(u16 port) {
    u16 ret;
    __asm__ __volatile__ ("inw %1, %0":"=a"(ret):"dN"(port));
    return ret;
}

inline void outportb(u16 port, u8 data) {
    __asm__ __volatile__ ("outb %1, %0"::"dN"(port),"a"(data));
}

inline void outportw(u16 port, u16 data) {
    __asm__ __volatile__ ("outw %1, %0"::"dN"(port),"a"(data));
}

STDC void keSetIrqHandler(int irq, void* pHandler);