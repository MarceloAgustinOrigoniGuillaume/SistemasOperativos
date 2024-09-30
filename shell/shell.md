# shell

### Búsqueda en $PATH

> ¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

Los wrappers de la librería estandar C, utilizan todos la syscall. Simplemente configuran los parametros de la syscall.

> ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

`exec()` puede fallar, en ese caso retorna -1, y en la implementacion de la shell se cancela la ejecución, retornando error la ejecución del comando.

---

### Flujo estándar

> Investigar el significado de 2>&1, explicar cómo funciona su forma general

```ls -C /home /noexiste >err.txt 2>&1 >out.txt```
En el bash: La secuencia de lo que hace es `>err.txt` apunta el stdout (1) a err.txt. Después `2>&1` apunta stderr (2) a lo que apunta stdout (1), es decir err.txt. Y luego, el último `>out.txt` cambia a lo que apunta el stdout, a out.txt, pero el stderr se queda con el valor anterior. Tal que redirigiría stderr a err.txt y stdout a out.txt

En la implementacion de la shell: Lee y parsea primero `>err.txt` `>out.txt` , quedando el último como archivo para el stdout. Y luego hace la redirección copiando al stderr el stdout. Es decir ambos al out.txt.

---

### Tuberías múltiples

>  Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe
En la bash se muestra el exit code del último. En la implementación de shell no esta predefinido pero nuestra shell en teoría retorna el exit code del último proceso del pipe también.

En la bash el siguiente comando sale con exit code 2. Ya que el `ls falla al no existir.
```ls -C /noexiste```

Por otro lado, el siguiente comando sale con exit code 0. Hipotéticamente porque el`echo` es el último proceso del pipe.
```ls -C /noexiste | echo "nuevo proceso piped" ```

Y por último, este comando sale con exit code 2. Hipotéticamente porque el `ls` el último proceso del pipe.
```echo "nuevo proceso piped" | ls -C /noexiste```

---

### Variables de entorno temporarias

>  ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Al hacer `fork` se comparte el env del proceso. Copiando variables de entorno. Lo que no es deseado en la funcionalidad esperada. Que es que sea por proceso hijo del pipe.

> En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).
> ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
> Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

Para poder hacer tenga el mismo efecto se debería agregar a esa lista de argumentos/variables de entorno todas las variables del entorno anterior antes de hacer las modificaciones especificas.

---

### Pseudo-variables

>  Investigar al menos otras tres variables mágicas estándar, y describir su propósito.

En bash se tiene varias 'palabras mágicas' :

`$#` te retorna la cantidad de parámetros pasada al programa.
Se podría usar para saber si un parámetro fue definido. Aunque haya otras maneras.
```if [ $# -gt 1 ];then```
esto implicaría al menos hay un parametro.

`$@` te expande los parametros en una lista dentro de un .sh
Sirve no solo para mandar todos los parámetros a un programa interno. También permite remover parámetros iniciales.
```${@:2}```
esto tomaría todos menos el primer parámetro pasado.

`$!` retorna el pid del último proceso.
Esta es similar al $?, pero tendrá más sentido a la hora de ejecutar un proceso en background.

---

### Comandos built-in

>  ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

El único que puede ser implementado sin ser built-in es `pwd` ya que solo muestra la ruta actual sin cambiar el estado de la shell. En cambio, `cd` si se ejecutase en un proceso aparte solo cambiaría el directorio de ese proceso hijo.
Se hace como built-in principalmente por temas de eficiencia, es mucho más rápido que tener que iniciar un nuevo proceso y cargarlo en memoria, similar a `true` y `false`, comandos built-in que solo devuelven 0 y 1, sería muy poco eficiente crear un nuevo proceso solo para eso.

---

### Procesos en segundo plano

> ¿Por qué es necesario el uso de señales?

El uso de señales nos permite reaccionar de manera automática a los eventos del sistema, como la señal `sigchild` que se emite cada vez que un proceso hijo termina, permitiendo configurar un handler que libera todos sus recursos, evitando la creacion de procesos zombie. Pero de forma no bloqueante. Similar pasaria con otras señales, con el `SIGINT` para el control c. Te permitiria liberar recursos. O incluso evitar la terminacion. Aunque hay señales que no pueden ser sobreescritas como el process `KILL`.

---
