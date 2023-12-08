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

#define INT2POINTER(a) ((char*)(intptr_t)(a))
#define POINTER2INT(a) ((int)(intptr_t)(a))


/*
	Найти произведение всех особых элементов матрицы А. Будем считать, элемент
	особым, если он равен сумме всех остальных элементов, стоящих в той же строке.
	Входные данные: целое положительное число n, массив чисел А размерности nxn.
*/ 

int get_message_queue()
{
    // Создать очередь сообщений
    int msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);

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
};


int main()
{
    std::cout << "Введите число N: ";
    unsigned N;
    std::cin >> N;

    std::vector<int> A;
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


    int msgid = get_message_queue();

    struct msg message;

    if(fork() == 0)
    {
        std::vector<int> special_elements;

        for(int i = 0; i < A.size()-N+1; i += N)
        {
            int sum = 0;
            for (int j = i; j < i + N; j++)
            {      
                sum += A[j]; // Считаем сумму всех элементов строки   
            }
            for (int j = i; j < i + N; j++)
            {      
                if (sum - A[j] == A[j]) // Если элемент равен сумме всех остальных 
                {
                    special_elements.push_back(A[j]);
                } 
            }

        }
        int number_of_spec_elems = special_elements.size();
        message.mtype = 500;    // 500 - Идентификатор сообщения, содержащего количество элементов
        message.mtext = number_of_spec_elems;
        msgsnd(msgid, &message, sizeof(struct msgbuf), 0);

        for(int i = 0; i < number_of_spec_elems; i++)
        {
            struct msg message;
            message.mtype = 500 + i;    // 500 + i - непосредственно особые элементы
            message.mtext = special_elements[i];
            msgsnd(msgid, &message, sizeof(struct msgbuf), 0);
        }

        exit(EXIT_SUCCESS);
    }
        //тип получаемого сообщения не важен
        msgrcv(msgid, &message, sizeof(struct msgbuf), 500, 0);
        int number_of_elems = message.mtext;
        int mult_result = 1;
        for (int i = 0; i < number_of_elems; i++)
        {
            msgrcv(msgid, &message, sizeof(struct msgbuf), 500+i, 0);
            int element = message.mtext;
            std::cout << "<i> Найден особый элемент - " << element << std::endl;
            mult_result *= element;
        }
        if (number_of_elems == 0) std::cout << "Нечего умножать. Особых элементов нет." << std::endl;
        else std::cout << "Произведение особых элементов:" << mult_result << "\n";
        release_message_queue(msgid);

    return EXIT_SUCCESS;
}