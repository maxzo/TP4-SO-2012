#include <fcntl.h>
#include <linux/fs.h>
#include <linux/ext2_fs.h>
#include <linux/magic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void datos_filesystem(char* path);
void entradas_directorios(char* path);
void consistencia_inodos(char* path);

int get_bloque_ind_simple(int fd, int numero_bloque, int tamanho_bloque,
	int punteros_por_bloque, char* buffer, int offset);
int get_bloque_ind_doble(int fd, int numero_bloque, int tamanho_bloque,
	int punteros_por_bloque, char* buffer, int offset);
int get_bloque_ind_triple(int fd, int numero_bloque, int tamanho_bloque,
	int punteros_por_bloque, char* buffer, int offset);

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
	lseek(fd, 1024, SEEK_SET);
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
	
	int tamanho_bloque;
	int bloques_por_grupo;
	int inodos_por_grupo;
	int punteros_por_bloque;
	
	fd = open(path, O_RDONLY);
	lseek(fd, 1024, SEEK_SET);
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
				* sizeof(struct ext2_group_desc), SEEK_SET);
			read(fd, grupo + i, sizeof(struct ext2_group_desc));
		}
		
		int numero_grupo = 2 / inodos_por_grupo;
		int offset_inodo = 2 - numero_grupo * inodos_por_grupo - 1;
		int offset_bloque = 1024 + (grupo[numero_grupo].bg_inode_table - 1) * tamanho_bloque;
		
		lseek(fd, offset_bloque + offset_inodo * sizeof(struct ext2_inode), SEEK_SET);
		read(fd, &inodo_raiz, sizeof(struct ext2_inode));
		
		if (S_ISDIR(inodo_raiz.i_mode))
		{
			char* buffer = malloc(tamanho_bloque * inodo_raiz.i_size);
			int tamanho_i = 0;
			struct ext2_dir_entry_2* entrada_directorio;
			
			int offset = 0;
			int bloque = 0;
			int cantidad_bloques = 0;
			
			while ((bloque < EXT2_NDIR_BLOCKS) && (cantidad_bloques <= inodo_raiz.i_blocks))
			{
				lseek(fd, 1024 + (inodo_raiz.i_block[bloque] - 1) * tamanho_bloque, SEEK_SET);
				read(fd, buffer + offset, tamanho_bloque);
				
				offset += tamanho_bloque;
				bloque++;
				cantidad_bloques++;
			}
			
			if (cantidad_bloques <= inodo_raiz.i_blocks)
			{
				offset = get_bloque_ind_simple(fd, inodo_raiz.i_block[EXT2_IND_BLOCK],
					tamanho_bloque, punteros_por_bloque, buffer, offset);
				cantidad_bloques += punteros_por_bloque;
			}
			
			if (cantidad_bloques <= inodo_raiz.i_blocks)
			{
				offset = get_bloque_ind_doble(fd, inodo_raiz.i_block[EXT2_IND_BLOCK],
					tamanho_bloque, punteros_por_bloque, buffer, offset);
				cantidad_bloques += punteros_por_bloque * punteros_por_bloque;
			}
			
			if (cantidad_bloques <= inodo_raiz.i_blocks)
			{
				offset = get_bloque_ind_triple(fd, inodo_raiz.i_block[EXT2_IND_BLOCK],
					tamanho_bloque, punteros_por_bloque, buffer, offset);
				cantidad_bloques += punteros_por_bloque * punteros_por_bloque
					* punteros_por_bloque;
			}
			
			char nombre_entrada[256];
			
			entrada_directorio = (struct ext2_dir_entry_2*) buffer;
			
			while (tamanho_i < inodo_raiz.i_size)
			{
				strncpy(nombre_entrada, entrada_directorio->name, entrada_directorio->name_len);
				nombre_entrada[entrada_directorio->name_len] = '\0';
				
				printf("Inodo: %2d - %s\n", entrada_directorio->inode, nombre_entrada);
				
				tamanho_i += entrada_directorio->rec_len;
				entrada_directorio = (void*) entrada_directorio + entrada_directorio->rec_len;
			}
		}
	}
	
	close(fd);
}

void consistencia_inodos(char* path)
{
	
}

int get_bloque_ind_simple(int fd, int numero_bloque, int tamanho_bloque,
	int punteros_por_bloque, char* buffer, int offset)
{
	char* buffer_local = malloc(tamanho_bloque);
	
	lseek(fd, 1024 + (numero_bloque - 1) * tamanho_bloque, SEEK_SET);
	read(fd, buffer_local, tamanho_bloque);
	
	int i;
	for (i = 0; i < punteros_por_bloque; i++)
	{
		lseek(fd, 1024 + (buffer_local[i] - 1) * tamanho_bloque, SEEK_SET);
		read(fd, buffer_local + offset, tamanho_bloque);
		offset += tamanho_bloque;
	}
	
	return offset;
}

int get_bloque_ind_doble(int fd, int numero_bloque, int tamanho_bloque,
	int punteros_por_bloque, char* buffer, int offset)
{
	printf("Lectura de bloque indirecto doble\n");
	exit(1);
}

int get_bloque_ind_triple(int fd, int numero_bloque, int tamanho_bloque,
	int punteros_por_bloque, char* buffer, int offset)
{
	printf("Lectura de bloque indirecto triple\n");
	exit(1);
}
