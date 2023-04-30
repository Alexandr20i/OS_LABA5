#include <iostream>
#include <winsock2.h>
#include <mutex>

using namespace std;

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib") // Для работы с сокетами

SOCKET Connection;
const int MAX_CONNECTION = 2; //константа для макс количества клиентов
SOCKET Connections[MAX_CONNECTION]; //сокеты
HANDLE hMutex; //мьютекс для организации взаимодействия клиентов с сервером

// принятие сообщения от сервера
void ClientHandler() {
	char msg[256];
	while (true) {
		if (recv(Connection, msg, sizeof(msg), NULL)) { // получение информации 
			Sleep(100);
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

	Connection = socket(AF_INET, SOCK_STREAM, NULL);

	//привязка адреса к сокету
	bind(Connection, (SOCKADDR*)&addr, sizeof(addr));
	// прослушивание, сколько запросов ожидается
	listen(Connection, SOMAXCONN);

	hMutex = CreateMutex(NULL, FALSE, NULL);//создание мьютекса
	HANDLE threads[MAX_CONNECTION]; //инициализуем потоки
	if (hMutex == NULL)
		return GetLastError(); //ошибка создания мьютекса

	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR){ //проверка на подключение к серверу 
		cout << "Error: failed connect to server.\n";
		closesocket(Connection);
		WSACleanup();
		return 0;
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
		string message(msgl);
		if (message == "exit")
			return 0;

		send(Connection, msgl, sizeof(msgl), NULL);
		Sleep(3);
	}
	
	closesocket(Connection);
	WSACleanup();

	system("pause");
	return 0;
}