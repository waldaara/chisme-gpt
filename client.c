#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER 1024

void enviar_mensaje(int socket, const char *mensaje);
void leer_respuesta(int socket, char *respuesta, size_t len);

int main() {
  int socket_cliente;
  struct sockaddr_in servidor_addr;
  char mensaje[MAX_BUFFER];
  char respuesta[MAX_BUFFER];

  // Crear socket
  socket_cliente = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_cliente == -1) {
    perror("Error al crear el socket");
    return EXIT_FAILURE;
  }

  // Configurar la dirección del servidor
  memset(&servidor_addr, 0, sizeof(servidor_addr));
  servidor_addr.sin_family = AF_INET;
  servidor_addr.sin_port = htons(8080);
  servidor_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Conectar al servidor
  if (connect(socket_cliente, (struct sockaddr *)&servidor_addr,
              sizeof(servidor_addr)) == -1) {
    perror("Error al conectar con el servidor");
    return EXIT_FAILURE;
  }

  printf("Connected to server\n\n");

  while (1) {
    leer_respuesta(socket_cliente, respuesta, sizeof(respuesta));
    printf("\n(server): %s\n", respuesta);

    printf("(client): ");
    fgets(mensaje, sizeof(mensaje), stdin);
    mensaje[strcspn(mensaje, "\n")] = 0;  // Eliminar salto de línea

    // Check for empty input
    while (strlen(mensaje) == 0) {
      printf("\nMessage cannot be empty. Please try again.\n\n");

      printf("(client): ");
      fgets(mensaje, sizeof(mensaje), stdin);
      mensaje[strcspn(mensaje, "\n")] = 0;  // Eliminar salto de línea
    }

    if (strcasecmp(mensaje, "EXIT") == 0) {
      break;  // Salir del bucle
    }

    enviar_mensaje(socket_cliente, mensaje);
  }

  close(socket_cliente);
  printf("\nConnection closed\n");
  return EXIT_SUCCESS;
}

void enviar_mensaje(int socket, const char *mensaje) {
  if (write(socket, mensaje, strlen(mensaje)) == -1) {
    perror("Error al enviar mensaje");
    close(socket);
    exit(EXIT_FAILURE);
  }
}

void leer_respuesta(int socket, char *respuesta, size_t len) {
  ssize_t bytes_leidos = read(socket, respuesta, len - 1);
  if (bytes_leidos == -1) {
    perror("Error al leer respuesta del servidor");
    close(socket);
    exit(EXIT_FAILURE);
  }

  respuesta[bytes_leidos] =
      '\0';  // Asegurarse de que la respuesta está bien terminada
}
