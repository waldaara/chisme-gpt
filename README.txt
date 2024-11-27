INSTRUCCIONES DE USO



Librerías necesarias:



Este proyecto requiere las siguientes bibliotecas en tu sistema:

-glib-2.0 (ver instrucciones de compilacion)
-pthread
-arpa/inet.h y netinet/in.h para manejo de sockets



Compilación y ejecución:



Para compilar el proyecto, se utiliza el archivo Makefile. Puedes usar los siguientes comandos:


- Compilar el proyecto:
    
    $ make

Este comando compilará el servidor y el cliente y generará los ejecutables.


- Limpiar archivos compilados:

    $ make clean


-Instalar librerías (solo en Debian based):

    $ make install

    Si se desea manualmente:

    $ sudo apt-get install libglib2.0-dev




Ejecutar el servidor:




Una vez que el proyecto esté compilado, puedes ejecutar el servidor con el siguiente comando:

    $ ./server -n <max_concurrent_messages> -t <time_in_ms>

Donde:

-n <max_concurrent_messages>: Especifica el número máximo de mensajes que pueden ser procesados de manera concurrente. Por ejemplo, -n 5 permitirá que 5 hilos trabajen simultáneamente.
-t <time_in_ms>: Especifica el tiempo que tarda el servidor en procesar cada mensaje (en milisegundos). Por ejemplo, -t 100 hará que el servidor tarde 100 ms en procesar cada mensaje.

Ejemplo de ejecución:

    $ ./server -n 5 -t 2000




Interacción con el servidor:





Una vez que el servidor está en funcionamiento, los clientes pueden conectarse al servidor a través de un cliente:

    $ ./client

Los clientes pueden realizar las siguientes acciones:

- Ingresar su ID de usuario: Si el cliente ya tiene un ID, puede ingresarlo. Si no tiene uno, se le solicitará crear uno nuevo.
- Enviar mensajes: Los usuarios prepagados tienen un límite de 10 mensajes, mientras que los usuarios postpago pueden enviar mensajes sin restricciones.
- Cambiar su tipo de usuario: Los usuarios pueden cambiar su tipo de plan (de prepagado a postpago o viceversa) enviando el comando change <user_type>, donde <user_type> es 0 para prepagado o 1 para postpago. El servidor procesará el cambio si es válido.

Comandos disponibles:

change 0: Cambia a prepagado.
change 1: Cambia a postpago.
exit: Cierra la conexión con el servidor.


Ejemplo de conversación con el servidor:


    Enter your ID (enter -1 if you don't have one): 1
    Enter your message (ID: 1, User type: prepaid, Messages left: 10): Hello
    Message 1 processed (prepaid) in 100 ms


En caso de que un usuario prepagado intente enviar más de 10 mensajes, el servidor le notificará que ha excedido el límite.



TEST AUTOMATIZADO:


Para su comodidad, hice un test en python (test_server.py) para simular 5 usuarios concurrentes que envian 20 messages cada uno

 - Primero levante el server, por ejemplo:

    $ ./server -n 5 -t 2000

 - Luego:

    $ python3 test_server.py

Puede modificar los parámetros del server y del test de python (checar las variables globales del script) para que pruebe todo el funcionamiento :)
