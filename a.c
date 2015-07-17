#include<stdio.h>

int main()
{
	union
	{
		short value ;
		char union_bytes[sizeof(short)];
	}test ;
	test.value = 0x0102;
	if(test.union_bytes[0]==1 ){
		printf("big endian\n");
	}else{
		printf("little endian\n");
	}

	printf("%d\n",sizeof(int*));
	return 0;
}	
