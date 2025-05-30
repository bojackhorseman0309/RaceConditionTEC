# RaceConditionTEC

Tarea de condiciones de carrera

# Requisitos

- Ubuntu (24.10) en ARM
- vMWare Fusion o similar

# Pasos iniciales

1. Pasar archivos vulp a la maquina virtual

```
# Servidor de archivos en macOS de atacante
python3 -m http.server 80

# Traer los archivos desde la maquina virtual
curl http://IP/vulp.c -o vulp.c
curl http://IP/vulp_repetition.c -o vulp_repetition.c
curl http://IP/vulp_seteuid.c -o vulp_seteuid.c
curl http://IP/looper.sh -o looper.sh
```

2. Eliminar protecciones de seguridad de symlinks para efectos de la tarea

```
# Esto parece restablecerse siempre por lo que es mejor ejecutarlo cada vez que se inicia la VM
sudo sysctl -w fs.protected_symlinks=0
sudo sysctl fs.protected_regular=0
```

3. Compilar el programa `vulp.c`, `vulp_repetition.c`, `vulp_seteuid.c`, `firegoblin.c` y asignar permisos de `setuid`
   de `root`

```
# Mismo proceso para todos los vulp*.c
gcc vulp.c -o vulp -lm
sudo chown root vulp
sudo chmod 4755 vulp

gcc firegoblin.c -o firegoblin
```

# Trabajo 1

Agregar entrada a `/etc/shadow` y `/etc/password` para un nuevo usuario `root` mediante un ataque al programa `vulp`.

1. Agregar nuevo usuario como prueba de como se agrega en `/etc/shadow` y `/etc/password`

```
# useradd pwned -m -G sudo
# passwd pwned

# cat /etc/shadow
pwned:$y$j9T$yb7F48eurkR2ZugVrkr4U/$TkSO.sGjtjyki/IlF018wo6kw7zd4nUUrhE9emRNb7C:20237:0:99999:7:::

# cat /etc/passwd
pwned:x:1001:1001::/home/pwned:/bin/sh
```

Es necesario para el ataque que este se asemeje a `root` y tenga los valores de 0 en `/etc/passwd`

Por lo que debería verse asi:

```
# cat /etc/passwd
pwned:$y$j9T$yb7F48eurkR2ZugVrkr4U/$TkSO.sGjtjyki/IlF018wo6kw7zd4nUUrhE9emRNb7C:0:0:pwned:/root:/bin/bash
```

No es necesario editar `/etc/shadow`, ya que es posible realizar una entrada en `/etc/passwd`
por medio de utilizar un hash como contraseña, una manera de hacerlo es creando un usuario
y copiándolo el valor por medio de `/etc/shadow`.
Porque se tiene la contraseña en lugar de "x" es posible no tener que modificar ambos, inclusive se puede
dejar vació y se puede acceder al usuario creado sin contraseña.

2. Ejecutar el ataque

Es necesario ejecutar el ataque por medio de un loop, ya que al ser un ataque de condición de carrera,
tiene que correrse en paralelo multiples veces hasta encontrar el éxito, esto es realizado
por medio de ejecutar un loop tanto en `firegoblin` como en `looper.sh`.

```
$ ./firegoblin

// En otra terminal
$ chmod +x looper.sh
$ ./looper.sh 
```

A cierto momento se debería recibir el mensaje de éxito en la terminal de looper
y se debería poder acceder al usuario creado y verificar que es root.

```
$ ./looper.sh 
PWNED :). El archivo /etc/passwd tiene que haber sido modificado.
$ su pwned
# whoami
root
# id
uid=0(root) gid=0(root) groups=0(root)
# cat /etc/passwd
pwned::0:0:pwned:/root:/bin/bash
```

## Explicación del ataque

El ataque se aprovecha de una condición de carrera en el archivo `vulp.c`, el cual verifica
que el archivo seleccionado dentro del código se tenga acceso por medio de la llamada `access`
y luego lo abre por medio de `fopen`.
Sin embargo, entre estas dos llamadas, el atacante puede modificar el archivo
`/etc/passwd` (o cualquier archivo en sí bajo `root`), dado que el archivo
`/tmp/XYZ` podría no ser el mismo archivo que se verifica con `access` y el que se abre con `fopen`.

Esto se logra por medio de un script que corre en paralelo, se ejecuta `vulp` por medio de `looper.sh`
el cual ejecuta el programa indefinidamente hasta que se determine que el archivo `/etc/passwd` ha sido modificado.

Casi paralelamente, el script `firegoblin` se encarga de antes de ejecutar `vulp`,
hacer un symlink de `/dev/null` a `/tmp/XYZ` y otro de `/etc/passwd` a `/tmp/lol`.
Esto se realiza antes de ejecutar el programa `vulp`, debido a que se necesita tener esos
links antes de que se realice la verificación para después por medio de la llamada `renameat2` en un loop
cambiar el archivo `/tmp/XYZ` por `/tmp/lol`, que es un enlace simbólico al archivo
`/etc/passwd`, esta llamada es atomica, por lo que básicamente es como hacer `symlink` y `unlink` al mismo tiempo,
aumentando las posibilidades de éxito.
Efectivamente, es muy exitoso y en cuestión de un par de segundos se logra
modificar el archivo `/etc/passwd` y se puede acceder al usuario creado llamado `pwned` con nivel de usuario `root`.

## Mejora de la tasa de éxito

Se redujo el valor de delay a 0 para comprobar si existe problemas para explotar la vulnerabilidad.
Se determinó que todavía se puede ejecutar la vulnerabilidad sin problema alguno y sin una perdida
en la tasa de éxito, esto, dado al uso de la función `renameat2`, que previene condiciones de carrera en
el script atacante `firegoblin.c`, además que el uso del loop en ambos archivos aumenta la probabilidades de éxito.

# Trabajo 2

Se realiza una modificación del programa `vulp.c` y se crea `vulp_repetition.c` que valida y abre el mismo archivo
varias veces con el fin de determinar si todos son el mismo archivo o no y ejecutar la concatenation de los caracteres
enviados.

Esto tiene el fin de hacer más difícil que se haga el ataque al tener que sobrepasar varias de estas pruebas. Para este
efecto se realizaron cuatro cadenas de verificación.

```

$ ./firegoblin

# En otra terminal (se modifica looper.sh para que use vulp_repetition)

$ ./looper.sh
b./looper.sh: line 14: 34543 Done echo 'pwned::0:0:pwned:/root:/bin/bash'
34544 Segmentation fault      (core dumped) | ./vulp_repetition
No permission fp3
No permission fp4
./looper.sh: line 14: 34552 Done echo 'pwned::0:0:pwned:/root:/bin/bash'
34553 Segmentation fault      (core dumped) | ./vulp_repetition
No permission fp2
Existe algun file descriptor invalido
Los file descriptors no apuntan al mismo archivo
No permission fp
No permission fp3
No permission fp4

```

En suma, se dejó el programa `looper.sh` corriendo durante un tiempo de 5 minutos y no se obtuvieron los resultados
anteriormente vistos, no fue posible realizar el ataque y el script menciona que no existen
los permisos para abrir el archivo tanto a nivel de la función `access` como en `fopen`.

Al bajar el nivel a solo dos verificaciones en lugar de cuatro,
si se pudo lograr modificar el archivo, pero con una tasa de éxito un poco menor,
se tomó menos de 10 segundos.

## Explicación del ataque

El ataque se basa en la misma lógica que el anterior, pero se agrega multiples llamadas a `access` y `fopen` para
abrir el mismo archivo `/tmp/XYZ` varias veces, se guarda los datos del archivo en variables y despues se extrae
su file descriptor para verificar que todos apuntan al mismo archivo por medio de unas llamadas a `fstat` y
verificando que el `st_dev` y `st_ino` son iguales, esto se hace para evitar que se pueda realizar el ataque.
Esto funciona ya que `st_dev` significa el id del dispositivo en el que se encuentra el archivo y
`st_ino` es el número de inodes del archivo, ambos son únicos para cada archivo en el sistema de archivos.

Por lo tanto se tiene que verificar que las cuatro iteraciones del archivo sean iguales, lo que hace más difícil
realizar el ataque.

# Trabajo 3

Principio de privilegio mínimo.

En esta parte se crea un nuevo script llamado `vulp_seteuid.c` que se encarga de bajar los privilegios del usuario
ejecutante del proceso al usuario que lo ejecuto, todo esto antes de las llamadas `access` y `fopen`.

Esto hace que aunque se den permisos de `SETUID` `root` al programa, este a nivel de código de manera explícita
los configura para bajar el nivel y que al momento de leer `/etc/passwd` en modo `a+` no se pueda, ya que el usuario
original no tiene permisos de `root` y no puede acceder al archivo de esta manera.

```
$ ./firegoblin

## En otra terminal (se modifica looper.sh para que use vulp_seteuid)

$ ./looper.sh
35397 Segmentation fault      (core dumped) | ./vulp_seteuid
No permission
./looper.sh: line 14: 35411 Done echo 'pwned::0:0:pwned:/root:/bin/bash'
35412 Segmentation fault      (core dumped) | ./vulp_seteuid
./looper.sh: line 14: 35423 Done echo 'pwned::0:0:pwned:/root:/bin/bash'
35424 Segmentation fault      (core dumped) | ./vulp_seteuid
./looper.sh: line 14: 35432 Done echo 'pwned::0:0:pwned:/root:/bin/bash'
35433 Segmentation fault      (core dumped) | ./vulp_seteuid
No permission
No permission
No permission
./looper.sh: line 14: 35453 Done echo 'pwned::0:0:pwned:/root:/bin/bash'
35454 Segmentation fault      (core dumped) | ./vulp_seteuid
No permission
```

## Explicación del ataque

Como se observa anteriormente, ya no se puede realizar el ataque porque por más que se haya
utilizado `setuid` como `root` para asignar el programa a ese usuario, el programa dentro de su código
baja los privilegios a los del nivel del usuario ejecutante antes de realizar los accesos a los archivos.
Por lo que no se puede acceder a `/etc/passwd` en modo `a+` y por ende no se puede modificar el archivo.

A nivel técnico, es posible realizar esto mediante la llamada `seteuid(real_user_id);`
que asigna el id de usuario efectivo al que usuario real del proceso llamado,
lo que hace que al momento de realizar las llamadas a `access` y `fopen`, este no tenga los permisos necesarios,
despues de eso se restablecen ya que se guardo el id de usuario efectivo en una variable
y se vuelve a asignar al final del programa.