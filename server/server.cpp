#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <mutex>

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib") // Для работы с сокетами

using namespace std;

HANDLE hMutex;
const int MAX_CONNECTION = 3; //константа для макс количества клиентов
SOCKET Connections[10]; // массив сокетов 
int Counter = 1; // иднекс массива(сокета/клиента)

// Client 1: отправляет данные 
// Client 2: получает результат

bool check_connect(int i) {
	// отправка эхо-сообщения на сервер
	const char* echoMsg = "";
	send(Connections[i], echoMsg, strlen(echoMsg), 0);

	// ожидание ответа от сервера
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	int bytesReceived = recv(Connections[i], buffer, sizeof(buffer), 0);

	// проверка на ошибку при приеме данных
	if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
		cout << "Client: " << i << " disconnect" << endl;
		closesocket(Connections[i]);
		--Counter;
		return 0;
		//WSACleanup();
		//exit(0);
	}
	return 1;

}

void ClientHandler(int index) {
	char msg[256];
	while (true) {

		bool flag = check_connect(index); // проверяем соединение с клиентом

		if (Counter == 0) {

		}

		if (flag && recv(Connections[index], msg, sizeof(msg), NULL)) { // принимает сообщение клиента
			if (msg == "exit") {
				CloseHandle(hMutex); //закрытие мьютексов
				closesocket(Connections[index]);// и прослушивающего сокета
				WSACleanup(); //освобождение использованных ресурсов
				return;
			}

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

	/*
	wchar_t wpath[1000], wbuf[1000];
	//wcscpy_s(&wpath[0], 500, L"D:\\ОС\\LW5\\client\\Debug\\client.exe");
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	for (int i = 1; i <= MAX_CONNECTION; ++i) {
		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		memset(&pi, 0, sizeof(pi));
		HRESULT hr = CreateProcess(wpath, NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	}*/

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

	hMutex = CreateMutex(NULL, FALSE, NULL);//создание мьютекса

	HANDLE threads[MAX_CONNECTION]; //инициализуем потоки

	if (hMutex == NULL)
		return GetLastError(); //ошибка создания мьютекса
	
	int i = 1;
	cout << "Server interaction with clients:" << endl;
	//SOCKET newConnection;
	//newConnection = accept(slisten, (SOCKADDR*)&addr, &sizeofaddr); // 

	do {
		Connections[i] = accept(slisten, (SOCKADDR*)&addr, &sizeofaddr);
		if (Connections[i] == INVALID_SOCKET) { 
			cout << "Error connecting to the client:" << endl << WSAGetLastError() << endl; //случай ошибки
			closesocket(slisten);
			WSACleanup();
			return 1;
		}
		
		cout << "Client " << i << " connected!" << endl;
		// дальше идёт межсетевое взаимодействие
		
		string m = "client " + to_string(i) + " is connected";
		char msg[256] = "Client:  \n";
		strcpy(msg, m.c_str());

		send(Connections[i], msg, sizeof(msg), NULL); // отправка клиенту его номер

		++Counter; 
		
		if (i == MAX_CONNECTION) {
			cout << "The maximum number of clients is connected!" << endl << "working with new clients is impossible!" << endl;
		}
		Sleep(1000);
		threads[i] = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL); //обработка каждого клиента в отдельном потоке
		++i;
		
	} while (i <= MAX_CONNECTION && Counter != 0);

	WaitForMultipleObjects(MAX_CONNECTION, threads, TRUE, INFINITE); //ожидание заверешения работы всех потоков(клиентов)
	for (int i = 1; i <= Counter; ++i) {  //закрытие потоков
		CloseHandle(threads[i]);
		cout << "The threard " << i << " is closed" << endl;
	}

	std::cout << "The server is closed to all streams!" << std::endl;
	CloseHandle(hMutex); //закрытие мьютексов
	closesocket(slisten);// и прослушивающего сокета
	WSACleanup(); //освобождение использованных ресурсов

	return 0;
}