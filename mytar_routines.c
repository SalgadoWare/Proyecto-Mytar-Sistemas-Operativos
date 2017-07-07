#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int copynFile(FILE * origin, FILE * destination, int nBytes) {
	int ByteLectura = 0;
	int BytesTotales = 0;
	int ByteEscritura = 0;

	if (origin != NULL) {
		while ((BytesTotales < nBytes) && (ByteLectura = getc(origin)) != EOF) {

			ByteEscritura = putc(ByteLectura, destination);
			if (ByteEscritura == EOF) {
				return -1;
			}
			BytesTotales++;
		}

		return BytesTotales;
	} else {
		return -1;
	}

}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor
 * buf: parameter to return the read string. Buf is a
 * string passed by reference.
 *
 * The loadstr() function must allocate memory from the heap to store
 * the contents of the string read from the FILE.
 * Once the string has been properly "built" in memory, return the starting
 * address of the string (pointer returned by malloc()) in the buf parameter
 * (*buf)=<address>;
 *
 * Returns: 0 if success, -1 if error
 */
char* loadstr(FILE * file) {
	int longitudNombre = 0; //en bytes
	char *name;
	char bit;

	while ((bit = getc(file) != '\0')) {
		longitudNombre++;
		if (bit == 0) {
		}
	}

	name = malloc(sizeof(char) * (longitudNombre + 1)); // +1 por \0
	fseek(file, -(longitudNombre + 1), SEEK_CUR);
	int i = 0;
	for (i = 0; i < longitudNombre + 1; i++) {
		name[i] = getc(file);
	}

	return name;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor
 * header: output parameter. It is used to return the starting memory address
 * of an array that contains the (name,size) pairs read from the tar file
 * nFiles: output parameter. Used to return the number of files stored in
 * the tarball archive (first 4 bytes of the header)
 *
 * On success it returns EXIT_SUCCESS. Upon failure, EXIT_FAILURE is returned.
 * (both macros are defined in stdlib.h).
 */
stHeaderEntry* readHeader(FILE * tarFile, int *nFiles) {

	int totalArchivos = 0;
	int nBytes = 0; // Tamaño de cada archivo
	stHeaderEntry *EstructurasCabecera = NULL;

	if (fread(&totalArchivos, sizeof(int), 1, tarFile) == 0) {
		printf("Error de lectura\n");
		return NULL;
	} else {

		EstructurasCabecera = malloc(sizeof(stHeaderEntry) * totalArchivos);

		int i = 0;

		for (i = 0; i < totalArchivos; i++) {

			//Cargamos el nombre de los ficheros a descomprimir
			EstructurasCabecera[i].name = loadstr(tarFile);

			//Leemos y cargamos la longitud en bytes del archivio descomprimido
			fread(&nBytes, sizeof(unsigned int), 1, tarFile);
			EstructurasCabecera[i].size = nBytes;

		}

		(*nFiles) = totalArchivos;
		return EstructurasCabecera;
	}

}

/** Creates a tarball archive
 *
 *	llamada : retCode=createTar(nExtra, &argv[optind], tarName);
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE.
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive.
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size)
 * pairs occupy strlen(name)+1 bytes.
 *
 */

// OBJETIVE: write in the tarbar file the header (filename+size) and the data for each file
// First calculate the size of the header (filename+size) for all the struct
int createTar(int nFiles, char *fileNames[], char tarName[]) {
	FILE * archivoEntra;
	FILE * archivoSale;
	stHeaderEntry *PunteroDescriptores;

	int BytesCopiados = 0, BytesTotales = 0;

	BytesTotales += sizeof(int); //4 bytes del entero inicial del tar
	BytesTotales += nFiles * sizeof(unsigned int); // Se suman los bytes correspondientes al unsigned int que representa el tamaño de cada archivo del tar

	int i = 0;
	for (i = 0; i < nFiles; i++) {
		BytesTotales += strlen(fileNames[i]) + 1; // Se suman los bytes correspondeintes a los nombres de los archivos
	}

	archivoSale = fopen(tarName, "w"); // Trunca a tamaño cero el fichero si existe y si no existe lo crea.
	//El fichero se abre en escritura apuntando el indicador de posición al comienzo del fichero.

	//Tenemos que situarnos en la parte del binario el tar para escribir los bits del los archivos de entrada,
	//es decir, dejar hueco a la cabecera
	fseek(archivoSale, BytesTotales, SEEK_SET);

	PunteroDescriptores = malloc(sizeof(stHeaderEntry) * nFiles); //Memoria reservada para los descriptores

	//Usamos un bucle para rellenar los descriptores
	for (i = 0; i < nFiles; i++) {

		if ((archivoEntra = fopen(fileNames[i], "r")) == NULL) {
			printf("Error! El archivo %s no existe\n", fileNames[i]);

			return (EXIT_FAILURE);
		} else {

			//Copiamos todos los bytes de cada archivo de lectura
			BytesCopiados = copynFile(archivoEntra, archivoSale, INT_MAX);

			if (BytesCopiados == -1) {
				printf("Error en la copia de %s\n", fileNames[i]);
				return EXIT_FAILURE;
			} else {

				//El nombre del descriptor es un puntero a char:
				PunteroDescriptores[i].name = malloc(sizeof(fileNames[i]) + 1);

				//Se copia el string
				strcpy(PunteroDescriptores[i].name, fileNames[i]);

				//El tamaño es igual a los bytes copiados
				PunteroDescriptores[i].size = BytesCopiados;
				printf("El archivo %s tiene %d bytes\n",
						PunteroDescriptores[i].name,
						PunteroDescriptores[i].size);
			}

			//Error si no se puede cerrar
			if (fclose(archivoEntra) == EOF)
				return EXIT_FAILURE;
		}
	}

	//----------------------------------------
	//Ahora movemos el puntero de nuestro nuevo tar al inicio para escribir la cabecera
	fseek(archivoSale, 0, SEEK_SET);

	fwrite(&nFiles, sizeof(int), 1, archivoSale); //Escribimos el numero de archivos que contiene que es lo primero

	//Escribimos cada descriptor
	for (i = 0; i < nFiles; i++) {
		fwrite(PunteroDescriptores[i].name,
				strlen(PunteroDescriptores[i].name) + 1, 1, archivoSale);
		fwrite(&PunteroDescriptores[i].size, sizeof(unsigned int), 1,
				archivoSale);
	}

	//------------------------------------------------
	// Liberamos memoria dinámica
	for (i = 0; i < nFiles; i++) {
		free(PunteroDescriptores[i].name); //Puntero a char
	}

	free(PunteroDescriptores);

	//Cerramos el archivo
	if (fclose(archivoSale) == EOF) {
		return (EXIT_FAILURE);
	}

	printf("El archivo %s se ha creado correctamente\n", tarName);

	return (EXIT_SUCCESS);
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE.
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the
 * tarball's data section. By using information from the
 * header --number of files and (file name, file size) pairs--, extract files
 * stored in the data section of the tarball.
 *
 */
int extractTar(char tarName[]) {
	FILE *ArchivoTar = NULL;
	FILE *ArchivoDescomprimido = NULL;
	stHeaderEntry* PunteroDescriptores;

	int totalArchivos = 0, BytesCopiados = 0;

	if ((ArchivoTar = fopen(tarName, "r")) == NULL) {
		printf("No se encuentra el archivo %s", tarName);
		return (EXIT_FAILURE);
	} else {

		PunteroDescriptores = readHeader(ArchivoTar, &totalArchivos);

		if (PunteroDescriptores == NULL) {
			printf("No se puede leer la cabecera \n");
			return (EXIT_FAILURE);
		} else {

			int i = 0;

			for (i = 0; i < totalArchivos; i++) {

				if ((ArchivoDescomprimido = fopen(PunteroDescriptores[i].name,
						"w")) == NULL) {
					return EXIT_FAILURE;
				} else {
					BytesCopiados = copynFile(ArchivoTar, ArchivoDescomprimido,
							PunteroDescriptores[i].size);
					if (BytesCopiados == -1) {
						return EXIT_FAILURE;
					}
				}

				fclose(ArchivoDescomprimido);
			}

			printf("El tar contenia %d\n", totalArchivos);
			for (i = 0; i < totalArchivos; i++) {
				printf("El archivo %s tenia %d bytes\n",
						PunteroDescriptores[i].name,
						PunteroDescriptores[i].size);

			}

			// Se libera la memoria
			for (i = 0; i < totalArchivos; i++) {
				free(PunteroDescriptores[i].name);
			}

			free(PunteroDescriptores);
			fclose(ArchivoTar);
			return (EXIT_SUCCESS);

		}
	}
}
