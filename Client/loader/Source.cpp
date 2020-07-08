#include <windows.h>
#include <string>
#include <iostream>
#include <assert.h>
#include <TlHelp32.h>
#include <iphlpapi.h>
#include <sstream>
#include <algorithm>
#include "xor.hpp"
#include "VMProtectSDK.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Iphlpapi.lib")
#pragma comment (lib,"Advapi32.lib")
#pragma comment (lib, "ntdll.lib")
#pragma comment (lib, "Urlmon.lib")

#pragma warning(disable:4996) 
SOCKET Connection;

BOOL isValidInput(char* pszInput)
{
	//Empty?
	if (strlen(pszInput) == 0)
		return FALSE;

	//Got space?
	for (unsigned int ax = 0; ax < strlen(pszInput); ax++)
	{
		if (pszInput[ax] == char(32))
			return FALSE;
	}

	return TRUE;
}

std::string m_CPU;
std::string m_ComputerName;
std::string m_Physical;
std::string hwid;

bool query_wmic(const std::string& input, std::string& out)
{
	auto* shell_cmd = _popen(input.c_str(), "r");
	if (!shell_cmd) {
		return false;
	}

	static char buffer[1024] = {};
	while (fgets(buffer, 1024, shell_cmd)) {
		out.append(buffer);
	}
	_pclose(shell_cmd);

	while (out.back() == '\n' ||
		out.back() == '\0' ||
		out.back() == ' ' ||
		out.back() == '\r' ||
		out.back() == '\t') {
		out.pop_back();
	}

	return !out.empty();
}

bool query()
{
	auto strip_keyword = [](std::string& buffer, const bool filter_digits = false)
	{
		std::string current, stripped;
		std::istringstream iss(buffer);

		buffer.clear();
		auto first_tick = false;
		while (std::getline(iss, current)) {
			if (!first_tick) {
				first_tick = true;
				continue;
			}
			if (filter_digits && std::isdigit(current.at(0))) {
				continue;
			}

			buffer.append(current).append("\n");
		}
		if (buffer.back() == '\n') {
			buffer.pop_back();
		}
	};

	XorS(test, "wmic cpu get name");
	XorS(test1, "WMIC OS GET CSName");
	XorS(test2, "WMIC diskdrive get SerialNumber");

	if (!query_wmic(test.decrypt(), m_CPU) ||
		!query_wmic(test1.decrypt(), m_ComputerName) ||
		!query_wmic(test2.decrypt(), m_Physical)) {
		return false;
	}

	strip_keyword(m_CPU);
	strip_keyword(m_ComputerName);
	strip_keyword(m_Physical, true);

	return true;
}

void load()
{
	XorS(chet, "Select Cheat.");
	XorS(pubg1, "Press f1 for Pubg!");
	
	system("cls");
	std::cout << chet.decrypt() << std::endl;
	std::cout << pubg1.decrypt() << std::endl;
	
	while (true)
	{
		if (GetAsyncKeyState(VK_F1))
		{
			system("cls");
			XorS(pubg1, "Loading Pubg!");
			std::cout << pubg1.decrypt() << std::endl;

			
		}
		Sleep(1);
	}
}

int main()
{
	VMProtectBeginUltra("1");

	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);

	if (WSAStartup(DllVersion, &wsaData) != 0)
	{
		return 0;
	}

	SOCKADDR_IN addr; //Address to be binded to our Connection socket
	int sizeofaddr = sizeof(addr); //Need sizeofaddr for the connect function
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Address = localhost (this pc)
	addr.sin_port = htons(1111); //Port = 1111
	addr.sin_family = AF_INET; //IPv4 Socket

	Connection = socket(AF_INET, SOCK_STREAM, NULL); //Set Connection socket

	if (connect(Connection, (SOCKADDR*)&addr, sizeofaddr) != 0) //If we are unable to connect...
	{
		XorS(fail, "Failed to Connect Server.");
		XorS(err, "Error");

		MessageBoxA(NULL, fail.decrypt(), err.decrypt(), MB_OK | MB_ICONERROR);
		return 0; //Failed to Connect
	}
	XorS(conn, "Connected to Server.");
	std::cout << conn.decrypt() << std::endl;

reset:
	char username[32];
	char password[32];
	char loginstatus[10];

	XorS(user, "Enter your username = ");
	std::cout << user.decrypt();

	std::cin.getline(username, sizeof(username));

	if (!isValidInput(username))
	{
		XorS(user1, "Invalid username.");
		std::cout << user1.decrypt() << std::endl;
		Sleep(1500);
		system("cls");
		goto reset;
	}

	XorS(pass1, "Enter your Password = ");
	std::cout << pass1.decrypt();

	std::cin.getline(password, sizeof(password));

	if (!isValidInput(password))
	{
		XorS(pass, "Invalid password.");
		std::cout << pass.decrypt() << std::endl;
		Sleep(1500);
		system("cls");
		goto reset;
	}

	query();
	hwid += m_CPU + m_ComputerName + m_Physical;
	hwid.erase(std::remove_if(hwid.begin(), hwid.end(), isspace), hwid.end());

	char* cstr = new char[hwid.length() + 1];
	strcpy(cstr, hwid.c_str());

	send(Connection, "login", sizeof("login"), NULL);

	Sleep(500);

	send(Connection, username, sizeof(username), NULL);
	send(Connection, password, sizeof(password), NULL);
	send(Connection, cstr, hwid.length() + 1, NULL);

	recv(Connection, loginstatus, sizeof(loginstatus), NULL);

	delete[] cstr;

	XorS(logintrue, "true");
	XorS(loginfail, "false");
	XorS(loginhwid, "hwid");
	XorS(expired, "expired");

	if (strcmp(loginstatus, logintrue.decrypt()) == 0)
	{
		load();
	}
	else if (strcmp(loginstatus, loginfail.decrypt()) == 0)
	{
		XorS(failed, "failed to login!");
		std::cout << failed.decrypt() << std::endl;
		Sleep(3000);
		ExitProcess(0);
	}
	else if (strcmp(loginstatus, loginhwid.decrypt()) == 0)
	{
		XorS(invalid, "invalid hwid!");
		std::cout << invalid.decrypt() << std::endl;
		Sleep(3000);
		ExitProcess(0);
	}
	else if (strcmp(loginstatus, expired.decrypt()) == 0)
	{
		XorS(expired1, "License Expired, Go Buy new one!");
		std::cout << expired1.decrypt() << std::endl;
		Sleep(3000);
		ExitProcess(0);
	}

	VMProtectEnd();
	return 0;
}

