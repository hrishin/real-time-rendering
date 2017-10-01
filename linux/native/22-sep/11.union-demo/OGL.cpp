#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


struct Person
{
	char *name;
	union gender 
	{
		bool male;
		bool female;
		bool other;
	};
};

void process_person_info(struct Person *p)
{
	if(p -> gender == NULL)
	{
		printf("Apply male rules \n");
	}
	else if(p -> female) 
	{
		printf("Apply female rules \n");
	}
	else 
	{
		printf("Apply other rules");
	}
	
}

// entry point function
int main(int args, char *argv[])
{

	return (0);		
}


