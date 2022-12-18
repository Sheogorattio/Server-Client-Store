#pragma once
#include<string>
#include<tchar.h>
#include<WS2tcpip.h>
#include"WinSock2.h"
#include<windows.h>
#include"resource.h"

#define MAXSTRLEN 100

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

WSADATA wsaData;
SOCKET _socket, acceptSocket;
sockaddr_in addr;
HWND hSendEdit, hMessagesEdit, hStart, hStop, hSend;


wstring History;
char receiveData[MAXSTRLEN], sendData[MAXSTRLEN];

int prices[3] = { 14000, 18000, 28000 };

wstring str;

BOOL SendData(char* str);
BOOL ReceiveData(char* buffer, INT size);
void Listen();
void ReceivingCycle();

BOOL CALLBACK DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			hSendEdit = GetDlgItem(hwnd, IDC_SendEdit);
			hMessagesEdit = GetDlgItem(hwnd, IDC_MessagesEdit);
			hStart = GetDlgItem(hwnd, IDOK);
			hStop = GetDlgItem(hwnd, IDCANCEL);
			hSend = GetDlgItem(hwnd, IDC_Send);

			EnableWindow(hSend, FALSE);

			return TRUE;
		}
		case WM_COMMAND:
		{
			if (wParam == IDOK)//start
			{
				if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
				{
					MessageBox(hwnd, L"WSAStartup error", L"Error", MB_ICONERROR | MB_OK);
					WSACleanup();
					exit(10);
				}

				_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (_socket == INVALID_SOCKET)
				{
					MessageBox(hwnd, L"Socket create error", L"Error", MB_ICONERROR | MB_OK);
					WSACleanup();
					exit(11);
				}

				addr.sin_family = AF_INET;
				inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
				addr.sin_port = htons(20000);
				if (bind(_socket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
				{
					MessageBox(hwnd, L"Failed to bind to port", L"Error", MB_OK | MB_ICONERROR);
					WSACleanup();
					exit(14);
				}
				SetWindowText(hMessagesEdit, L"Server is started\n");
				EnableWindow(hStart, FALSE);
				Listen();

				ReceiveData(receiveData, MAXSTRLEN);

				wstring ReceiveData = L"CLIENT:\t";
				for (int i = 0; i < strlen(receiveData); i++)
				{
					ReceiveData.push_back(receiveData[i]);
				}
				ReceiveData.push_back('\n');

				SetWindowText(hMessagesEdit, ReceiveData.c_str());
				ReceivingCycle();
			}
			else if (wParam == IDCANCEL)//stop
			{
				closesocket(_socket);
				
			}
			break;
		}
		case WM_CLOSE:
		{
			closesocket(acceptSocket);
			closesocket(_socket);
			WSACleanup();
			EndDialog(hwnd, 0);
			break;
		}
	}
	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdLine, int nCmdShow) {
	return DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc);
}

void Listen() {

	SetWindowText(hMessagesEdit, L"Waiting for client...");

	if (listen(_socket, 1) == SOCKET_ERROR)
	{
		MessageBox(0, L"Listening error", L"Error", MB_OK | MB_ICONERROR);
		WSACleanup();
		exit(15);
	}
	acceptSocket = accept(_socket, NULL, NULL);
	while (acceptSocket == SOCKET_ERROR)
	{
		acceptSocket = accept(_socket, NULL, NULL);
	}
	_socket = acceptSocket;
}

BOOL ReceiveData(char* buffer, INT size)
{
	int i = recv(_socket, buffer, size, 0);
	buffer[i] = '\0';
	return TRUE;
}

BOOL SendData( char* buffer)
{
	send(_socket, buffer, strlen(buffer), 0);
	return TRUE;
}

void ReceivingCycle() {
	while (1)
	{
		int code, number;
		char rez[MAXSTRLEN] = "total price is ";

		History.append(L"\tWaiting...\n");
		//Получаем данные от клиента
		//и сохраняем в переменной receiveMessage
		ReceiveData(receiveData, MAXSTRLEN);
		History.append(L"Received:\t");
		History.append((wchar_t*)receiveData);
		History.push_back('\n');
		code = atoi(receiveData);
		//Отправляем данные клиенту
		char EnterNum[] = "Enter number\n";
		SendData(EnterNum);
		History.append(L"\tWaiting...\n");
		History.push_back('\n');
		SetWindowText(hMessagesEdit, History.c_str());
		ReceiveData(receiveData, MAXSTRLEN);
		History.append(L"Recieved:\t");
		History.append((wchar_t*)receiveData);
		History.push_back('\n');
		SetWindowText(hMessagesEdit, History.c_str());
		if (strcmp(receiveData, "end") == 0) {
			return;
		}
		number = atoi(receiveData);

		char totPrice[MAXSTRLEN];
		_itoa_s(number * prices[code - 1], totPrice, 10);
		strcat_s(rez, MAXSTRLEN, totPrice);
		SendData(totPrice);
		History.append(L"\tWaiting...\n");
		SetWindowText(hMessagesEdit, History.c_str());
	}
}