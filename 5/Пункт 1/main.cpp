#include <iostream>
#include <thread>
#include <chrono>
#include <semaphore>
#include <set>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sched.h>
#include <sys/sem.h>

// Структура разделяемой памяти.
struct flags{
	bool flag_possible = false; // Флаг возможности создания слова S из символьного массива
	// Этот флаг будет изменяться потоками в критической секции.
} *flagss;

int main()
{
	int shmid = shmget(IPC_PRIVATE, 2, IPC_CREAT|0666); // Инициализация разделяемой памяти;

	flagss = (flags *)shmat(shmid,NULL,0); // Создание разделяемого флага;


	// Ввод слова S
	std::string S;
	std::cout << "\nСлово S: \n";
	std::cin >> S;
	

	// Ввод символов
	char symbol;
	std::vector<char> symbols;
	while(symbol != '0')
	{
		std::cout << "Добавить символ (0 для завершения): \n";
		std::cin >> symbol;
		if (symbol != '0') symbols.push_back(symbol);
	}

	// Вывод на экран перед выполнением
	std::cout << "Слово: " << S << "\n";
	std::cout << "Cимволы:";
	for(char smb : symbols)
	{
		std::cout << " " << smb;
	}
	std::cout << std::endl;


	int num_of_permutations = 0; // Счётчик количества возможных перестановок символов.

	std::vector<std::string> words; // Хранилище всех возможных слов из символов массива.

	std::sort(symbols.begin(), symbols.end());
	do{
		std::string word = "";
		for(int i = 0; i < symbols.size(); i++) 
		{
			word += symbols[i];
		}
		words.push_back(word);
		num_of_permutations++;
	} while (std::next_permutation(symbols.begin(), symbols.end()));
	// ^^^ Этот блок создаёт все возможные комбинации символов и записывает их в массив слов


	std::set<char> unique_symbols;
	for(char smb : symbols)
	{
		unique_symbols.insert(smb);
	}
	// Количество потоков = количеству уникальных символов
	short num_of_threads = unique_symbols.size(); 

	// Обьявление семафоров.
	int shmcontrol_semid; // Семафор контроля критической секции.
	shmcontrol_semid = semget (IPC_PRIVATE, 1, IPC_CREAT|0666) ;
	int thread_control_semid; // Семафор контроля выполнения потоков.
	thread_control_semid = semget (IPC_PRIVATE, 1, IPC_CREAT|0666) ;

	// Проверка валидности семафоров
	if (shmcontrol_semid < 0 || thread_control_semid < 0)
	{
		std:: cout << "Ошибка обьявления семафоров.";
		return -1;
	}

	// Операции над семафорами
	sembuf Plus1 = 			{shmcontrol_semid, 1, 0} ; 						//операция прибавляет единицу к семафору контроля критической секции
	sembuf Minus1 = 		{shmcontrol_semid, -1, 0} ; 					//операция вычитает единицу от семафора контроля критической секции
	sembuf PlusThread = 	{thread_control_semid, 1, 0} ;					//операция прибавляет единицу к семафору контроля выполнения потоков
	sembuf MinusThread = 	{thread_control_semid, -num_of_threads, 0};		//операция блокирует мастер-поток

	std::cout << "Число потоков: " << num_of_threads << std::endl;
	std::cout << "Число комбинаций: " << num_of_permutations << std::endl;

	unsigned chunk_size = num_of_permutations / num_of_threads; // Размер куска массива, который будет передан каждому потоку
	std::cout << "Каждый поток получит " << chunk_size << std::endl;

	semop( shmcontrol_semid, &Plus1, 1); // Семафор контроля критической секции должен быть равен 1, чтобы какой-либо поток смог его заблокировать.
	for (int i = 0; i < num_of_threads; i++)
	{

		if (fork() == 0) // С этого момента начинается выполнения потомков
		{
			
			std::cout << "Обрабатываю " << i * chunk_size << " по " << i * chunk_size + chunk_size;
			for (int j = i * chunk_size; j < i * chunk_size + chunk_size; j++) 
			{
				if(words[j].compare(S) == 0) 
					{
						semop( shmcontrol_semid, &Minus1, 1); // Блокировка ресурса
						// === Критическая секция ===
						flagss->flag_possible = true;
						// === Критическая секция ===
						semop( shmcontrol_semid, &Plus1, 1); // Высвобождение ресурса
					}
			}

			std::cout << "[Поток " << i << "]\n"; 
			semop( thread_control_semid, &PlusThread, 0); return 0; // Говорим мастеру что поток завершен.
		}	
		
	}
	semop( thread_control_semid, &MinusThread, SEM_UNDO); 	// Блокируем мастера пока есть выполняющиеся потоки.
	sleep(1); 												// Ожидаем синхронизации в этой точке.
	if (flagss->flag_possible) std::cout << "Слово возможно составить из данных букв\n";
	else std::cout << "Слово невозможно составить из данных букв\n";
	return 0;
}