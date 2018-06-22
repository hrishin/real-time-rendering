#include <stdio.h>


int main() {
	int width =64, height=64;
	int interval = 4;

	int array[width * height * interval];
	
	printf("Before :\n");

	for (int i=0; i < width*height*interval; i++) {
		printf("[%d] : %d\n", i, array[i]);
	}

	printf("Filling array:");
	for(int i=0; i < height; i++)
	{
		for(int j=0; j < width; j++)
		{
			int basei = i * width * interval;
			int basej = j * interval;

			for(int k=0; k < interval; k++) 
			{
				int c;
				if(k == interval - 1)
				{
					c = 255;
				}
				else 
				{
					c = (((i & 0X8) == 0) ^ ((j & 0X8) == 0)) * 255; 
				}
				array[basei + basej + k] = c; 
			}
		}
	}

	printf("After :\n");

	for (int i=0; i < width*height*interval; i++) {
		printf("[%d] : %d\n", i, array[i]);
	}


	return 0;
}
