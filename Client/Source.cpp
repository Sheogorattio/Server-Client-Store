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

WSADATA wsaData;//структура для хранения информации о инициализации сокетов
SOCKET _socket; //дескриптор слушающего сокета
sockaddr_in addr; //локальный адрес и порт
string ipAddress; //Адрес сервера
int port = 24242; //выбираем порт
wstring History;

HWND hMessagesEdit, hIPEdit, hSendEdit, hSendButton, hConnectButton, hCloseButton;

void CloseConnection();
void ConnectToServer(const char* ipAddress, int port);
bool SendData(char* buffer);
bool ReceiveData(char* buffer, int size);

BOOL DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
		case WM_INITDIALOG:
		{
			hMessagesEdit = GetDlgItem(hwnd, IDC_EDIT2);
			hSendEdit = GetDlgItem(hwnd, IDC_EDIT3);
			hIPEdit = GetDlgItem(hwnd, IDC_IPADDRESS1);
			hSendButton = GetDlgItem(hwnd, IDCANCEL2);
			hConnectButton = GetDlgItem(hwnd, IDC_BUTTON1);
			hCloseButton = GetDlgItem(hwnd, IDCANCEL);
			return TRUE;
		}
		case WM_COMMAND:
		{
			if (wParam == IDC_BUTTON1)//connect
			{
				TCHAR IP[20];
				GetWindowText(hIPEdit, IP, 20);
				ConnectToServer((char*)IP, port);
				SetWindowText(hMessagesEdit, L" 1) phone 14000\n2)tablet 18000\n3)laptop 28000\nEnter code(1 / 2 / 3)\n");
				History.append(L"1) phone 14000\n2)tablet 18000\n3)laptop 28000\nEnter code(1/2/3)\n");
			}
			if (wParam == IDCANCEL2)//send
			{
				TCHAR buff[MAXSTRLEN];
				GetWindowText(hSendEdit, buff, MAXSTRLEN);
				SendData((char*)buff);
				History.push_back('\n');
				History.append(buff);
				History.append(L"\nWaiting...\n");
				SetWindowText(hMessagesEdit, History.c_str());
				ReceiveData((char*)buff, MAXSTRLEN);
				History.append(L"Received:\t");
				History.append(buff);
				History.push_back('\n');
				SetWindowText(hwnd, History.c_str());
				if (wcscmp(buff, L"end") == 0) {
					CloseConnection();
				}
			}
			if (wParam == IDCANCEL)
			{
				CloseConnection();
			}
			break;
		}
		case WM_CLOSE:
		{
			CloseConnection();
			EndDialog(hwnd, 0);
		}
	}
	return FALSE;
}


BOOL WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdLine, int nCmdShow) {
	DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc);
}


void ConnectToServer(const char* ipAddress, int port)
{
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, ipAddress, &addr.sin_addr);
	addr.sin_port = htons(port);
	//при неудачном подключении к серверу выводим сообщение про ошибку
	if (connect(_socket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		SetWindowText(hMessagesEdit, L"Failed to connect to server\n");
		WSACleanup();
		exit(13);
	}
}

bool SendData(char* buffer)
{
	/*Отправляем сообщение на указанный сокет*/
	send(_socket, buffer, strlen(buffer), 0);
	return true;
}

bool ReceiveData(char* buffer, int size)
{

	/*Получаем сообщение и сохраняем его в буфере.
	Метод является блокирующим!*/
	int i = recv(_socket, buffer, size, 0);
	buffer[i] = '\0';
	return true;
}

void CloseConnection()
{
	//Закрываем сокет
	closesocket(_socket);
}