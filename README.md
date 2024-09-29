# Responder: ¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

Los wrappers de la libreraria estandar C, utilizan todos la syscall. Simplemente configuran los parametros de la syscall


# Responder: ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?
Exec puede fallar, en ese caso retorna -1, la implementacion de la shell cancela la ejecucion, retornando error la ejecucion del comando.

# Responder: Investigar el significado de 2>&1, explicar cómo funciona su forma general
Mostrar qué sucede con la salida de cat out.txt en el ejemplo.
Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt). ¿Cambió algo? Compararlo con el comportamiento en bash(1).


```ls -C /home /noexiste > err.txt 2>&1 > out.txt```
En el bash:
La secuencia de lo que hace es "> err.txt" apunta el stdout (1) a err.txt
Despues "2>&1" apunta 2, stderr, a lo que apunta 1, stdout. Es decir err.txt
Y despues el ultimo "> out.txt" cambia a lo que apunta el stdout, a out.txt, pero el 2, stderr, se queda con el valor anterior.
Tal que redirigiria stderr a err.txt y stdout a out.txt

En la implementacion de la shell:
Lee y parsea primero "> err.txt"  "> out.txt" , quedando el ultimo como archivo para el stdout.
Y luego hace la redireccion copiando al stderr el stdout. Es decir ambos al out.txt.



# Responder: Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe

    ¿Cambia en algo?
    ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.


# Responder: ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?
# Responder: En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).

    ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
    Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.


# Responder: Investigar al menos otras tres variables mágicas estándar, y describir su propósito.

    Incluir un ejemplo de su uso en bash (u otra terminal similar).

# Responder: ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

El único que puede ser implementado sin ser built-in es pwd ya que solo muestra la ruta actual, sin cambiar el estado de la shell, no como cd que si se ejecutase en un proceso aparte, solo cambiaría el directorio de ese proceso hijo.
Se hace como built-in principalmente por temas de eficiencia, es mucho más rápido que tener que iniciar un nuevo proceso y cargarlo en memoria, similar a true y false, comandos built-in que solo devuelven 0 y 1, sería muy poco eficiente crear un nuevo proceso solo para eso.

En bash se tiene varias 'palabras magicas' 
$# = te retorna la cantidad de parametros pasada al programa.
$@ = te expande los parametros en una lista dentro de un .sh

El $# se podria usar para saber si un parametro fue definido. Aunque haya otras maneras.
```if [ $# -gt 1 ];then``` implicaria almenos hay un parametro.

$@ sirve no solo para mandar todos los parametros a un programa interno. Tambien permite remover parametros iniciales.
```${@:2} ``` tomaria todos menos el primer parametro pasado.

$! = retorna el pid del ultimo proceso

Esta es similar al $?, pero tendra mas sentido a la hora de ejecutar un proceso background.


# Responder: ¿Por qué es necesario el uso de señales?

El uso de señales nos permite reaccionar de manera automática a los eventos del sistema, como la señal sigchild que se emite cada vez que un proceso hijo termina, permitiendo configurar un handler que libera todos sus recursos, evitando la creacion de procesos zombie. Pero de forma no bloqueante. Similar pasaria con otras señales, con el SIGINT para el control c. Te permitiria liberar recursos. O incluso evitar la terminacion. Aunque hay señales que no pueden ser sobreescritas como el process KILL.






 
