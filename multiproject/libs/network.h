#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <sys/types.h>
#include <WinSock2.h>

namespace Network {

	inline int WSAAPI net_bind(
		_In_ SOCKET s,
		_In_reads_bytes_(namelen) const struct sockaddr FAR * name,
		_In_ int namelen
	) {
		return bind(s, name, namelen);
	}

	void send_data(SOCKET& socket, const char* data, size_t size);
	size_t recieve_data(SOCKET& socket, char* data, size_t admitted_size);

	void send_data(SOCKET& socket, const char* data, size_t size) {
		send(socket, (const char*)&size, sizeof(size), 0);
		size_t sent = 0;
		while (sent != size) {
			send(socket, data + sent, size - sent, 0);
			size_t recieved;
			recv(socket, (char*)&recieved, sizeof(recieved), 0);
			sent += recieved;
		}
	}

	size_t recieve_data(SOCKET& socket, char* data, size_t admitted_size) {
		size_t data_size;
		recv(socket, (char*)&data_size, sizeof(data_size), 0);
		admitted_size = min(data_size, admitted_size);
		size_t recieved = 0;
		while (recieved != admitted_size) {
			size_t tmp = recv(socket, data + recieved, admitted_size - recieved, 0);
			send(socket, (const char*)&tmp, sizeof(tmp), 0);
			recieved += tmp;
		}
		return data_size - recieved;
	}

	int load_WSA() {
		// Инициализируем процесс библиотеки wsock32.dll 
		WSADATA wsaData;
		int err = WSAStartup(0x0101, &wsaData);
		return err == SOCKET_ERROR;
	}

	int unload_WSA() {
		// Выгружаем библиотеку wsock32.dll
		return WSACleanup();
	}

}