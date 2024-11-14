ChismeGPT - Sistema de Mensajería Concurrente




Para compilar y ejecutar este sistema, necesitas tener instalados los siguientes paquetes:

    GCC: El compilador de C.
    glib-2.0: Una biblioteca que se utiliza para manejar estructuras de datos como tablas hash y colas.




Puedes instalar las dependencias necesarias en una distribución de Ubuntu con el siguiente comando:

    sudo apt-get install build-essential libglib2.0-dev




Compilación del Proyecto:

El proyecto incluye dos programas: el servidor y el cliente. Puedes compilar ambos utilizando el siguiente comando:

    make

Esto generará dos ejecutables:

    server: El programa del servidor.
    client: El programa del cliente.




Ejecutando el Servidor:

Para ejecutar el servidor, usa el siguiente comando:

    ./server

El servidor escuchará en el puerto 8080 para conexiones entrantes de clientes.




Ejecutando el Cliente:

Para ejecutar un cliente, usa el siguiente comando en otro terminal:

    ./client

El cliente se conectará al servidor en la dirección 127.0.0.1 y el puerto 8080. El cliente te pedirá que ingreses un nombre de usuario y un tipo de cuenta (prepago o pospago). Después de eso, podrás enviar mensajes al servidor.




Interacción del Cliente:

        El cliente te pedirá un nombre de usuario. Si el usuario no existe, se creará una cuenta para ti.
        
        Luego, te pedirá que selecciones un tipo de usuario (pre-pago o pos-pago).
        
        Finalmente, podrás enviar mensajes. Los usuarios pre-pago tienen un límite de 10 mensajes, mientras que los pos-pago no tienen límite.
        
        Los mensajes de los usuarios pos-pago tienen prioridad sobre los mensajes de los usuarios pre-pago.




Salir del Cliente:

Para salir del cliente, simplemente escribe el comando: 
      
      exit




Comportamiento del Servidor:

    El servidor acepta conexiones de múltiples clientes simultáneamente mediante hilos.
    
    Cada mensaje es procesado en orden, pero los mensajes de los usuarios pos-pago se procesan con prioridad, incluso si hay mensajes de usuarios pre-pago en espera.
    
    Los usuarios pre-pago tienen un límite de 10 mensajes. Una vez que este límite es alcanzado, no podrán enviar más mensajes.
    
    El servidor muestra en todo momento el número de usuarios conectados.




Comandos Útiles:

    make: Compila el proyecto (el servidor y el cliente).
    make clean: Elimina los archivos objeto y los ejecutables generados.
    make install: Instala las dependencias necesarias.




Notas Importantes:

    Puerto: El servidor escucha en el puerto 8080. Asegúrate de que este puerto esté libre en tu máquina.
    
    Concurrencia: El sistema maneja múltiples conexiones simultáneas utilizando hilos. Esto permite que varios clientes interactúen con el servidor de manera concurrente.
    
    Base de Datos en Memoria: La base de datos de usuarios se almacena en memoria. Si el servidor se detiene, todos los datos se perderán.
