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
	int fd;
	struct ext2_group_desc* grupo;
	struct ext2_inode inodo_raiz;
	struct ext2_super_block super_block;
	
	int tamanho_bloque ;
	int bloques_por_grupo ;
	int inodos_por_grupo ;
	int punteros_por_bloque;
	
	fd = open(path, O_RDONLY);
	lseek(fd, 1024, SEEK_CUR);
	read(fd, &super_block, sizeof(struct ext2_super_block));
	
	if (super_block.s_magic != EXT2_SUPER_MAGIC)
	{
		printf("ERROR: La unidad no tiene un sistema de archivos EXT2\n");
	}
	else
	{
		tamanho_bloque = 1024 << super_block.s_log_block_size;
		bloques_por_grupo = super_block.s_blocks_per_group;
		inodos_por_grupo = super_block.s_inodes_per_group;
		
		int cantidad_grupos = (super_block.s_blocks_count + bloques_por_grupo - 1)
			/ bloques_por_grupo;
		int tamanho = cantidad_grupos * sizeof(struct ext2_group_desc);
		grupo = malloc(tamanho);
		
		int i;
		for (i = 0; i < cantidad_grupos; i++)
		{
			lseek(fd, 1024 + tamanho_bloque + i
				* sizeof(struct ext2_group_desc), SEEK_CUR);
			read(fd, grupo + i, sizeof(struct ext2_group_desc));
			
			printf("Bloques libres: %d\n", grupo[i].bg_free_blocks_count);
			printf("Inodos libre: %d\n", grupo[i].bg_free_inodes_count);
			printf("Directorios: %d\n", grupo[i].bg_used_dirs_count);
		}
	}
	
	close(fd);
}

void consistencia_inodos(char* path)
{
	
}
