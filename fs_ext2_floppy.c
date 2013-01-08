#include <stdio.h>
#include <sys/vfs.h>
#include <unistd.h>

/* Extraídas del man page de statfs */
#define EXT2_OLD_SUPER_MAGIC 0xEF51
#define EXT2_SUPER_MAGIC 0xEF53

int formato_ext2(char* path);

int main(int argc, char *argv[])
{
	char opcion = getopt(argc, argv, "s:e:c:");
	
	switch (opcion)
	{
		case 's':
			if (formato_ext2(optarg))
			{
				
			}
			else
			{
				
			}
			break;
		case 'e':
			if (formato_ext2(optarg))
			{
				
			}
			else
			{
				
			}
			break;
		case 'c':
			if (formato_ext2(optarg))
			{
				
			}
			else
			{
				
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
