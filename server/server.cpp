#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <sstream>
#include <windows.h>
#include <ws2tcpip.h>
#include <condition_variable>
#include <queue>

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib") // Для работы с сокетами

std::mutex g_mutex;
std::condition_variable g_condVar;
std::queue<SOCKET> g_clientQueue;
int g_numThreads = 0;
int counter = 0;

const int MAX_THREADS = 2;
const int MAX_QUEUE_SIZE = 10;

using namespace std;


/*

void ClientHandler(int index) {
	std::string s = "Stop! You are not active yet.";
	char msg[256];
	strcpy(msg, s.c_str());

	//SOCKET client_socket = (SOCKET)arg;
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
*/

bool processClient(SOCKET clientSocket, int index) {
	// Получаем данные от клиента
	char buffer[1024];
	int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

	// Обработка выражения
	if (bytesReceived) {
		if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
			closesocket(clientSocket);
			cout << "Client: " << index << " disconnect" << endl;
			Sleep(500);
			return 0;
		}
		else {
			cout << "from " << index << " : " << buffer << endl;
			string str(buffer);
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
			string m = "To client: " + to_string(res);
			char msg[sizeof(m)];
			strcpy_s(msg, m.c_str());							// Преобразование std::string в char[]
			cout << "The result sent to " << index << " client" << endl;
			int bytesSent = send(clientSocket, msg, sizeof(msg), 0);				
		}
	}

	else {
		closesocket(clientSocket);
		return 0;
		
	}
}


void threadFunc() {
	int i = 1;
	while (true) {
		SOCKET clientSocket;

		{
			std::unique_lock<std::mutex> lock(g_mutex);

			// Ожидаем клиента, если все потоки заняты
			while (g_clientQueue.empty()) {
				g_condVar.wait(lock);
			}

			// Извлекаем следующего клиента из очереди
			clientSocket = g_clientQueue.front();
			g_clientQueue.pop();
		}

		// Обрабатываем запрос клиента
		bool fd;
		do {
			fd = processClient(clientSocket, i);
		} while (fd != 0);
			
		++i;

		// Закрываем сокет клиента
		closesocket(clientSocket);
		{
			// Освобождаем один слот для нового клиента
			std::lock_guard<std::mutex> lock(g_mutex);
			--g_numThreads;
			--counter;
		}

		//cout << "counter: " << counter << endl;
		if (counter == 0) {
			closesocket(clientSocket);
			WSACleanup();
			exit (0);
		}
		g_condVar.notify_one(); // Уведомляем потоки, что есть свободный слот
	}
}


int main() {
	WSADATA wsaData; // создаём структуру wsaData
	WORD DLLVersion = MAKEWORD(2, 1); // используется для указания версии Winsock, которая будет запрошена при инициализации библиотеки с помощью функции WSAStartup
	if (WSAStartup(DLLVersion, &wsaData) != 0) { // проверка на инициализацию библиотеки Winsock
		std::cout << "Error for inicialization Winsock! " << endl;
		exit(1);
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr; // структура для хранение адреса
	int sizeofaddr = sizeof(addr); //размер
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP-адрес сервера (локал хост)
	addr.sin_port = htons(1111); // Порт, на котором будет слушать сервер
	addr.sin_family = AF_INET;   // семейство протоколов, для интерент протоколов: AF_INET

	if (bind(serverSocket, (SOCKADDR*)&addr, sizeof(addr)) != 0) {		 //проверка на привязку адреса сокету
		closesocket(serverSocket);
		WSACleanup();
	}

	listen(serverSocket, SOMAXCONN); // прослушивание, сколько запросов ожидается, (остальные получат ошибку)

	HANDLE semaphore = CreateSemaphore(nullptr, MAX_THREADS, MAX_THREADS, nullptr);
	std::cout << "Waiting for client connection..." << endl;

	std::vector<std::thread> threads;
	for (int i = 0; i < MAX_THREADS; ++i) {
		threads.emplace_back(threadFunc);
	}

	int o = 1;
	// Ожидаем клиентов и добавляем их в очередь
	while (true) {
		SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
		++counter;
		string mes = "Client " + to_string(o) + " is connected";
		char msg[sizeof(mes)];
		strcpy_s(msg, mes.c_str());
		send(clientSocket, msg, sizeof(msg), 0);
		
		if (clientSocket == INVALID_SOCKET) {
			cout << "All clients are disconected" << endl;
			/*
			for (auto& thread : threads) 
				thread.join();
			
			// Закрываем сокеты и освобождаем ресурсы
			CloseHandle(semaphore);
			closesocket(serverSocket);
			WSACleanup();
			
			return 0;*/
		}

		else {
			Sleep(1000);
			std::unique_lock<std::mutex> lock(g_mutex);

			// Ожидаем свободный слот
			while (g_numThreads == MAX_THREADS) {
				g_condVar.wait(lock);
				send(clientSocket, "Client to wait ", 64, 0);
			}

			// Добавляем клиента в очередь
			g_clientQueue.push(clientSocket);
			send(clientSocket, "Client to queue ", 64, 0);
			++g_numThreads;
		}
		g_condVar.notify_one(); // Уведомляем потоки о новом клиенте
		++o;
	}

	
	// Ожидаем завершения всех потоков
	for (auto& thread : threads) {
		thread.join();
	}

	// Закрываем сокеты и освобождаем ресурсы
	CloseHandle(semaphore);
	closesocket(serverSocket);
	WSACleanup();
	return 0;





			/*

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
	*/
}