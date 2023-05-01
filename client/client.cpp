#include <iostream>
#include <winsock2.h>
#include <mutex>
#include <string>

using namespace std;

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib") // Для работы с сокетами

SOCKET Connection;

//проверка соединения с сервером (в случае внез)
void check_connect() {
	// отправка эхо-сообщения на сервер
	const char* echoMsg = "";
	send(Connection, echoMsg, strlen(echoMsg), 0);

	// ожидание ответа от сервера
	char buffer[10];
	memset(buffer, 0, sizeof(buffer));
	int bytesReceived = recv(Connection, buffer, sizeof(buffer), 0);

	// проверка на ошибку при приеме данных
	if (bytesReceived == SOCKET_ERROR) {
		cout << "Error: failed to receive data from server." << endl;
		closesocket(Connection);
		WSACleanup();
		exit(0);
	}

	// проверка на разрыв соединения с сервером
	if (bytesReceived == 0) {
		cout << "Error: server disconnected." << endl;
		closesocket(Connection);
		WSACleanup();
		exit(0);
	}
}

// принятие сообщения от сервера
void ClientHandler() {
	char msg[256];
	while (true) {

		check_connect();

		Sleep(500);

		if (recv(Connection, msg, sizeof(msg), NULL)) { // получение информации 
			Sleep(1000);
			cout << msg << endl;
		}
		else {
			return;
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
	while (true) {



		cin.getline(msgl, sizeof(msgl));
		string message(msgl);
		if (message == "exit")
			return 0;
		
		//Sleep(1000);
		send(Connection, msgl, sizeof(msgl), NULL);
		Sleep(1000);
	}
	
	closesocket(Connection);
	WSACleanup();

	system("pause");
	return 0;
}