#include <string>
#include <fstream>
#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <time.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable:4996) 

using namespace std;

SOCKET Connections[250];
int ConnectionCounter = 0;
bool loginstatus = 0;

const int SIZE1 = 250;
string Usernames[SIZE1];
string Password[SIZE1];
string Expirydate[SIZE1];

int GetNameIndex(string query, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (query == Usernames[i]) 
			return i;
	}
	return -1;
}

bool PasswordMatches(int index, string passwd)
{
	return (passwd == Password[index]);
}

bool HwidMatches(string hwid, string user)
{
	fstream test;
	test.open(user + "_hwid.txt");

	if (test.fail()) {
		fstream file;
		file.open(user + "_hwid.txt", ios::out);

		file << hwid << endl;
		file.close();
		return 1;
	}

	char hwid1[200];
	fstream file;
	file.open(user + "_hwid.txt", ios::in);
	file >> hwid1;

	if (hwid == hwid1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

long getCurrentLocalDate()
{
	time_t rawtime;
	struct tm* timeinfo;
	char buffer[10];
	long lbuff;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, sizeof(buffer), "%Y%m%d", timeinfo);
	lbuff = atol(buffer);
	return lbuff;
}

bool isExpired(long dueDate)
{
	long currdt = getCurrentLocalDate();
	return (currdt > dueDate) ? true : false;
}

void ClientHandlerThread(int conindex)
{
	try
	{
		char action[256];
		recv(Connections[conindex], action, sizeof(action), NULL);

		if (strcmp(action, "login") == 0)
		{
			char username[256];
			char password[256];
			char hwid[256];

			recv(Connections[conindex], username, sizeof(username), NULL);
			recv(Connections[conindex], password, sizeof(password), NULL);
			recv(Connections[conindex], hwid, sizeof(hwid), NULL);

			ifstream usr("users.txt");
			int i = 0;

			while (!usr.eof())
			{
				usr >> Usernames[i] >> Password[i] >> Expirydate[i];
				i++;
			}
			int index = GetNameIndex(username, i);

			if (PasswordMatches(index, password))
			{
				if (HwidMatches(hwid, username))
				{
					std::string::size_type test;
					long expirydate = std::stol(Expirydate[index], &test);

					if (!isExpired(expirydate))
					{
						send(Connections[conindex], "true", sizeof("true"), NULL);
						loginstatus = false;
						closesocket(Connections[conindex]);
						ConnectionCounter -= 1;
						ExitThread(1);
					}
					else
					{
						send(Connections[conindex], "expired", sizeof("expired"), NULL);
						loginstatus = false;
						closesocket(Connections[conindex]);
						ConnectionCounter -= 1;
						ExitThread(1);
					}
				}
				else
				{
					send(Connections[conindex], "hwid", sizeof("hwid"), NULL);
					loginstatus = false;
					closesocket(Connections[conindex]);
					ConnectionCounter -= 1;
					ExitThread(1);
				}
			}
			else
			{
				send(Connections[conindex], "false", sizeof("false"), NULL);
				cout << "client failed to login" << endl;
				closesocket(Connections[conindex]);
				ConnectionCounter -= 1;
				ExitThread(1);
			}
		}
	}
	catch (...)
	{
	
	}

	closesocket(Connections[conindex]);
	ConnectionCounter -= 1;
	ExitThread(1);
}

int main()
{
	//Winsock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) //If WSAStartup returns anything other than 0, then that means an error has occured in the WinSock Startup.
	{
		MessageBoxA(NULL, "WinSock startup failed", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}
	
	SOCKADDR_IN addr; //Address that we will bind our listening socket to
	int addrlen = sizeof(addr); //length of the address (required for accept call)
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Broadcast locally
	addr.sin_port = htons(1111); //Port
	addr.sin_family = AF_INET; //IPv4 Socket

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL); //Create socket to listen for new connections
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); //Bind the address to the socket
	listen(sListen, SOMAXCONN); //Places sListen socket in a state in which it is listening for an incoming connection. Note:SOMAXCONN = Socket Oustanding Max Connections

	SOCKET newConnection; //Socket to hold the client's connection
	ConnectionCounter = 0; //# of client connections
	
	for (int i = 0; i < 250; i++)
	{
		newConnection = accept(sListen, (SOCKADDR*)&addr, &addrlen); //Accept a new connection
		
		if (newConnection == 0) //If accepting the client connection failed
		{
			std::cout << "Failed to accept the client's connection." << std::endl;
		}
		else //If client connection properly accepted
		{
			std::cout << "Client Connected!" << std::endl;
			Connections[i] = newConnection; //Set socket in array to be the newest connection before creating the thread to handle this client's socket.
			ConnectionCounter += 1; //Incremenent total # of clients that have connected
			std::cout << "clients connected: " << ConnectionCounter << std::endl;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandlerThread, (LPVOID)(i), NULL, NULL); //Create Thread to handle this client. The index in the socket array for this thread is the value (i).
		}
	}
	system("pause");
	return 0;
}