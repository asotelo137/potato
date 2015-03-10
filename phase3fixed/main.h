// main.h, 159

#ifndef _MAIN_H_
#define _MAIN_H_

int main();
void InitData();
void SelectCRP();
void Kernel();
void InitIDT();
void SetEntry(int, func_ptr_t);

#endif
