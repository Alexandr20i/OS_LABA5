#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <sstream>

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib") // Для работы с сокетами

using namespace std;

const int MAX_CONNECTION = 2; //константа для макс количества клиентов
SOCKET Connections[10]; // массив сокетов 
int Counter = 0; // иднекс массива(сокета/клиента)


void ClientHandler(int index) {
	std::string s = "Stop! You are not active yet.";
	char msg[256];
	strcpy(msg, s.c_str());
	while (true) {

		int ds = recv(Connections[index], msg, sizeof(msg), NULL);

		if (ds) {
			if (ds == SOCKET_ERROR || ds == 0) {
				cout << "Client: " << index << " disconnect" << endl;
				Sleep(500);
				closesocket(Connections[index]);							
				--Counter;
				//cout << "Counter: " << Counter << endl;
				if (Counter == 1) {
					WSACleanup(); //освобождение использованных ресурсов
					exit(0);
					return;
				}
				return;
			}
			else {
				cout << "from " << index << " : " << msg << endl;
				string str(msg);
				int res;				// результат 
				stringstream ss(str);
				char sign;
				int first_number, second_number;
				ss >> sign >> first_number >> second_number;

				if (sign == '+') {
					res = first_number + second_number;					
				}
				else if (sign == '-') {
					res = first_number - second_number;					
				}
				else if (sign == '/') {
					res = first_number / second_number;			
				}
				else if (sign == '*') {
					res = first_number * second_number;
				}
				string m = "To " + to_string(index) + " client: " + to_string(res);
				char msg[sizeof(m)];
				strcpy_s(msg, m.c_str());							// Преобразование std::string в char[]
				send(Connections[Counter], msg, sizeof(msg), NULL); // отправка клиенту результат
				cout << "The result sent to " << index << " client" << endl;
			}
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
	addr.sin_port = htons(1111); // Порт, на котором будет слушать сервер
	addr.sin_family = AF_INET;   // семейство протоколов, для интерент протоколов: AF_INET

	SOCKET slisten = socket(AF_INET, SOCK_STREAM, NULL); //сокет для прослушивания порта
	if (bind(slisten, (SOCKADDR*)&addr, sizeof(addr)) != 0) {		 //проверка на привязку адреса сокету
		closesocket(slisten);
		WSACleanup();
	}

	//int c = 2; // кол-во клиентов, которые могут подключиться
	listen(slisten, 2); // прослушивание, сколько запросов ожидается, (остальные получат ошибку)

	cout << "Waiting for client connection..." << endl;

	HANDLE threads[MAX_CONNECTION]; //инициализуем потоки


	do {
		++Counter;
		cout << "Counters: " << Counter << endl;
		//Connections[Counter] = accept(slisten, (SOCKADDR*)&addr, &sizeofaddr);
		if (Counter >= MAX_CONNECTION + 1) {
			
			//--Counter;
			Connections[Counter] = accept(slisten, (SOCKADDR*)&addr, &sizeofaddr);

			string m = "client " + to_string(Counter) + " is waiting ";
			char msgd[256] = "Client:  \n";
			strcpy(msgd, m.c_str());

			send(Connections[Counter], msgd, sizeof(msgd), NULL); // отправка клиенту его номер
			//while (Counter >= MAX_CONNECTION + 1);
			//--Counter;
		}
		else{
			//++Counter;
			Connections[Counter] = accept(slisten, (SOCKADDR*)&addr, &sizeofaddr);
			if (Connections[Counter] == INVALID_SOCKET) {
				cout << "Error connecting to the client:" << endl << WSAGetLastError() << endl; //случай ошибки
				closesocket(slisten);
				WSACleanup();
				return 1;
			}

			cout << "Client " << Counter << " connected!" << endl;
			// дальше идёт межсетевое взаимодействие

			string m = "client " + to_string(Counter) + " is connected";
			char msg[256] = "Client:  \n";
			strcpy(msg, m.c_str());

			send(Connections[Counter], msg, sizeof(msg), NULL); // отправка клиенту его номер

			//++Counter;

			if (Counter == MAX_CONNECTION + 1) {
				cout << "The maximum number of clients is connected!" << endl << "working with new clients is impossible!" << endl;
			}
			Sleep(1000);
			threads[Counter] = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(Counter), NULL, NULL); //обработка каждого клиента в отдельном потоке
			//++Counter;
		}
		cout << "Counters_after: " << Counter << endl;
		
	} while (Counter != 0);

	std::cout << "The server is closed to all streams!" << std::endl;
	closesocket(slisten);// и прослушивающего сокета
	WSACleanup(); //освобождение использованных ресурсов

	return 0;
}