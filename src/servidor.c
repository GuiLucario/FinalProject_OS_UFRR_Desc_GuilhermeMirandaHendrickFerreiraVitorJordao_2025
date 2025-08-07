#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5050

int main(){
	int sockfd, ret;
	struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;
	socklen_t addr_size;

	char buffer[1024];
	pid_t childpid;

	// Criação do socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-] Erro na criação do socket.\n");
		exit(1);
	}
	printf("[+] Socket do servidor criado!\n");

	// Permitir reuso da porta
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// Configuração do endereço
	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// Associação do socket com o endereço local
	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-] Erro na vinculação. \n");
		exit(1);
	}
	printf("[+] Vinculado à porta %d\n", PORT);

	if(listen(sockfd, 10) == 0){
		printf("[+] Aguardando conexão...\n");
	}else{
		printf("[-] Erro ao escutar.\n");
	}

	while(1){
		addr_size = sizeof(newAddr);
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}
		printf("Conexão aceita com %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		if((childpid = fork()) == 0){
			close(sockfd);

			while(1){
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, sizeof(buffer), 0);
				if(strcmp(buffer, ":exit") == 0){
					printf("Desconectado de %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					break;
				}else{
					printf("Cliente: %s\n", buffer);
					send(newSocket, buffer, strlen(buffer), 0);
				}
			}
			close(newSocket);
			exit(0);
		}
	}

	return 0;
}

