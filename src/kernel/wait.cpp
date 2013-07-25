#include "os.h"

void keInitWaitable(PKWAITABLE p, int type, int initValue) {
	p->ownerTask = 0;
	p->type = type;
	p->signalValue = initValue;
	keInitTaskList(&p->waitListHead);
}

void keInitMutex(PKMUTEX p) {
	keInitWaitable(p, KWAITABLE_TYPE_MUTEX, 1);
}

void keInitSemaphore(PKSEMAPHORE p) {
	keInitWaitable(p, KWAITABLE_TYPE_SEMAPHORE, 0);
}

void keInitSignal(PKSIGNAL p) {
	keInitWaitable(p, KWAITABLE_TYPE_SIGNAL, 0);
}

void keInitEvent(PKEVENT p) {
	keInitWaitable(p, KWAITABLE_TYPE_EVENT, 0);
}

void keAcquireMutex(PKMUTEX p) {
	assert(p->type == KWAITABLE_TYPE_MUTEX);
	
	if (p->ownerTask == kpc->currentTask) return;
	keWaitOneObject(p);
	assert(p->ownerTask == kpc->currentTask);
}

void keReleaseMutex(PKMUTEX p) {
	assert(p->type == KWAITABLE_TYPE_MUTEX);
	assert(p->ownerTask == kpc->currentTask);
	
	keEnterCli();
	p->signalValue = 1;
	p->ownerTask = 0;
	keTestUnwaitObject(p);
	keLeaveCli();
}


void keFinishObjectWait(PKWAITABLE p, PKTASK task) {

	REQUIRE_CLI_LEVEL;
	assert(p->signalValue > 0);
	switch(p->type) {
	case KWAITABLE_TYPE_MUTEX:
		p->ownerTask = task;
		p->signalValue = 0;
		break;
	case KWAITABLE_TYPE_EVENT:
		break;
	case KWAITABLE_TYPE_SEMAPHORE:
		p->signalValue --;
		break;
	case KWAITABLE_TYPE_SIGNAL:
		break;
	default:
		assert(0);
	}
}

int keTestWait(PKTASK task) {
	int count;
	PKWAITABLE obj;
	
	REQUIRE_CLI_LEVEL;
	assert(task->state == TASK_STATE_WAITING);
	if (task->waitAny) {
		for (count = 0; count < task->waitCount; count++) {
			obj = task->waitObj[count];
			assert(obj);
			assert(obj->signalValue >= 0);
			if (obj->signalValue > 0) {
				keFinishObjectWait(obj, task);
				return 1;
			} 
		}
		return 0;
	} else {
		for (count = 0; count < task->waitCount; count++) {
			obj = task->waitObj[count];
			assert(obj);
			assert(obj->signalValue >= 0);
			if (obj->signalValue == 0) {
				return 0;
			} 
		}
		for (count = 0; count < task->waitCount; count++) {
			keFinishObjectWait(obj, task);
		}
		return 1;
	}
}

void keUnwaitTask(PKTASK task) {
	int count;
	
	REQUIRE_CLI_LEVEL;
	for (count = 0; count < task->waitCount; count++) {
		keRemoveTaskList(&task->waitList[count]);
	}
}

int keTestUnwaitObject(PKWAITABLE obj) {
	PTASK_LIST_ENTRY ptr;
	PKTASK task;
	int count, taskAffected;
	
	REQUIRE_CLI_LEVEL;
	taskAffected = 0;
	ptr = obj->waitListHead.next;
	while((ptr != &obj->waitListHead) && (obj->signalValue)) {
		assert(ptr);
		task = ptr->ptr;
		ptr = ptr->next;
		if (keTestWait(task)) {
			keUnwaitTask(task);
			keReadyTask(task);
			taskAffected ++;
		}
	}
	return taskAffected;
}

void keWaitMultipleObjects(PKWAITABLE p[], int waitCount, int waitAny) {
	int count;
	PKTASK task;
	PKWAITABLE obj;
	
	REQUIRE_TASK_LEVEL;
	assert(waitCount <= TASK_MAX_WAIT);
	keEnterCli();
	task = kpc->currentTask;
	task->waitCount = waitCount;
	task->waitAny = waitAny;
	
	for (count = 0; count < waitCount; count++) {
		obj = p[count];
		assert(task->waitList[count].ptr == task);
		keInsertTaskList(&obj->waitListHead, &task->waitList[count]);
		task->waitObj[count] = obj;
	}
	task->state = TASK_STATE_WAITING;
	if (keTestWait(task)) {
		keUnwaitTask(task);
		task->state = TASK_STATE_RUNNING;
		keLeaveCli();
		return;
	}
	task->state = TASK_STATE_WAITING;
	keRescheduleTask();
	
}

void keWaitOneObject(PKWAITABLE p) {
	keWaitMultipleObjects(&p, 1, 1);
}
