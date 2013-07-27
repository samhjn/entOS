#include "os.h"

KPROCESSOR_CONTEXT keContextCpu0;
extern MULTIBOOT_INFO  *mbd;
extern u32 magic;




void io_wait() {
    asm volatile( "outb %%al, $0x80"
                  : : "a"(0) );
}

void keEnterCli() {
	kpc->cliCount ++;
	asm("cli");
	kpc->exLevel = CLI_LEVEL;
}

void keLeaveCli() {
	kpc->cliCount --;
	assert(kpc->cliCount >= 0);
	if (kpc->cliCount == 0) {
		asm("sti");
		kpc->exLevel = TASK_LEVEL;
	}
}

PKTASK keStartupTask;
PKTASK keIdleTask;
PKTASK keTestTask2;
PKTASK keTestTask3;
KTASK tmpTask;

KMUTEX mutex;
void keTestThread2() {
    int i;

	while(1) {
		keAcquireMutex(&mutex);
		for (i = 0; i < 10; i++) {
			kprintf("a");
			keSleep(1);
		}
		kprintf("\n");
		keReleaseMutex(&mutex);
	}
	keExitTask();
}

void keTestThread3() {
	char a[]="This is a test thread\n";
	kprintf("here is keTestThread3()\n");
	while(1)
	{
		kprintf(a);
		keSleep(1);
	}
	keExitTask();
}

void keSystemStartup() {
	int i;
	
    	kprintf("keSystemStartup\n");

	
	keInitSysTick();
    	keInitMutex(&mutex);
	keTestTask2 = keCreateTask(keTestThread2);
	keTestTask3 = keCreateTask(keTestThread3);
   	while(1) {
		keAcquireMutex(&mutex);
		for (i = 0; i < 10; i++) {
			kprintf("B");
			keSleep(1);
		}
		kprintf("\n");
		keReleaseMutex(&mutex);
    }
}

void keIdleLoop() {
	while(1) {
		keYield();
	}
}

STDC
void keInit() {

	kpc->cliCount = 0;
	keEnterCli();
	conInitStage0();
	kprintf("keInit\n"); 
	if (magic != 0x2BADB002) {
		kprintf("multiboot magic mismatch (%08x), halted.\n", magic);
		while(1);
	}

	keSetIDT();
	mmInitStage0();
	//asm(".intel_syntax noprefix");
	kePICInit();
	keInitTaskMgr();
	memset(&tmpTask, 0, sizeof(tmpTask));
	tmpTask.storedEsp = 0;
	tmpTask.state = TASK_STATE_INVALID;
	keStartupTask = keCreateTask(keSystemStartup);
	keIdleTask = keCreateTask(keIdleLoop);
	kpc->currentTask = &tmpTask;
	
	kprintf("tmpTask: %08x, startupTask: %08x, idleTask: %08x\n", &tmpTask, keStartupTask, keIdleTask); 
	keRescheduleTask();
	assert(0);
	while(1);
}


