
#include <stdio.h>
#include "MulticastErr.h"
#include "MulticastMsg.h"
#include "FecUnpad.h"

int main(int argc, char* argv[])
{
	int r;

	r = multiCastCreateMsg();
	if(r != SUCCESS) {
		printf("Init message error, return code: %d\n", r);
		return r;
	}

	r = FecUnpadInit();
	if(r != SUCCESS) {
		printf("Init fecUnpad error, return code: %d\n", r);
		return r;
	}

	r = FecSetOutFile();
	if(r != SUCCESS) {
		printf("FecUnpad open file error, return code: %d\n", r);
		return r;
	}

	while(1) {
		r = FecUnpad();
		if(r != SUCCESS) {
			printf("unpad error, return code: %d\n", r);
		}
	}

	return 0;
}

