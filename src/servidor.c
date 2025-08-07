#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4950

int main() {
	int sockfd, ret;
	struct sockaddr_in serverAddr;
	int newSocket;
	struct sockaddr_in newAddr;
	socklen_t addr_size;
	char buffer[1024];
	pid_t childpid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("[-] Erro na criação do socket.\n");
		exit(1);
	}
	printf("[+] Socket do servidor criado.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (ret < 0) {
		printf("[-] Erro na vinculação.\n");
		exit(1);
	}
	printf("[+] Vinculado à porta %d\n", PORT);

	if (listen(sockfd, 10) == 0) {
		printf("[+] Aguardando conexões...\n");
	} else {
		printf("[-] Erro ao escutar conexões.\n");
	}

	while (1) {
		addr_size = sizeof(newAddr);
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if (newSocket < 0) {
			exit(1);
		}
		printf("Conexão aceita de %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		if ((childpid = fork()) == 0) {
			close(sockfd);
			int atendimento = 0;
			char resposta[1024];

			while (1) {
				bzero(buffer, 1024);
				if (recv(newSocket, buffer, 1024, 0) <= 0) {
					printf("[-] Cliente desconectado.\n");
					break;
				}

				// Se o cliente quiser encerrar
				if (strcmp(buffer, ":exit") == 0 || strcmp(buffer, "4") == 0) {
					printf("Cliente encerrou o atendimento.\n");
					send(newSocket, "Conexão encerrada. Até logo!", 1024, 0);
					break;
				}

				// Primeira etapa: oferecer menu
				if (!atendimento) {
					char *menu = "Qual tipo de atendimento você deseja?\n1 - Comercial\n2 - Suporte\n3 - Financeiro\n4 - Encerrar atendimento?";
					send(newSocket, menu, strlen(menu), 0);
					atendimento = 1;
					continue;
				}

				// Escolha do tipo de atendimento
				if (strcmp(buffer, "1") == 0) {
					send(newSocket, "Redirecionando para o setor Comercial...", 1024, 0);
					continue;
				} else if (strcmp(buffer, "2") == 0) {
					send(newSocket, "Redirecionando para o setor de Suporte Técnico...", 1024, 0);
					continue;
				} else if (strcmp(buffer, "3") == 0) {
					send(newSocket, "Redirecionando para o setor Financeiro...", 1024, 0);
					continue;
				}

				// Conversa ativa: exibir mensagem do cliente
				printf("Cliente: %s\n", buffer);

				// Servidor digita a resposta
				printf("Servidor: ");
				bzero(resposta, 1024);
				fgets(resposta, 1024, stdin);
				resposta[strcspn(resposta, "\n")] = 0; // Remove newline

				send(newSocket, resposta, strlen(resposta), 0);
			}
			close(newSocket);
			exit(0);
		}
	}
	return 0;
}

