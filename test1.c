#include <stdio.h>
int main () 
{    
	int aVal = 10;   
	int bbb;     
	for (int i = 0; i < 5; i++){
	        int* aaaaaa = &aVal;
	        bbb = *aaaaaa + 1;
	}    
	return 0;
}
