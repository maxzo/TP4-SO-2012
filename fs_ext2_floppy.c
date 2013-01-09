#include <ext2_fs.h>
#include <stdio.h>
#include <sys/vfs.h>
#include <unistd.h>

/* Extraídas del man page de statfs */
#define EXT2_OLD_SUPER_MAGIC 0xEF51
#define EXT2_SUPER_MAGIC 0xEF53

int formato_ext2(char* path);
void datos_filesystem(char* path);
void entradas_directorios(char* path);
void consistencia_inodos(char* path);

int main(int argc, char *argv[])
{
	char opcion = getopt(argc, argv, "s:e:c:");
	
	switch (opcion)
	{
		case 's':
			if (formato_ext2(optarg))
			{
				datos_filesystem(optarg);
			}
			else
			{
				printf("ERROR: La unidad no tiene un sistema de archivos EXT2\n");
			}
			break;
		case 'e':
			if (formato_ext2(optarg))
			{
				entradas_directorios(optarg);
			}
			else
			{
				printf("ERROR: La unidad no tiene un sistema de archivos EXT2\n");
			}
			break;
		case 'c':
			if (formato_ext2(optarg))
			{
				consistencia_inodos(optarg);
			}
			else
			{
				printf("ERROR: La unidad no tiene un sistema de archivos EXT2\n");
			}
			break;
		default:
			printf("Sintaxis: %s [-sec] /dev/fd0\n", argv[0]);
	}
	
	return 0;
}

int formato_ext2(char* path)
{
	struct statfs buf;
	
	if (statfs(path, &buf) == 0) /* Éxito */
	{
		return (buf.f_type == EXT2_OLD_SUPER_MAGIC || buf.f_type == EXT2_SUPER_MAGIC);
	}
	
	return 0;
}

void datos_filesystem(char* path)
{
	
}

void entradas_directorios(char* path)
{
	
}

void consistencia_inodos(char* path)
{
	
}
