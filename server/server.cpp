#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib") // Для работы с сокетами

using namespace std;

HANDLE hMutex;

SOCKET Connections[10]; // массив сокетов 
int Counter = 0; // иднекс массива(сокета/клиента)

// Client 1: отправляет данные 
// Client 2: получает результат

void ClientHandler(int index) {
	char msg[256];
	while (true) {
		if (recv(Connections[index], msg, sizeof(msg), NULL)) { // принимает сообщение клиента
			cout << "from " << index << " : "<< msg  << endl;
		}
		else {
			return;
		}
		/*
		for (int i = 1; i <= Counter; ++i) {
			if (i == index)
				continue;
			Sleep(10);
			send(Connections[i], msg, sizeof(msg), NULL); // отправляет клиенту 
		}*/
	}
}

int main() {
	WSADATA wsaData; // создаём структуру wsaData
	WORD DLLVersion = MAKEWORD(2, 1); // используется для указания версии Winsock, которая будет запрошена при инициализации библиотеки с помощью функции WSAStartup
	if (WSAStartup(DLLVersion, &wsaData) != 0) { // проверка на инициализацию библиотеки Winsock
		cout << "Error for inicialization Winsock! " << endl;
		exit(1);
	}

	SOCKADDR_IN addr; // структура для хранение адреса
	int sizeofaddr = sizeof(addr); //размер
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP-адрес сервера (локал хост)
	//addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(1111); // Порт, на котором будет слушать сервер
	addr.sin_family = AF_INET;   // семейство протоколов, для интерент протоколов: AF_INET

	SOCKET slisten = socket(AF_INET, SOCK_STREAM, NULL); //сокет для прослушивания порта
	if (bind(slisten, (SOCKADDR*)&addr, sizeof(addr)) != 0) {		 //проверка на привязку адреса сокету
		closesocket(slisten);
		WSACleanup();
	}
	int c = 10; // кол-во клиентов, которые могут подключиться
	listen(slisten, SOMAXCONN); // прослушивание, сколько запросов ожидается, (остальные получат ошибку)

	cout << "Waiting for client connection..." << endl;

	hMutex = CreateMutex(NULL, FALSE, NULL);//создание мьютекса
	HANDLE threads[100]; //инициализуем потоки

	SOCKET newConnection;
	//newConnection = accept(slisten, (SOCKADDR*)&addr, &sizeofaddr); // 
	for (int i = 1; i <= c; ++i) {
		newConnection = accept(slisten, (SOCKADDR*)&addr, &sizeofaddr);
		if (newConnection == 0)
			cout << "Error conect with client" << endl;
		else {
			cout << "Client " << i << " connected!" << endl;
			// дальше идёт межсетевое взаимодействие

			string m = "client " + to_string(i) + " is connected";
			char msg[256] = "Client:  \n";
			strcpy(msg, m.c_str());

			send(newConnection, msg, sizeof(msg), NULL); // отправка клиенту его номер

			Connections[i] = newConnection;
			++Counter; 
			Sleep(3);
			//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE) ClientHandler, (LPVOID)(i), NULL, NULL); 
			threads[i] = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL); // создаём новый поток, в котором будет выполняться функция ClientHandler
		}
	}

	WaitForMultipleObjects(c, threads, TRUE, INFINITE); //ожидание заверешения работы всех потоков(клиентов)
	for (int i = 1; i <= Counter; ++i) {  //закрытие потоков
		CloseHandle(threads[i]);
		cout << "The threard " << i << " is closed" << endl;
	}

	std::cout << "The server is closed to all streams!" << std::endl;
	CloseHandle(hMutex); //закрытие мьютексов
	closesocket(slisten);// и прослушивающего сокета
	WSACleanup(); //освобождение использованных ресурсов

	system("pause");
	return 0;
}