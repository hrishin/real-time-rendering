#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

Typedef int (*add)(int,int);

int main()
{
	void *handle = NULL;
	
	handle = dlopen("./mathlib.so", RTLD_LAZY);
	if(handle == NULL)
	{
		printf("unable to load lib\n");
		exit(EXIT_FAILURE);
	}
	
	add pfn = (add) dlsym(handle, "add");
	printf("\n %d", pfn(1,1));
	
	dlclose(handle);
	return (0);	
}
