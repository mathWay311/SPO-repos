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
#include <cmath>

/*
ВАР 17
б) Первое приложение отсылает второму двумерную квадратную матрицу В.
Второе приложение заполняет матрицу С, которая получается из В ее
транспонированием и обнулением всех элементов, больших по модулю чем 10.
Второе приложение отсылает матрицу С первому, которое вычисляет
произведение матриц В и С.
*/

key_t msg_recieve_key = 123;
key_t msg_send_key = 900;

int get_message_queue()
{
    // Создать очередь сообщений
    int msgid = msgget(msg_recieve_key, IPC_CREAT | 0666);

    // Если не удалось создать очередь сообщений, завершить выполнение
    if (msgid < 0) {
        fprintf(stdout, "\nError");
        return 0;
    }

    return msgid;
}

int get_message2_queue()
{
    // Создать очередь сообщений
    int msgid = msgget(msg_send_key, IPC_CREAT | 0666);

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

void send_transposed_matrix(int *Arr, int msgid, int arr_size)
{
	struct msg message3;
    message3.mtype = 500;
    message3.mtext = arr_size;
    msgsnd(msgid, &message3, sizeof(struct msgbuf), 0);

    for(int i = 0; i < arr_size; i++)
    {
        struct msg message2;
        message2.mtype = 500 + i;    // 500 + i - непосредственно особые элементы
        message2.mtext = Arr[i];
        msgsnd(msgid, &message2, sizeof(struct msgbuf), 0);
    }

    std::cout << "<i> Сообщение отправлено\n";
}

void wait_timer(int secs)
{
	for (int i = secs; i > 0; i--)
    {
    	std::cout << "<i> Отправка через " << i << std::endl;
    	sleep(1);	
    }
}

int main(int argc, char const *argv[])
{
	int msgid = get_message_queue();
    struct msg message;

	std::cout << "<i> Ожидание сообщения...\n";

	msgrcv(msgid, &message, sizeof(struct msgbuf), 500, 0);

	std::cout << "<i> Сообщение получено. Начинаю проводить операции...\n";

    int number_of_elems = message.mtext;
    std::cout << "<i> Количество элементов: " << number_of_elems << "\n";
    int B[number_of_elems];
    int C[number_of_elems];

    for (int i = 0; i < number_of_elems; i++)
    {
        msgrcv(msgid, &message, sizeof(struct msgbuf), 500+i, 0);
        int element = message.mtext;
        B[i] = element;
    }

    int N = sqrt(number_of_elems);
    for (int a = 0; a < number_of_elems; a++)
    {
        int i = a % N;
        int j = a / N;
        C[i * N + j] = B[j * N + i];
        if (abs(C[i * N + j]) > 10) C[i * N + j] = 0; // зануляем элемент если он больше 10 по модулю
    }

    // Вывод массива на экран.
    std::cout << "<i> Отсылаю транспонированную матрицу:\n";
    for (int i = 0; i < number_of_elems; i++)
    {
        std::cout << C[i] << " ";
        if (i % N == N - 1) std::cout << std::endl;
    }
    std::cout << std::endl;
    
    wait_timer(3);

	int send_message_id = get_message2_queue();
	send_transposed_matrix(C, send_message_id, number_of_elems);

	return 0;
} 
