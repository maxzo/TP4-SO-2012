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

void leer_inodo(int fd, struct ext2_super_block* super_block,
	struct ext2_group_desc* descriptor_grupo, int numero_inodo);

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
	struct ext2_group_desc descriptor_grupo;
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
		lseek(fd, 1024 + EXT2_BLOCK_SIZE(&super_block), SEEK_SET);
		read(fd, &descriptor_grupo, sizeof(struct ext2_group_desc));
		
		leer_inodo(fd, &super_block, &descriptor_grupo, EXT2_ROOT_INO);
	}
	
	close(fd);
}

void consistencia_inodos(char* path)
{
	int fd;
	struct ext2_group_desc descriptor_grupo;
	struct ext2_super_block super_block;
	
	int tamanho_bloque;
	
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
		
		char* buffer = malloc(tamanho_bloque);
		
		lseek(fd, 2048, SEEK_SET);
		read(fd, &descriptor_grupo, sizeof(struct ext2_group_desc));
		
		lseek(fd, 1024 + (descriptor_grupo.bg_inode_bitmap - 1) * tamanho_bloque, SEEK_SET);
		read(fd, buffer, tamanho_bloque);
		
		int i = 0, j;
		int inodos_libres_s_bitmap = 0;
		
		do
		{
			for (j = 7; j > -1; j--)
			{
				if ((buffer[i] >> j) % 2 == 0) /* Me fijo si el último bit es 0 */
				{
					inodos_libres_s_bitmap++;
				}
			}
		}
		while (buffer[++i] != 0);
		
		do
		{
			inodos_libres_s_bitmap += 8;
		}
		while (buffer[++i] == 0);
		
		printf("Cantidad de inodos libres (segun superblock): %d\n", super_block.s_free_inodes_count);
		printf("Cantidad de inodos libres (segun bitmap de inodos): %d\n", inodos_libres_s_bitmap);
	}
	
	close(fd);
}

void leer_inodo(int fd, struct ext2_super_block* super_block,
	struct ext2_group_desc* descriptor_grupo, int numero_inodo)
{
	struct ext2_inode inodo;
	int offset_aux = lseek(fd, 0, SEEK_CUR);
	
	/* Primero me muevo a la tabla de inodos y luego al inodo */
	lseek(fd, 1024 + (descriptor_grupo->bg_inode_table - 1) * EXT2_BLOCK_SIZE(super_block), SEEK_SET);
	lseek(fd, sizeof(struct ext2_inode) * (numero_inodo - 1), SEEK_CUR);
	read(fd, &inodo, sizeof(struct ext2_inode));
	
	if (S_ISDIR(inodo.i_mode))
	{
		lseek(fd, 1024 + (inodo.i_block[0] - 1) * EXT2_BLOCK_SIZE(super_block), SEEK_SET);
		
		struct ext2_dir_entry_2 entrada_directorio;
		int offset;
		int suma_rec_len = 0;
		
		read(fd, &(entrada_directorio.inode), sizeof(__le32));
		read(fd, &(entrada_directorio.rec_len), sizeof(__le16));
		read(fd, &(entrada_directorio.name_len), sizeof(__u8));
		read(fd, &(entrada_directorio.file_type), sizeof(__u8));
		read(fd, &(entrada_directorio.name), entrada_directorio.name_len);
		entrada_directorio.name[entrada_directorio.name_len] = '\0';
	
		suma_rec_len += entrada_directorio.rec_len;
		
		while (suma_rec_len <= EXT2_BLOCK_SIZE(super_block)
			&& entrada_directorio.rec_len != 0)
		{
			printf("Inodo: %2d - %s", entrada_directorio.inode, entrada_directorio.name);
			
			if (entrada_directorio.file_type == EXT2_FT_DIR
				&& (!(entrada_directorio.name_len == 1 && entrada_directorio.name[0] == '.')
				&& !(entrada_directorio.name_len == 2 && entrada_directorio.name[0] == '.' && entrada_directorio.name[1] == '.')))
			{
				printf("/\n");
				leer_inodo(fd, super_block, descriptor_grupo, entrada_directorio.inode);
			}
			else
			{
				printf("\n");
			}
			
			offset = entrada_directorio.rec_len - sizeof(__le32) - sizeof(__le16) - sizeof(__u8)
				- sizeof(__u8) - entrada_directorio.name_len;
			lseek(fd, offset, SEEK_CUR);
			
			read(fd, &(entrada_directorio.inode), sizeof(__le32));
			read(fd, &(entrada_directorio.rec_len), sizeof(__le16));
			read(fd, &(entrada_directorio.name_len), sizeof(__u8));
			read(fd, &(entrada_directorio.file_type), sizeof(__u8));
			read(fd, &(entrada_directorio.name), entrada_directorio.name_len);
			entrada_directorio.name[entrada_directorio.name_len] = '\0';
			
			suma_rec_len += entrada_directorio.rec_len;
		}
		
		lseek(fd, offset_aux, SEEK_SET);
	}
}
