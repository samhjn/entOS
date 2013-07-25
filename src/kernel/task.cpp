#include "os.h"

#define PIT_DATA0 0x40
#define PIT_CMD 0x43

u32 keSysTickCount;
TASK_LIST_ENTRY keAliveTaskListHead;
TASK_LIST_ENTRY keSleepTaskListHead;
TASK_LIST_ENTRY keReadyTaskListHead[TASK_NICE_MAX + 1];

STDC
void keSysTickHandler() {
	PTASK_LIST_ENTRY ptr;
	PKTASK task;
	
	keSysTickCount ++;
	ptr = keSleepTaskListHead.next;
	while (ptr != &keSleepTaskListHead) {
		task = ptr->ptr;
		ptr = ptr->next;
		assert(task->state == TASK_STATE_SLEEPING);
		if (task->sleepCount > 0) {
			task->sleepCount --;
			if (task->sleepCount == 0) {
				keRemoveTaskList(&task->sleepList);
				keReadyTask(task);
			}
		}
	}
}

void keInitSysTick() {
	keSysTickCount = 0;
	keSetIrqHandler(0, (void*)keSysTickHandler);
	outportb(PIT_CMD, 0x36);
	outportb(PIT_DATA0, 0);
	outportb(PIT_DATA0, 0);
	kePICEnableIrq(0);
}

void keInitTaskList(PTASK_LIST_ENTRY head) {
	head->ptr = 0;
	head->next = head;
	head->prev = head;
}

void keInsertTaskList(PTASK_LIST_ENTRY head, PTASK_LIST_ENTRY e) {
	PTASK_LIST_ENTRY tail;
	
	REQUIRE_CLI_LEVEL;
	assert(e->next == e);
	tail = head->prev;
	
	tail->next = e;
	e->next = head;
	e->prev = tail;
	head->prev = e;
	//kprintf("insert: %08x %08x\n", head, e);
}

void keRemoveTaskList(PTASK_LIST_ENTRY e) {
	
	REQUIRE_CLI_LEVEL;
	assert(e->next != e);
	assert(e->prev != e);
	e->prev->next = e->next;
	e->next->prev = e->prev;
	e->next = e;
	e->prev = e;
}

void keInitTaskMgr() {
	int count;
	
	keInitTaskList(&keAliveTaskListHead);
	keInitTaskList(&keSleepTaskListHead);
	for (count = 0; count <= TASK_NICE_MAX; count++) {
		keInitTaskList(&keReadyTaskListHead[count]);
	}
}

void keReadyTask(PKTASK task) {
	REQUIRE_CLI_LEVEL;
	assert(task->state != TASK_STATE_READY);
	//kprintf("insertready: %d %08x \n", task->nice, &keReadyTaskListHead[task->nice]);
	keInsertTaskList(&keReadyTaskListHead[task->nice], &(task->readyList));
	task->state = TASK_STATE_READY;
}

void keUnreadyTask(PKTASK task, int newState) {
	REQUIRE_CLI_LEVEL;
	assert(task->state == TASK_STATE_READY);
	keRemoveTaskList(&task->readyList);
	task->state = newState;
}

void keSwapTask(PKTASK currentTask, PKTASK swapToTask) {
	
	REQUIRE_CLI_LEVEL;
	assert(kpc->cliCount == 1);
	
	assert(swapToTask->magic1 == TASK_MAGIC);
	assert(swapToTask->magic2 == TASK_MAGIC);
	kpc->currentTask = swapToTask;
	
	//kprintf("swaptask from: %x, to: %x\n", currentTask,swapToTask);
	keSwapTaskInternal(currentTask, swapToTask);
}

void keEndSwapTask() {
	REQUIRE_CLI_LEVEL;
	assert(kpc->cliCount == 1);
	
	keLeaveCli();
}

PKTASK keFindReadyTask() {
	int nice;
	PTASK_LIST_ENTRY listHead;
	
	REQUIRE_CLI_LEVEL;
	for (nice = 0; nice <= TASK_NICE_MAX; nice++) {
		listHead = (&keReadyTaskListHead[nice]);
		if (listHead->next != listHead) {
			return listHead->next->ptr;
		}
	}
	return 0;
}


void keRescheduleTask() {
	PKTASK readyTask;
	PKTASK currTask;
	
	REQUIRE_CLI_LEVEL;
	assert(kpc->cliCount == 1);
	currTask = kpc->currentTask;
	readyTask = keFindReadyTask();
	assert(currTask);
	assert(readyTask);
	assert(currTask->state != TASK_STATE_RUNNING);
	keUnreadyTask(readyTask, TASK_STATE_RUNNING);
	keSwapTask(currTask, readyTask);
	keEndSwapTask();
}

void keSleep(int ms) {
	PKTASK task;
	
	REQUIRE_TASK_LEVEL;
	keEnterCli();
	task = kpc->currentTask;
	task->state = TASK_STATE_SLEEPING;
	task->sleepCount = ms / TICK_MS + 1;
	if (ms < 0) task->sleepCount = -1;
	task->state = TASK_STATE_SLEEPING;
	keInsertTaskList(&keSleepTaskListHead, &task->sleepList);
	keRescheduleTask();
}

void keYield() {
	PKTASK task;
	
	REQUIRE_TASK_LEVEL;
	keEnterCli();
	task = kpc->currentTask;
	keReadyTask(task);
	keRescheduleTask();
}

void keExitTask() {
	PKTASK task;
	
	REQUIRE_TASK_LEVEL;
	keEnterCli();
	task = kpc->currentTask;
	task->state = TASK_STATE_KILLED;
	keRescheduleTask();
}

void keTaskStartup() {
	PKTASK task;
	
	task = kpc->currentTask;
	keEndSwapTask();
	task->startupFunc();
	assert(0);
}

PKTASK keCreateTask(FnKernelThreadStartup startupFunc) {
	PKTASK task;
	TASK_INIT_CONTEXT init;
	int count;
	
	task = (PKTASK) malloc(sizeof(KTASK));   
	if (!task) return 0;
	memset(&init, 0, sizeof(TASK_INIT_CONTEXT));
	memset(task, 0, sizeof(KTASK));
	
	init.eflags = 0x00000046;
	init.retAddr = (u32)keTaskStartup;
	task->startupFunc = startupFunc;
	
	keInitTaskList(&(task->sleepList));
	task->sleepList.ptr = task;
	keInitTaskList(&(task->readyList));
	task->readyList.ptr = task;
	for (count = 0; count < TASK_MAX_WAIT; count++) {
		keInitTaskList(&(task->waitList[count]));
		task->waitList[count].ptr = task;
		//kprintf("init %08x %08x\n", &(task->waitList[count]), (task->waitList[count]).next);
	}
	task->magic1 = TASK_MAGIC;
	task->magic2 = TASK_MAGIC;
	task->kernelStackBase = (u32)(task->kernelStack);
	task->kernelStackEnd = task->kernelStackBase + TASK_KERNEL_STACK_SIZE;
	task->preemptableInKernel = 0;
	task->nice = 8;
	task->storedEsp = task->kernelStackEnd - 16 - sizeof(TASK_INIT_CONTEXT);
	assert(task->kernelStackBase % 4 == 0);
	assert(task->storedEsp % 4 == 0);
	memcpy((PTASK_INIT_CONTEXT)(task->storedEsp), &init, sizeof(TASK_INIT_CONTEXT));
	
	keEnterCli();
	keReadyTask(task);
	keLeaveCli();
	return task;
}

