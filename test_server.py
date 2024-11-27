import socket
import threading
import time
import random

# Variables globales para configurar el comportamiento del test
NUM_CLIENTS = 5  # Número de clientes a crear
NUM_MESSAGES_PER_USER = 20

# Dirección y puerto del servidor
SERVER_HOST = '127.0.0.1'
SERVER_PORT = 8080

# Función para crear un cliente y conectar al servidor
def client_thread(client_id):
    try:
        # Crear socket de cliente
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((SERVER_HOST, SERVER_PORT))
        
        # Crear nuevo cliente (puede ser prepago o postpago)
        # El cliente elige aleatoriamente entre prepago y postpago
        user_type = random.choice([0, 1])  # 0: prepago, 1: postpago
        
        # Iniciar sesión en el servidor
        buffer = client_socket.recv(1024).decode('utf-8')  # Recibir el mensaje para ingresar ID
        client_socket.send(str(-1).encode('utf-8'))  # Enviar -1 para crear usuario
        
        # Recibir y enviar el tipo de usuario
        buffer = client_socket.recv(1024).decode('utf-8')
        client_socket.send(str(user_type).encode('utf-8'))  # Enviar tipo de usuario
        
        # Aceptar el mensaje para comenzar
        buffer = client_socket.recv(1024).decode('utf-8')
        
        message_count = 0  # Contador de mensajes enviados por el cliente

        while message_count < NUM_MESSAGES_PER_USER:  # Limitar el número de mensajes enviados
            time.sleep(random.uniform(0.5, 1.5))  # Simular tiempo de espera entre mensajes
            
            if user_type == 0 and message_count >= 10:
                # Si es prepago y ya ha enviado 10 mensajes, intentar cambiar a postpago
                print(f"Cliente {client_id} (prepago) ha superado el límite de 10 mensajes.")
                client_socket.send("change 1".encode('utf-8'))  # Solicitar cambiar a postpago
                buffer = client_socket.recv(1024).decode('utf-8')  # Recibir confirmación de cambio
                user_type = 1  # El cliente ahora es postpago

            # Enviar mensaje al servidor
            message = f"Mensaje del cliente {client_id} tipo {user_type}, mensaje {message_count + 1}"
            client_socket.send(message.encode('utf-8'))
            print(f"Cliente {client_id} (tipo {user_type}) ha enviado: {message}")

            # Incrementar el contador de mensajes enviados
            message_count += 1

            # Recibir respuesta del servidor
            buffer = client_socket.recv(1024).decode('utf-8')
            print(f"Cliente {client_id} recibió: {buffer}")
        
        # Cerrar la conexión al final
        client_socket.close()
        print(f"Cliente {client_id} desconectado.")
    
    except Exception as e:
        print(f"Error en cliente {client_id}: {e}")

# Crear y ejecutar múltiples clientes
def start_test():
    threads = []
    for client_id in range(1, NUM_CLIENTS + 1):
        thread = threading.Thread(target=client_thread, args=(client_id,))
        thread.start()
        threads.append(thread)

    # Esperar a que todos los clientes terminen
    for thread in threads:
        thread.join()

if __name__ == "__main__":
    # Configuración global de prueba
    print(f"Iniciando la prueba con {NUM_CLIENTS} clientes y {NUM_MESSAGES_PER_USER} mensajes por usuario...")
    start_test()
    print("Prueba terminada.")
