#include <fcntl.h>
#include <linux/fs.h>
#include <linux/ext2_fs.h>
#include <linux/magic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void datos_filesystem(char* path);
void entradas_directorios(char* path);
void consistencia_inodos(char* path);

int main(int argc, char *argv[])
{
	char opcion = getopt(argc, argv, "s:e:c:");
	
	switch (opcion)
	{
		case 's':
			datos_filesystem(optarg);
			break;
		case 'e':
			entradas_directorios(optarg);
			break;
		case 'c':
			consistencia_inodos(optarg);
			break;
		default:
			printf("Sintaxis: %s [-sec] /dev/fd0\n", argv[0]);
	}
	
	return 0;
}

void datos_filesystem(char* path)
{
	int fd;
	struct ext2_super_block super_block;
	
	fd = open(path, O_RDONLY);
	lseek(fd, 1024, SEEK_CUR);
	read(fd, &super_block, sizeof(struct ext2_super_block));
	
	if (super_block.s_magic != EXT2_SUPER_MAGIC)
	{
		printf("ERROR: La unidad no tiene un sistema de archivos EXT2\n");
	}
	else
	{
		printf("Sistema de archivos: EXT2\n");
		printf("Cantidad de inodos: %d\n", super_block.s_inodes_count);
		printf("Cantidad de inodos libres: %d\n", super_block.s_free_inodes_count);
		printf("Primer inodo usable: %d\n", super_block.s_first_ino);
	}
	
	close(fd);
}

void entradas_directorios(char* path)
{
	
}

void consistencia_inodos(char* path)
{
	
}
