/*
	ВАРИАНТ 17
	Сформировать массив В заменяя элементы массива А их наибольшими делителями.
Найти среднее арифметическое элементов массива В, сумма индексов которых является
нечетным числом. 
	Входные данные: целое положительное число n, массив чисел от А
размерности nxn, целое число к ≥ 2 и ≤ n/2. Использовать к процессов для решения задачи.
*/

#include <iostream>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

const int MAXIMUM_ARRAY_SIZE = 1024;

struct shared_memory{
  int *B = new int[MAXIMUM_ARRAY_SIZE];
} *shared_mem;


 // Валидация K согласно условию
bool validate_K(unsigned K, unsigned N)
{
	if (K >= 2 && K <= N/2) return true;
	else return false;
}

int main(int argc, char const *argv[])
{
	unsigned N;
	std::cout << "Введите N: " << std::endl;
	std::cin >> N;

	unsigned K;
	std::cout << "Введите K (кол-во процессов): " << std::endl;
	std::cin >> K;

	if(!validate_K(K, N))
	{
		std::cout << "Введённое значение К не соответствует условию.";
		return -1;
	}

	std::vector<int> A;
	std::cout << "Теперь необходимо ввести " << N*N << " элементов массива А. (можно просто через пробел)" << std::endl;
	for (int i = 0; i <= N * N; i++)
	{
		int temp_num;
		std::cin >> temp_num;
		A.push_back(temp_num);
	}
	// Вывод массива на экран.
	std::cout << "Массив А: ";
	for (int i = 0; i < A.size(); i++)
	{
		std::cout << A[i] << " ";
	}
	std::cout << std::endl;

	// === Работа с разделяемой памятью
	int shmid = shmget(IPC_PRIVATE, 4 * MAXIMUM_ARRAY_SIZE, IPC_CREAT|0666) ;

  	shared_mem = (shared_memory*)shmat(shmid,NULL,0);

  	unsigned chunk_size = A.size() / K;
  	std::cout << "<i> Размер куска для каждого потока: " << chunk_size << "\n";
  	// === Работа с разделяемой памятью (КОНЕЦ)

	int status;
	for (int i = 0; i < K; i++)
  	{
  		int pid = fork();
  		if (pid == 0)
  		{
  			std::cout << "<i> [Дочерний поток " << i << " начал выполнение]\n"; 
  			for (int j = i * chunk_size; j < i * chunk_size + chunk_size; j++) 
		    {
		        shared_mem->B[j] = A[j];
		    }
  			exit(0);
  		}
    	
  	}
  	wait(&status);
  	std::cout << "<i> [Мастер-поток дождался дочерних потоков]\n"; 

	std::cout << "Массив B: ";
	for (int i = 0; i < A.size(); i++)
	{
		std::cout << shared_mem->B[i] << " ";
	}

  	


	return 0;
}