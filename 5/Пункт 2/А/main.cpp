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

const int MAXIMUM_ARRAY_SIZE = 1024;

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

struct mymem //структура, под которую будет выделена разделяемая память
{
int sum ; //для записи суммы
int counter; // для записи количества чисел
} *mem_sum ;

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
	for (int i = 0; i < N * N; i++)
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

	// === Работа с разделяемой памятью массива
	int *B;
	int count = A.size();
	key_t key = 12345;
	int shmid = shmget(key, sizeof(int) * count, IPC_CREAT|0666) ;

  	B = (int*)shmat(shmid,NULL,0);

  	unsigned chunk_size = A.size() / K;
  	std::cout << "<i> Размер куска для каждого потока: " << chunk_size << "\n";
  	// === Работа с разделяемой памятью массива (КОНЕЦ)

  // === Работа с разделяемой памятью суммы
	int shmid2 = shmget(IPC_PRIVATE, 4, IPC_CREAT|0666) ;
  mem_sum = (mymem *)shmat(shmid2,NULL,0) ;
  // === Работа с разделяемой памятью суммы (КОНЕЦ)

	int status;
	for (int i = 0; i < K; i++)
  	{
  		int pid = fork();
  		if (pid == 0)
  		{
  			std::cout << "<i> [Дочерний поток " << i << " начал выполнение]\n"; 
  			for (int j = i * chunk_size; j < i * chunk_size + chunk_size; j++) 
		    {
		        B[j] = greatest_divisor(A[j]);
		    }
  			exit(0);
  		}
    	
  	}
  	wait(&status);
  	sleep(1);
  	std::cout << "<i> [Мастер-поток дождался дочерних потоков]\n"; 



  int shmcontrol_semid; // Семафор контроля критической секции.
	shmcontrol_semid = semget (IPC_PRIVATE, 1, IPC_CREAT|0666) ;
  
  sembuf Plus1 = 			{shmcontrol_semid, 1, 0} ; 						//операция прибавляет единицу к семафору контроля критической секции
	sembuf Minus1 = 		{shmcontrol_semid, -1, 0} ; 					//операция вычитает единицу от семафора контроля критической секции
  
  semop( shmcontrol_semid, &Plus1, 1); // Предустановка ресурса

	int status2;
	mem_sum->sum = 0;
	mem_sum->counter = 0;
	for (int i = 0; i < K; i++)
  	{
  		int pid = fork();
  		if (pid == 0)
  		{
  			std::cout << "<i> [Дочерний поток " << i << " начал выполнение]\n"; 
  			for (int j = i * chunk_size; j < i * chunk_size + chunk_size; j++) 
		    {
		    		if ( (j % N + j / N)%2 == 1 ) // считаем нечетная ли сумма индексов
		    		{
		    			semop( shmcontrol_semid, &Minus1, 1); // Блокировка ресурса
			        	mem_sum->sum += B[j];
			        	mem_sum->counter += 1;
			        	semop( shmcontrol_semid, &Plus1, 1); // Высвобождение ресурса
		    		}
		    }
  			exit(0);
  		}
    	
  	}
  	wait(&status2);
  	sleep(1);
  	std::cout << "<i> [Мастер-поток дождался дочерних потоков]\n"; 

	std::cout << "Массив B (Наибольшие делители):" << std::endl;
	for (int i = 0; i < A.size(); i++)
	{
			std::cout << B[i] << " ";
			if (i % N == N - 1) std::cout << std::endl;
	}

	std::cout << "Сумма элементов массива B, лежащих на нечётной сумме индексов: " << mem_sum->sum << std::endl;
	std::cout << "Количество элементов массива B, лежащих на нечётной сумме индексов: " << mem_sum->counter << std::endl;
	std::cout << "Среднее арифметическое: " << (static_cast<double>(mem_sum->sum) / mem_sum->counter) << std::endl;

	return 0;
}