// tool.c, 159


#include "spede.h"
#include "type.h"
#include "extern.h"


void MyBZero(char *p, int size) {
	while(size--){
		*p++=(char)0;
	}
}


void EnQ(int pid, q_t *p) {
// ?????????????????????????????????????????????????
// show error msg and return if queue's already full
// needs coding
// ????????????????????????????????????????????????	
	if(p->size == Q_SIZE){
		//breakpoint();
		cons_printf(" Queue full!\n");
		
		return;
	}
	p->q[p->tail]=pid;
	p->size++;
	p->tail++;
	if(p->tail == Q_SIZE){
		p->tail = 0;
	}
}

int DeQ(q_t *p) { // return -1 if q is empty
// ?????????????????????????????????????????????????
// needs coding
// ?????????????????????????????????????????????????
	int pid;
	if(p->size ==0){
		cons_printf("Queue Empty!\n");
		return -1;
	}
	pid = p->q[p->head];
	p->size--;
	p->head++;
	if(p->head == Q_SIZE){
		p->head = 0;
	}
	return pid;
}
//phase 5 *******************************************************************************888

void MsgEnQ(msg_t *p, msg_q_t *q){
	if(q->size == Q_SIZE){
		//breakpoint();
		cons_printf("Mesage queue is full!\n");
		return;
	}
	q->msg[q->tail]=*p;
	q->size++;
	q->tail++;
	if(q->tail == Q_SIZE){
		q->tail = 0;
	}
}

msg_t *MsgDeQ(msg_q_t *p){
	
	msg_t *msg;
	if(p->size ==0){
		cons_printf("Message queue is Empty!\n");
		return '\0';
	}
	msg = &p->msg[p->head];
	p->head++;
	if(p->head == Q_SIZE){
		p->head = 0;
	}
	p->size--;
	return msg;
	
}

void MyStrCpy( char *dest, char *src){
	
	while(*src){
		*dest = *src;
		src++;
		dest++;
	}
	*dest='\0';
	
}

int MyStrcmp(char *s1, char *s2){
	while( *s1 && *s2){
		if(*s1 == *s2){
			s1++;
			s2++;
		}else 
			return 0;
	}
	if(*s1 != *s2){
		return 0;
	}
		return 1;
}

int MyStrlen(char *s){
	int length=0;
	char *string;
	for(string = s; * string != '\0'; string++){
		length++;
	}
	return length;
}	

void MyMemcpy(char *dest, char *src, int size){
	int i;
	for(i = 0; i <= size ; i++){
		*dest = *src;
		src++;
		dest++;
	}
	
}

int MyStrcmpSize(char *p, char *q, int size){
	int i;
	for(i = 0; i < size ; i++){
		if(*p == *q){
			p++;
			q++;
		}else 
			return 0;
	}
	return 1;
}