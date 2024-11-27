#include <arpa/inet.h>
#include <bits/getopt_core.h>
#include <glib.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_BUFFER 1024
#define PREPAID_LIMIT 10

// Global variables
int n = 0;  // Max number of concurrent messages (threads)
int t = 0;  // Time for message processing in ms

static int message_counter = 0;  // Global counter for unique message IDs

// Message Queues for prepaid and postpaid messages
GQueue *prepaid_queue;
GQueue *postpaid_queue;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

// Users DB
GHashTable *users_table;
pthread_mutex_t table_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
  int user_type;      // 0 prepaid and 1 postpaid
  int message_count;  // -1 when user type is postpaid
} User;

// Function to simulate message processing (sleep for `t` ms)
void process_message(int message_id, const char *message_type) {
  usleep(t * 1000);  // Simulate message processing
  printf("Message %d processed (%s) in %d ms\n", message_id, message_type, t);
}

// Add prepaid message to the queue
void add_prepaid_message(int message_id) {
  pthread_mutex_lock(&queue_mutex);
  g_queue_push_tail(prepaid_queue, GINT_TO_POINTER(message_id));
  pthread_cond_broadcast(&queue_cond);  // Notify all waiting threads
  pthread_mutex_unlock(&queue_mutex);
}

// Add postpaid message to the queue
void add_postpaid_message(int message_id) {
  pthread_mutex_lock(&queue_mutex);
  g_queue_push_tail(postpaid_queue, GINT_TO_POINTER(message_id));
  pthread_cond_broadcast(&queue_cond);  // Notify all waiting threads
  pthread_mutex_unlock(&queue_mutex);
}

// Worker thread function to process messages
void *worker_thread(void *arg) {
  while (1) {
    int message_id = 0;
    char message_type[10];

    pthread_mutex_lock(&queue_mutex);

    // Prioritize postpaid messages
    while (g_queue_peek_head(postpaid_queue) != NULL) {
      message_id = GPOINTER_TO_INT(g_queue_pop_head(postpaid_queue));
      strcpy(message_type, "postpaid");
      pthread_mutex_unlock(&queue_mutex);
      process_message(message_id, message_type);
      pthread_mutex_lock(&queue_mutex);
    }

    // Process prepaid messages if no postpaid messages are available
    if (g_queue_peek_head(prepaid_queue) != NULL) {
      message_id = GPOINTER_TO_INT(g_queue_pop_head(prepaid_queue));
      strcpy(message_type, "prepaid");
      pthread_mutex_unlock(&queue_mutex);
      process_message(message_id, message_type);
      pthread_mutex_lock(&queue_mutex);
    } else {
      // No messages to process, wait for new messages
      pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    pthread_mutex_unlock(&queue_mutex);
  }
  return NULL;
}

// Function to send a message to the client and receive a response
int send_message_to_client(int client_socket, const char *message,
                           char buffer[MAX_BUFFER]) {
  send(client_socket, message, strlen(message), 0);

  int read_size = recv(client_socket, buffer, MAX_BUFFER, 0);
  if (read_size <= 0) {
    printf("Client %d disconnected\n", client_socket);
    close(client_socket);
    return -1;
  }

  buffer[read_size] = '\0';

  return 0;
}

// Function to create a new user
int create_user(int client_socket, char buffer[MAX_BUFFER], int *user_id) {
  if (send_message_to_client(client_socket,
                             "Enter your user type (0 prepaid - 1 postpaid)",
                             buffer) == -1) {
    return -1;
  }

  int user_type = atoi(buffer);
  User *new_user = malloc(sizeof(User));
  new_user->user_type = user_type;
  new_user->message_count = (user_type == 0) ? 0 : -1;

  int *user_id_ptr = malloc(sizeof(int));

  pthread_mutex_lock(&table_mutex);

  *user_id_ptr = g_hash_table_size(users_table) + 1;
  g_hash_table_insert(users_table, user_id_ptr, new_user);

  pthread_mutex_unlock(&table_mutex);

  printf("Client %d successfully created\n", *user_id_ptr);
  *user_id = *user_id_ptr;  // Update caller's user_id
  return 0;
}

// Function to handle client connection
void handle_client(int client_socket) {
  char buffer[MAX_BUFFER];

  if (send_message_to_client(client_socket,
                             "Enter your ID (enter -1 if you don't have one)",
                             buffer) == -1) {
    close(client_socket);
    return;
  }

  int user_id = atoi(buffer);
  if (user_id <= -1) {
    if (create_user(client_socket, buffer, &user_id) == -1) {
      close(client_socket);
      return;
    }
  }

  pthread_mutex_lock(&table_mutex);
  User *user = g_hash_table_lookup(users_table, &user_id);
  pthread_mutex_unlock(&table_mutex);

  while (user == NULL) {
    if (send_message_to_client(
            client_socket,
            "Invalid ID, enter again a valid one or -1 to create one",
            buffer) == -1) {
      close(client_socket);
      return;
    }

    user_id = atoi(buffer);
    if (user_id <= -1) {
      if (create_user(client_socket, buffer, &user_id) == -1) {
        close(client_socket);
        return;
      }
    }

    pthread_mutex_lock(&table_mutex);
    user = g_hash_table_lookup(users_table, &user_id);
    pthread_mutex_unlock(&table_mutex);
  }

  while (1) {
    while (!user->user_type && user->message_count >= 10) {
      send(client_socket, "Limit for prepaid user exceeded", 32, 0);

      int read_size = recv(client_socket, buffer, MAX_BUFFER, 0);
      if (read_size <= 0) {
        printf("Client %d disconnected\n", client_socket);
        close(client_socket);
        return;
      }

      buffer[read_size] = '\0';

      int new_user_type = -1;

      // Check if the string matches the form "change 0" or "change 1"
      if (sscanf(buffer, "change %d", &new_user_type) == 1 &&
          (new_user_type == 0 || new_user_type == 1)) {
        pthread_mutex_lock(&table_mutex);

        if (user->user_type != new_user_type) {
          user->user_type = new_user_type;
          user->message_count = new_user_type ? -1 : 0;

          printf("User updated: user_type = %d\n", user->user_type);

          pthread_mutex_unlock(&table_mutex);
          break;
        }

        pthread_mutex_unlock(&table_mutex);
      }
    }

    char message[MAX_BUFFER];

    if (user->user_type) {
      snprintf(message, sizeof(message),
               "Enter your message (ID: %d, User type: postpaid)", user_id);
    } else {
      snprintf(
          message, sizeof(message),
          "Enter your message (ID: %d, User type: prepaid, Messages left: %d)",
          user_id, PREPAID_LIMIT - user->message_count);
    }

    send(client_socket, message, strlen(message), 0);

    int read_size = recv(client_socket, buffer, MAX_BUFFER, 0);
    if (read_size <= 0) {
      printf("Client %d disconnected\n", client_socket);
      close(client_socket);
      return;
    }

    buffer[read_size] = '\0';

    int new_user_type = -1;

    // Check if the string matches the form "change 0" or "change 1"
    if (sscanf(buffer, "change %d", &new_user_type) == 1 &&
        (new_user_type == 0 || new_user_type == 1)) {
      pthread_mutex_lock(&table_mutex);

      if (user->user_type != new_user_type) {
        user->user_type = new_user_type;
        user->message_count = new_user_type ? -1 : 0;

        printf("User updated: user_type = %d\n", user->user_type);
      }

      pthread_mutex_unlock(&table_mutex);

      continue;
    }

    int message_id = __atomic_add_fetch(
        &message_counter, 1,
        __ATOMIC_SEQ_CST);  // Atomic increment for thread safety

    if (user->user_type) {
      printf("New message (postpaid): %s\n", buffer);

      add_postpaid_message(message_id);
    } else {
      printf("New message (prepaid): %s\n", buffer);

      pthread_mutex_lock(&table_mutex);
      user->message_count += 1;
      pthread_mutex_unlock(&table_mutex);

      add_prepaid_message(message_id);
    }
  }
}

// Function to start the server and accept connections
void start_server(int port) {
  int server_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_len = sizeof(client_addr);

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    perror("Error opening socket");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    perror("Error binding socket");
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  if (listen(server_socket, 5) < 0) {
    perror("Error listening on socket");
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d...\n", port);

  while (1) {
    client_socket =
        accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    if (client_socket < 0) {
      perror("Error accepting client connection");
      continue;
    }

    printf("New client connected: %d\n", client_socket);
    pthread_t thread;
    if (pthread_create(&thread, NULL, (void *(*)(void *))handle_client,
                       (void *)(intptr_t)client_socket) != 0) {
      perror("Error creating thread");
      close(client_socket);
    }
  }

  close(server_socket);
}

// Main function
int main(int argc, char *argv[]) {
  int port = 8080;
  int opt;

  while ((opt = getopt(argc, argv, "n:t:p:")) != -1) {
    switch (opt) {
      case 'n':
        n = atoi(optarg);
        if (n <= 0) {
          fprintf(stderr, "Error: 'n' must be a positive integer.\n");
          return EXIT_FAILURE;
        }
        break;
      case 't':
        t = atoi(optarg);
        if (t <= 0) {
          fprintf(stderr, "Error: 't' must be a positive integer.\n");
          return EXIT_FAILURE;
        }
        break;
      case 'p':
        port = atoi(optarg);
        if (port <= 0) {
          fprintf(stderr, "Error: 'p' must be a positive integer.\n");
          return EXIT_FAILURE;
        }
        break;
      default:
        fprintf(stderr,
                "Usage: %s -n <max_concurrent_messages> -t <time_in_ms> -p "
                "<port>\n",
                argv[0]);
        return EXIT_FAILURE;
    }
  }

  if (n <= 0 || t <= 0) {
    fprintf(stderr, "Error: Both n and t must be positive integers.\n");
    return EXIT_FAILURE;
  }

  users_table = g_hash_table_new_full(g_int_hash, g_int_equal, free, free);
  prepaid_queue = g_queue_new();
  postpaid_queue = g_queue_new();

  printf(
      "Processing %d messages concurrently, each taking %d ms to process...\n",
      n, t);

  // Create a thread pool of n threads to process messages concurrently
  pthread_t message_threads[n];

  for (int i = 0; i < n; i++) {
    if (pthread_create(&message_threads[i], NULL, worker_thread, NULL) != 0) {
      perror("pthread_create");

      return EXIT_FAILURE;
    }
  }

  // Start the server
  start_server(port);

  // Clean up the queues and threads
  for (int i = 0; i < n; i++) {
    pthread_join(message_threads[i], NULL);
  }

  g_hash_table_destroy(users_table);
  g_queue_free(prepaid_queue);
  g_queue_free(postpaid_queue);

  return EXIT_SUCCESS;
}
