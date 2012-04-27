#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ndsrom.h"
#include "tree.h"

int main(int argc, char **argv)
{
	nds_file *stuff;
	
	stuff = nds_do_magic("polarium.nds");
	free_nds_file(stuff);
	
	return 0;
}
