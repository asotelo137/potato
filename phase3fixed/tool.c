// tool.c, 159


#include "spede.h"
#include "type.h"
#include "extern.h"


void MyBZero( char *p, int size){
	while(size--){
		*p = '\0';
      		p++;
	}
}

void EnQ(int pid, q_t *p) {
// ?????????????????????????????????????????????????
// show error msg and return if queue's already full
// needs coding
// ????????????????????????????????????????????????	
	if (p->size == Q_SIZE){
		printf("\nQueue is \n");
		return;
	}else
	// printf("EnQ pid %d \n",pid);
	p->q[p->tail]= pid;
	p->tail ++;
	
	if (p->tail >= Q_SIZE) {
		p->tail = 0;
	}
	p->size ++;
}

int DeQ(q_t *p) { // return -1 if q is empty
// ?????????????????????????????????????????????????
// needs coding
// ?????????????????????????????????????????????????
	int pid;
	
	if (p->size == 0){
		printf("Queue is empty\n");
		return -1;
	}else
	
	pid=p->q[p->head];
	p->head++;
	
	if(p->head==Q_SIZE){
		p->head=0;
	}
	p->size--;
//	printf("DeQ pid %d \n",pid);
	return pid;
}

