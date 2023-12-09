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
#include <sys/sem.h>

#define NUMSTACK 5000

int chunk_size;
int counter = 0;
int *A; int A_size = 0;
int *B;
int sum = 0;
int N;

 // Валидация K согласно условию
bool validate_K(unsigned K, unsigned N)
{
	if (K >= 2 && K <= N/2) return true;
	else return false;
}

int greatest_divisor(unsigned n)
{
	  if (n%2 == 0) return n/2;
    else 
    {
    	int i;
        for(i = 3; i*i <= n; ++i)
            if (n%i == 0) break;
        if (i*i <= n) return n/i;
        else return 1;
    }
}

int fill_with_divisors(void* param){
	int i = *(int*)param;

	std::cout<< "<d> recieving i = " << i << "\n";
	std::cout << "<i> [Дочерний поток " << i << " начал выполнение]\n"; 
  	for (int j = i * chunk_size; j < i * chunk_size + chunk_size; j++) 
	{
		B[j] = greatest_divisor(A[j]);
		std::cout << "B[" << j << "] = " << B[j] << "\n";
	}
  	exit(0);
  		
}

int calc_sum(void *param)
{
	int i = *(int*)param;
	std::cout << "<i> [Дочерний поток " << i << " начал выполнение]\n"; 

	for (int j = i * chunk_size; j < i * chunk_size + chunk_size; j++) 
    {
    		if ( (j % N + j / N)%2 == 1 ) // считаем нечетная ли сумма индексов
    		{
	        	sum += B[j];
	        	counter += 1;
    		}
    }
	exit(0);
}


int main(int argc, char const *argv[])
{
	std::cout << "Введите N: ";
	std::cin >> N;

	int K;
	std::cout << "Введите K: ";
	std::cin >> K;
	if (!validate_K(K, N)) {
		std::cout << "K не соответствует условию.\n";
		return -1;
	};

	A = new int[N*N]; 
	std::cout << "Теперь необходимо ввести " << N*N << " элементов массива А. (можно просто через пробел)" << std::endl;
	for (int i = 0; i < N * N; i++)
	{
		int temp_num;
		std::cin >> temp_num;
		A[i] = temp_num;
		A_size++;
	}
	chunk_size = A_size / K;
	// Вывод массива на экран.
	std::cout << "Массив А: ";
	for (int i = 0; i < A_size; i++)
	{
		std::cout << A[i] << " ";
	}
	std::cout << std::endl;

	B = new int[N*N];

	char stack[K][NUMSTACK] ; // Выделяем стек под clone-ы
	
	char param[K]; // Инициализируем массив параметров.

	for (int i = 0; i < K; i++)
  	{
  		param[i] = i;
		char *tostack2 = stack[i] ;
		std::cout << "sent " << *(int*)(param + i) << "\n";
  		clone(fill_with_divisors, (void*)( tostack2+ NUMSTACK -1), CLONE_VM, (void *)(param+i)) ;
  		sleep(1);
  	}
  	std::cout << "<i> [Мастер-поток дождался дочерних потоков]\n"; 


  	char stack2[K][ NUMSTACK] ; // Выделяем стек под clone-ы
	
	char param2[K]; // Инициализируем массив параметров.

	for (int i = 0; i < K; i++)
  	{
  		param2[i] = i;
		char *tostack=stack2[i] ;
  		clone(calc_sum, (void*)( tostack+ NUMSTACK -1), CLONE_VM, (void *)(param2+i)) ;
  		sleep(1);
  	}
  	std::cout << "<i> [Мастер-поток дождался дочерних потоков]\n"; 
  	sleep(1);

  	std::cout << "Массив B (Наибольшие делители):" << std::endl;
	for (int i = 0; i < A_size; i++)
	{
			std::cout << B[i] << " ";
			if (i % N == N - 1) std::cout << std::endl;
	}


	std::cout << "Сумма элементов массива B, лежащих на нечётной сумме индексов: " << sum << std::endl;
	std::cout << "Количество элементов массива B, лежащих на нечётной сумме индексов: " << counter << std::endl;
	std::cout << "Среднее арифметическое: " << (static_cast<double>(sum) / counter) << std::endl;


	return 0;
}