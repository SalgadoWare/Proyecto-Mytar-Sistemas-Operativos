# Comprueba que mytar esta en el directorio actual y es ejecutable:
if [ ! -e mytar ]
then echo "No existe"
exit 1
else 
if [ ! -x mytar ]
then echo "No ejecutable"
exit 1
fi
fi
# Comprueba si existe tmp en el directorio actual:
if [ -d tmp ]
then rm -r tmp
fi
# Crea un tmp en el directorio actual y cambia de directorio:
mkdir tmp
cd tmp
# Crea los ficheros file1.txt, file2.txt y file3.dat:
touch file1.txt
echo "Hello world!" > file1.txt
touch file2.txt
head /etc/passwd > file2.txt
touch file3.dat
head -c 1024 /dev/urandom > file3.dat
# Invoca mytar:
../mytar -cf filetar.mtar file1.txt file2.txt file3.dat
# Crea out y copia filetar.mtar:
mkdir out
cp filetar.mtar out
# Cambia al directorio out y ejecuta mytar:
cd out
../../mytar -xf filetar.mtar
# Compara los ficheros con diff:
x=1
if diff -q file1.txt ../file1.txt
then if diff -q file2.txt ../file2.txt
then if diff -q file3.dat ../file3.dat
then x=0
fi
fi
fi
# Si los ficheros extraídos y originales son iguales devuelve Correct, sino muestra Error:
cd ../../
if [ $x=0 ]
then echo "Correct"
else
echo "Error"
fi
exit $x

