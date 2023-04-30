#include <iostream>
#include <string>
#include <winsock2.h>

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib") // Для работы с сокетами

using namespace std;

SOCKET Connections[100]; // массив сокетов 
int Counter = 0; // иднекс массива(сокета/клиента)

void ClientHandler(int index) {
	char msg[256];
	while (true) {
		recv(Connections[index], msg, sizeof(msg), NULL); // принимает сообщение клиента
		for (int i = 0; i < Counter; ++i) {
			if (i == index)
				continue;
			send(Connections[i], msg, sizeof(msg), NULL); // отправляет клиенту 
		}
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
	int c = 3;
	listen(slisten, c); // прослушивание, сколько запросов ожидается, (остальные получат ошибку)

	cout << "Waiting for client connection..." << endl;

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

			send(newConnection, msg, sizeof(msg), NULL); // отправка клиенту номер клиента 

			Connections[i] = newConnection;
			++Counter;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE) ClientHandler, (LPVOID)(i), NULL, NULL); // создаём новый поток, в котором будет выполняться функция ClientHandler
		}
	}
	system("pause");
	return 0;
}