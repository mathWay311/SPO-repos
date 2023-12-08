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

/*
ВАР 17
б) Первое приложение отсылает второму двумерную квадратную матрицу В.
Второе приложение заполняет матрицу С, которая получается из В ее
транспонированием и обнулением всех элементов, больших по модулю чем 10.
Второе приложение отсылает матрицу С первому, которое вычисляет
произведение матриц В и С.
*/

key_t msg_send_key = 123;
key_t msg_recv_key = 900;

int get_message_queue_SEND()
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

int get_message_queue_RECV()
{
    // Создать очередь сообщений
    int msg_recv_id = msgget(msg_recv_key, IPC_CREAT | 0666);

    // Если не удалось создать очередь сообщений, завершить выполнение
    if (msg_recv_id < 0) {
        fprintf(stdout, "\nError");
        return 0;
    }

    return msg_recv_id;
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

void send_matrix(int *A, int msgid, int number_of_elems)
{
    struct msg message;

	message.mtype = 500;    // 500 - Идентификатор сообщения, содержащего количество элементов
	message.mtext = number_of_elems;
	msgsnd(msgid, &message, sizeof(struct msgbuf), 0);

    for(int i = 0; i < number_of_elems; i++)
    {
        struct msg message;
        message.mtype = 500 + i;    // 500 + i - непосредственно особые элементы
        message.mtext = A[i];
        msgsnd(msgid, &message, sizeof(struct msgbuf), 0);
    }

    std::cout << "<i> Сообщение отправлено\n";
}

void array_entry(int *A, int N)
{
	std::cout << "Теперь необходимо ввести " << N * N << " элементов массива А. (можно просто через пробел)" << std::endl;
    for (int i = 0; i < N * N; i++)
    {
        int temp_num;
        std::cin >> temp_num;
        A[i] = temp_num;
    }
}

void print_array(int *A, int N)
{
	std::cout << "Массив А:\n";
    for (int i = 0; i < N * N; i++)
    {
        std::cout << A[i] << " ";
        if (i % N == N - 1) std::cout << std::endl;
    }
    std::cout << std::endl;
}

int main(int argc, char const *argv[])
{
	release_message_queue(get_message_queue_RECV()); // Очищаем очередь получения чтобы не было ложных срабатываний
	std::cout << "Введите число N: ";
    unsigned N;
    std::cin >> N;

    int A[N * N];

    array_entry(A, N);
    print_array(A, N);
    
    int delay;
    std::cout << "Перед отправкой сообщения укажите задержку отправки (сек): ";
    std::cin >> delay;

    sleep(delay);

    int msgid = get_message_queue_SEND();

    send_matrix(A, msgid, N * N);


    int msg_recv_id = get_message_queue_RECV();
    struct msg message;

	std::cout << "<i> Ожидание сообщения...\n";

	msgrcv(msg_recv_id, &message, sizeof(struct msgbuf), 500, 0);

	std::cout << "<i> Сообщение получено. Начинаю проводить операции...\n";

    int number_of_elems = message.mtext;
    std::cout << "<i> Количество элементов: " << number_of_elems << "\n";
    int B[number_of_elems];

    for (int i = 0; i < number_of_elems; i++)
    {
        msgrcv(msg_recv_id, &message, sizeof(struct msgbuf), 500+i, 0);
        int element = message.mtext;
        B[i] = element;
    }

    // Вывод массива на экран.
    std::cout << "<i> Принял матрицу:\n";
    for (int i = 0; i < number_of_elems; i++)
    {
        std::cout << B[i] << " ";
        if (i % N == N - 1) std::cout << std::endl;
    }
    std::cout << std::endl;

	return 0;
} 
