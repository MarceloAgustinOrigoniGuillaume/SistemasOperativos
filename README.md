## Estructuras

Para una mejor modularizacion y separacion entre componentes. Se dividio en tres partes el filesystem. Un inodes.h, para el manejo y alocacion de inodos, que contienen la metadata ya sea un directorio, un archivo regular, o si fuere un soft link. El cual no existe en este tp.

Y luego se tiene:
- un manager de directorios, el directories.h. Que se encarga de tener el ownership y administrar la informacion de directorios. Este manager tiene una dependencia tambien en el de inodos. Ya que al buscar hijos de un directorio debe obtener el inodo. Esto se hace mediante un id del inodo. Que al ser un arreglo estatico, el indexado es rapido.

- un manager de bloques, el blocks.h. Se encarga de administrar informacion raw de bytes en forma de bloques. En especial para archivos. Ya que no habia porque usarlo para directorios. Este modulo perse no tiene dependencia en el de inodos. Solo interactua con el Inodo en cuestion.

- Un modulo "integrador", el filesystem.h , se encarga de integrar los 3 modulos para formar las funcionalidades de las fuse operations.


El Inodo en si no tiene una dependencia directa en los datos del mismo. Sino que guarda un "first_block" que representa un id del manager de directorios u bloques.


La secuencia general para el filesystem seria:
- Buscas un inodo por medio del directories.h, el manager de directorios.
- Aplicas una accion. En inodes.h, removerlo, crear uno, obtener las stats. En esto cabe remarcar que el directories para uno nuevo devuelve el nombre del inodo a crear y el padre. No lo crea, para evitar dependencias de mas. En contraste para la remocion de un directorio, al ser recursivo, si se tiene una dependencia en el deleteInode. Al eliminar los hijos primero. Se podria hacer como normalmente se hace en filesystems que no se deja eliminar si el directorio no esta vacio.


Tras crear el inodo en si, se delega la alocacion de datos a directories.h o blocks.h, quienes al alocar internamente la data. Le setean al inodo el "first_block" que representa el id de la data.

En caso del readchildren o read/write que acceden a la data del inodo. Se sabe por medio del tipo, a quien delegar, fuera de que ya por el metodo se sabe a quien delegar.

Para manejar la alocacion de nuevos elementos y la remocion. En los 3 modulos que manejan cosas en si. Se tiene una lista enlaza por encima de los arreglos estaticos para manejar quien esta free. 

En cuanto a limitaciones se plantearon que las cosas por ahora son estaticas. El size maximo de un archivo es un bloque de 512 bytes, la cantidad maxima de hijos es 128 para un directorio. Y la cantidad de directorios, bloques y/o inodos, tambien es limitada a la capacidad inicial.


# Serializacion
Lo bueno de no tener dependencias entre modulos y trabajar con ids. Es que se puede serializar/deserializar cada modulo independientemente.

La serializacion consta simplemente de guardar los datos internos.
Y en los 3 modulos se tiene una logica analoga para tener en cuenta los huecos en los arreglos estaticos. 

Primero se serializa la cantidad de elementos alocados.

Luego se serializa esa cantidad de elementos, ignorando un elemento si esta pertenece a la lista free. Importante para le deserializacion, en vez de tener un estado o booleano para denotar que uno lo este. O una lista enlazada. 

El serializado de cada elemento es guardar la data interna del mismo.
En el inodo esto es el id, las estadisticas y el "first_block".
En el directorio el id, size, capacidad y los ids hijos.
En el block el id,size y los bytes en si.

Ademas en el caso de los inodos. Se decidio aserverar el inodo root siempre este al inicio en el serializado. Asi que se manda/serializa antes de mandar la cantidad de inodos y cada inodo. Y en la cantidad no se lo tiene en cuenta.


Para el serializado se creo un modulo de serializado. serial.h que encapsula la logica de little endian vs big endian. Entre otras funcionalidades, que no se usan al final.


# Testing
Se creo aparte de los tests de integracion el main_debug.c que se compila como dbg_serial, que se encarga de testear el serializado de cosas internamente. Tiene una estructura similar a la de librerias de testeado, usando macros para simplificar.


Los tests de integracion se hicieron con python, usando de base los del tp de shell provistos por la catedra. 

Se los dividio en "steps" que son definidos por el yaml. Teniendo un step base que ejecuta el comando y compara el out y ret code de ser necesario.

Y casos especificos para Ls y Stat, que parsean el output para la bash de ubuntu. Ademas se agrego el step "serialize" interno, que desmonta y vuelve a montar el filesystem para probar el serializado.

Se agrego tiempos entre steps y montados. Para tener mejor consistencias ya que suele haber tests que fallan erraticamente.
