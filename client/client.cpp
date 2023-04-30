#include <iostream>
#include <winsock2.h>

using namespace std;

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib") // Для работы с сокетами

SOCKET Connection;

// принятие сообщения от сервера
void ClientHandler() {
	char msg[256];
	while (true) {
		recv(Connection, msg, sizeof(msg), NULL); // получение информации 
		cout << msg << endl;
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

	Connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0){ //проверка на подключение к серверу 
		cout << "Error: failed connect to server.\n";
		return 1;
	}
	
	cout << "Connected!\n"; //подключился
	/* // уже не надо, так как это делает void ClientHandler, запущенная в новом потоке 
	char msg[256];
	recv(Connection, msg, sizeof(msg), NULL);
	cout << msg << endl;
	*/

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

	char msgl[256];
	while (true) {
		cin.getline(msgl, sizeof(msgl));			
		send(Connection, msgl, sizeof(msgl), NULL);
		Sleep(3);
	}
	
	system("pause");
	return 0;
}