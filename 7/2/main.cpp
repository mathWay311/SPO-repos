#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <limits.h>
#include <vector>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INT2POINTER(a) ((char*)(intptr_t)(a))
#define POINTER2INT(a) ((int)(intptr_t)(a))


/*
	Найти произведение всех особых элементов матрицы А. Будем считать, элемент
	особым, если он равен сумме всех остальных элементов, стоящих в той же строке.
	Входные данные: целое положительное число n, массив чисел А размерности nxn.
*/ 
key_t arr_size = 5024;  // Ключ очереди с размером массива
key_t elems = 19090;    // Ключ очереди с элементами массива
int get_message_queue(key_t key)
{
    // Создать очередь сообщений по ключу KEY
    int msgid = msgget(key, IPC_CREAT | 0666);

    // Если не удалось создать очередь сообщений, завершить выполнение
    if (msgid < 0) {
        fprintf(stdout, "\nError");
        return 0;
    }

    return msgid;
}



void release_message_queue(int message_queue_id)
{
    struct msqid_ds info;
    msgctl(message_queue_id, IPC_RMID, &info);
}


struct msg
{
    long mtype;
    int mtext;
} message;

int msgid;
int mult_result = 1;
int number_of_elems = 0;
int num_elements = 0;
int msgid_arr_size, msgid_arr;
bool any_elems_present = false;
void recieve_message(int signum)
{
    std::cout << "PID : "<< getpid() << std::endl;
    signal(SIGUSR1, recieve_message);   
    msgrcv(msgid_arr_size, &message, sizeof(struct msgbuf), 500, 0);
    num_elements = message.mtext;

    if(num_elements > 0) any_elems_present = true;

        for (int i = 0; i < num_elements; i++)
        {
            msgrcv(msgid_arr, &message, sizeof(struct msgbuf), 500+i, 0);
            int element = message.mtext;
            std::cout << "<i> Найден особый элемент - " << element << std::endl;
            mult_result *= element;    
        }

    release_message_queue(msgid_arr_size);
    release_message_queue(msgid_arr);
}

// Обработчик сигнала выключения по таймеру.
// int signum   ->  Идентификатор сигнала. (в данном случае не обрабатывается)
void terminated_by_alarm(int signum)
{
    signal(SIGALRM, recieve_message);
    printf("Terminated by alarm\n");

    // Высвобождаем очередь перед завершением.
    release_message_queue(msgid_arr_size);
    release_message_queue(msgid_arr);

    exit(EXIT_FAILURE);
}

// Процедура присвоения сигнала к обработчику
void set_reliable_signal(int signum, void(*sighandler)(int))
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = sighandler;
    //Зададим реакцию на приход сигнала SIGUSR1
    sigaction(signum, &act, 0);
}


int main()
{
    int alarm_time; // Время до срабатывания таймера
    set_reliable_signal(SIGALRM, terminated_by_alarm); // Установка надежного сигнала отключения по таймеру

    std::cout << "Введите время работы программы: ";
    std::cin >> alarm_time;
    alarm(alarm_time);


    std::cout << "Введите число N: ";
    unsigned N;
    std::cin >> N;


    std::vector<int> A; // Массив А
    std::cout << "Теперь необходимо ввести " << N*N << " элементов массива А. (можно просто через пробел)" << std::endl;
    for (int i = 0; i < N * N; i++)
    {
        int temp_num;
        std::cin >> temp_num;
        A.push_back(temp_num);
    }

    // Вывод массива на экран.
    std::cout << "Массив А:\n";
    for (int i = 0; i < A.size(); i++)
    {
        std::cout << A[i] << " ";
        if (i % N == N - 1) std::cout << std::endl;
    }
    std::cout << std::endl;

    // Получаем идентификаторы очереди
    msgid_arr_size = get_message_queue(arr_size);
    msgid_arr = get_message_queue(elems);

    // Инициализация структуры сообщения
    struct msg message;

    set_reliable_signal(SIGUSR1, recieve_message);                   // Установка по срабатыванию SIGUSR1 - вызвать recieve_message
    if(fork() == 0)
    {
        std::vector<int> special_elements;              // Массив особых элементов

        for(int i = 0; i < A.size()-N+1; i += N)
        {
            int sum = 0;
            for (int j = i; j < i + N; j++) sum += A[j]; // Считаем сумму всех элементов строки   
            
            for (int j = i; j < i + N; j++)             // Для каждого элемента строки
            {      
                if (sum - A[j] == A[j])                 // Если элемент равен сумме всех остальных 
                {
                    special_elements.push_back(A[j]);   // Добавить в массив особых элементов
                } 
            }

        }

        int number_of_spec_elems = special_elements.size(); // Количество особых элементов
        message.mtype = 500;    // 500 - Идентификатор сообщения, содержащего количество элементов
        message.mtext = number_of_spec_elems;
        ::number_of_elems = number_of_spec_elems; // Записываем количество элементов в глобальную переменную
        msgsnd(msgid_arr_size, &message, sizeof(struct msgbuf), 0); 

        for(int i = 0; i < number_of_spec_elems; i++)
        {
            struct msg message;
            message.mtype = 500 + i;    // 500 + i - непосредственно особые элементы
            message.mtext = special_elements[i];
            msgsnd(msgid_arr, &message, sizeof(struct msgbuf), 0);
        }
        sleep(1);
        kill(getppid(), SIGUSR1); // Вызов сигнала
        exit(0);
    }

    pause(); // Точка синхронизации

    std::cout << "Количество особых элементов:\t" << num_elements << "\n";            
        
    if (!any_elems_present) std::cout << "Нечего умножать. Особых элементов нет.\n";
    else std::cout << "Произведение особых элементов:\t" << mult_result << "\n";
    release_message_queue(msgid_arr_size);
    release_message_queue(msgid_arr);

    return EXIT_SUCCESS;
}