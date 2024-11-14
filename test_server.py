import asyncio
import socket
import time

# Configuración del servidor y pruebas
SERVER_HOST = '127.0.0.1'
SERVER_PORT = 8080
NUM_CLIENTS = 5  # Número total de clientes a simular
NUM_MESSAGES = 5  # Número de mensajes por cliente
TIME_BETWEEN_MESSAGES = 1  # Tiempo en segundos entre mensajes

# Función asincrónica para manejar la conexión de un cliente
async def manejar_cliente(cliente_id, tipo_usuario):
    # Crear un socket de cliente
    cliente_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    cliente_socket.connect((SERVER_HOST, SERVER_PORT))

    # Enviar el ID
    print(f"Cliente {cliente_id} ({tipo_usuario}) conectado.")
    cliente_socket.sendall(f"{cliente_id}".encode())

    # Recibir la respuesta del servidor para el tipo de usuario
    respuesta = cliente_socket.recv(1024).decode()
    print(f"[{cliente_id}] - {respuesta}")

    # Enviar el tipo de usuario (prepago o pospago)
    cliente_socket.sendall(f"{tipo_usuario}".encode())

    # Recibir la confirmación de registro
    respuesta = cliente_socket.recv(1024).decode()
    print(f"[{cliente_id}] - {respuesta}")

    # Enviar mensajes al servidor
    for i in range(NUM_MESSAGES):
        mensaje = f"Mensaje {i+1} de {cliente_id} ({tipo_usuario})"
        print(f"[{cliente_id}] Enviando: {mensaje}")
        cliente_socket.sendall(mensaje.encode())
        await asyncio.sleep(TIME_BETWEEN_MESSAGES)  # Esperar el tiempo definido antes de enviar el siguiente mensaje

    # Cerrar la conexión
    cliente_socket.close()

# Función para probar el servidor con múltiples clientes de forma asincrónica
async def test_servidor():
    # Crear una lista de tareas de clientes
    tareas = []
    for i in range(NUM_CLIENTS):
        tipo = 'prepago' if i % 2 == 0 else 'pospago'  # Alternar entre prepago y pospago
        cliente_id = f"cliente_{i+1}"
        print(f"--- Iniciando cliente {cliente_id} ({tipo}) ---")
        tarea = asyncio.create_task(manejar_cliente(cliente_id, tipo))
        tareas.append(tarea)
    
    # Esperar a que todas las tareas se completen
    await asyncio.gather(*tareas)

if __name__ == "__main__":
    print(f"Test de servidor comenzando... (Clientes: {NUM_CLIENTS}, Mensajes por cliente: {NUM_MESSAGES}, Tiempo entre mensajes: {TIME_BETWEEN_MESSAGES}s)")
    asyncio.run(test_servidor())
    print("Prueba completada.")

