#include <iostream>
#include <winsock2.h>
#include <mutex>
#include <string>
#include <sstream>

using namespace std;

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib") // Для работы с сокетами

SOCKET Connection;




// принятие сообщения от сервера
void ClientHandler() {
	char msg[256];
	while (true) {

		int ds = recv(Connection, msg, sizeof(msg), NULL);
			
		if (ds) {
			if (ds == SOCKET_ERROR || ds == 0) { // получение информации 
				cout << "Error, failed to receive data from server " << endl;
				closesocket(Connection);
				WSACleanup();
				exit(0);
			}
			cout << msg << endl;
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

	Connection = socket(AF_INET, SOCK_STREAM, NULL);//сокет для соединения с сервером
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);//создание сокета для прослушки порта
	
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0){ //проверка на подключение к серверу 
		cout << "Error: failed connect to server.\n";
		closesocket(Connection);
		WSACleanup();
		return 0;
	}
	
	cout << "Connected!\n"; //подключился	

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

	//Sleep(1000);
	char msgl[256];
	bool flag;
	while (true) {

		cin.getline(msgl, sizeof(msgl));

		string message(msgl);
		if (message == "exit")
			return 0;

		string str(msgl);
		stringstream ss(msgl);
		char sign;
		int first_number, second_number;
		try
		{
			flag = ss >> sign >> first_number >> second_number ? 1 : 0;
		}
		catch (...)
		{
			cout << "Incorrect input" << endl;
		}
		if (flag) {
			send(Connection, msgl, sizeof(msgl), NULL);
		}

	}
	
	closesocket(Connection);
	WSACleanup();

	system("pause");
	return 0;
}