/* б) Даны последовательности символов А = {а0…аn–1} и С = {с0…ск–1}. В общем случае
n ≠ k. Создать многопоточное приложение, определяющее все те символы, которые входят в
строку А и в строку С.
*/

#include <iostream>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sched.h>
#include <string>

#define NUMSTACK 5000


char common_smb[1024] ; 	// для записи символов
int counter = 0; 				// текущий индекс
unsigned lenA, lenC;		// Длины строк
char* C_shared;

int func(void *param) //стартовая функция потоков
{
	char p = *(char*)param;
	for(int i = 0; i < lenC; i++)
	{
		if (C_shared[i] == p)
		{
			common_smb[counter] = p;
			counter += 1;
			break;
		}
	}
	return 1 ;
}

int main(int argc, char const *argv[])
{
	std::cout << "Введите длину строки A: " << std::endl;
	std::cin >> lenA;
	std::cout << "Введите длину строки C: " << std::endl;
	std::cin >> lenC;

	char A[lenA];
	char C[lenC];

	std::cout << "Введите строку А (через пробел): " << std::endl;
	for (int i = 0; i < lenA; i++)
	{
		std::cin >> A[i];
	}

	std::cout << "Введите строку C (через пробел): " << std::endl;
	for (int i = 0; i < lenC; i++)
	{
		std::cin >> C[i];
	}
	C_shared = new char[lenC];
	for (int i = 0; i < lenC; i++)
	{
		C_shared[i] = C[i];
	}
	std::cout << "Разделяемая строка: " << C_shared << std::endl;

  	char stack[lenA][ NUMSTACK] ; // Выделяем стек под clone-ы
	
	char param[lenA]; // Инициализируем массив параметров.
	for (int i = 0; i < lenA; i++)
  	{
  		param[i] = A[i];
		char *tostack=stack[i] ;
  		clone(func,(void*)( tostack+ NUMSTACK -1),CLONE_VM, (void *)(param+i)) ;
  	}
  	param[lenA] = lenA;
  	char *tostack=stack[lenA];
  	clone(func,(void*)( tostack+NUMSTACK-1), CLONE_VM|CLONE_VFORK, (void*)(param+lenA));
  	std::cout << "<i> [Мастер-поток дождался дочерних потоков]\n"; 

  	std::cout << "Общие символы между строками: " << "\n";
  	for (int i = 0; i < counter; i++)
  	{
  		std::cout << common_smb[i] << " ";
  	}
	

	return 0;
}