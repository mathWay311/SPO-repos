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

struct mymem //структура, под которую будет выделена разделяемая память
{
	char common_smb[1024] ; 	// для записи символов
	int counter; 				// текущий индекс
} *my_mem ;

int main(int argc, char const *argv[])
{
	unsigned lenA, lenC;
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

	// Разделяемая память (массив символов)
	int shmid = shmget(IPC_PRIVATE, 1024 * sizeof(char) + sizeof(int), IPC_CREAT|0666) ;
  	my_mem = (mymem *)shmat(shmid, NULL, 0) ;



  	int shmcontrol_semid; // Семафор контроля критической секции.
	shmcontrol_semid = semget (IPC_PRIVATE, 1, IPC_CREAT|0666) ;
  
  	sembuf Plus1 = 			{shmcontrol_semid, 1, 0} ; 						//операция прибавляет единицу к семафору контроля критической секции
	sembuf Minus1 = 		{shmcontrol_semid, -1, 0} ; 					//операция вычитает единицу от семафора контроля критической секции
  
  	semop( shmcontrol_semid, &Plus1, 1); // Предустановка ресурса

	int status2;
	my_mem->counter = 0;
	for (int i = 0; i < lenA; i++)
  	{
  		int pid = fork();
  		if (pid == 0)
  		{
  			std::cout << "<i> [Дочерний поток " << i << " начал выполнение]\n"; 
  			for (int j = 0; j < lenC; j++)
		    	if (A[i] == C[j])
		    	{
		    		semop( shmcontrol_semid, &Minus1, 1); // Блокировка ресурса
			        my_mem->common_smb[my_mem->counter] = A[i];
			        my_mem->counter += 1;
		        	semop( shmcontrol_semid, &Plus1, 1); // Высвобождение ресурса
		    	} 
		    		
  			exit(0);
  		}
    	
  	}
  	wait(&status2);
  	sleep(1);
  	std::cout << "<i> [Мастер-поток дождался дочерних потоков]\n"; 

  	std::cout << "Общие символы между строками: ";
  	for (int i = 0; i < my_mem->counter; i++)
  	{
  		std::cout << my_mem->common_smb[i] << " ";
  	}

	return 0;
}