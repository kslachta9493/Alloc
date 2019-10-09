#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 	
	1. k number of registers
	2. flags
		b - bottom up
		s - simple top down
		t - top down with live ranges
		o - my own top down
	3. name of input file
*/
typedef struct ops
{
	char* op;
	int reg1;
	int reg2;
	int reg3;
} operator;
int row = 0;
int reg = 0;
int instruct = 0;
char *strip(char* found)
{
	int j = 0;
	for (j = 0; j < sizeof(found); j++)
	{
		if (found[j + 1] == ',')
			found[j + 1] = '\0';
		found[j] = found[j+1];
	}
	return found;
}
int findOcc (int find, int livereg[], int k)
{
	int i;
	for (i = 0; i < k; i++)
	{
		if (livereg[i] == find)
		{
			return i;
		}
	}
	return 0;
}
void printILOC (operator ops[])
{
}
int findOff(int find, int offset[100][2])
{
	int m = 1;
	while (offset[m][0] != find)
	{
		if (m > 98)
		{
			return m;
		}
		m++;
	}
	return m;						
}
void bottomup (int k, operator ops[])
{
	int livemap[instruct][row];
	int i = 0;
	int j = 0;
	int fill = 0;
	int count =0;
	reg++;

	for (i = 0; i < instruct; i++)
	{	
		for (j = 0; j < reg; j++)
		{
			livemap[i][j] = 0;
		}
	}

	for (i = 1; i < reg; i++)
	{
		count = 0;
		for (j = instruct - 1; j >= 0; j--)
		{
			if (strcmp(ops[j].op, "loadI") == 0)
			{
				if (ops[j].reg2 == i)
				{
					count++;
					livemap[j][i] = count;
				}
			} else if (strcmp(ops[i].op, "addI") == 0 || strcmp(ops[i].op, "multI") == 0 
				|| strcmp(ops[i].op, "subI") == 0 || strcmp(ops[i].op, "divI") == 0 
				|| strcmp(ops[i].op, "lshiftI") == 0 || strcmp(ops[i].op, "rshiftI") == 0)
			{
				if (ops[j].reg1 == i || ops[j].reg3 == i)
				{
					count++;
					livemap[j][i] = count;
				}
			} else
			{
				if (ops[j].reg1 == i || ops[j].reg2 == i || ops[j].reg3 == i)
				{
					count++;
					livemap[j][i] = count;
				}
			}
		}
	}
	
	for (i = 0; i < reg; i++)
	{
		fill = 0;
		for (j = 0; j < instruct; j++)
		{
			if (livemap[j][i] > 1)
			{
				fill = 1;
				livemap[j][i] = 1;
			} else if (livemap[j][i] == 1)
			{
				fill = 0;
				livemap[j][i] = 0;
			} else if (fill == 1)
			{
				livemap[j][i] = 1;
			}
		}
	}
	if (k > reg)
	{
		printILOC(ops);
	} else
	{
		int livereg[k + 1];
		for (i = 0; i < k + 1; i++)
		{
			livereg[i] = 0;
		}
		int assigned = 0;
		int offset[100][2];
		for (i = 0; i < 100; i++)
		{
			for (j = 0; j < 2; j++)
			{
				offset[i][j] = 0;
			}
		}
		printf("\t%s\t%d\t=>\tr%d\n", ops[0].op, ops[0].reg1, ops[0].reg2);

		for (i = 1; i < instruct; i++)
		{
			assigned = 0;
			//printf("%s\n", ops[i].op);
			if (strcmp(ops[i].op, "loadI") == 0)
			{
				for (j = 1; j < k + 1; j++)
				{
					if (livereg[j] == 0)
					{
						livereg[j] = ops[i].reg2;
						printf("\t%s\t%d\t=>\tr%d\n", ops[i].op, ops[i].reg1, j);
						assigned = 1;
						break;
					} else if (livemap[i][livereg[j]] == 0)
					{
						livereg[j] = ops[i].reg2;
						printf("\t%s\t%d\t=>\tr%d\n", ops[i].op, ops[i].reg1, j);
						assigned = 1;
						break;
					} 
					//spill code for loadI
				}
				if (assigned == 0)
				{
					j = findOff(0, offset);
					assigned = 1;
					offset[j][0] = ops[i].reg2;
					offset[j][1] = -4 * j;
					livereg[assigned] = ops[i].reg2;
					printf("\tstoreAI\tr1\t=>r0, %d\n", offset[j][1]);
					printf("\t%s\t%d\t=>\tr%d\n", ops[i].op, ops[i].reg1, assigned);
				}
			} else if (strcmp(ops[i].op, "addI") == 0 || strcmp(ops[i].op, "multI") == 0 
				|| strcmp(ops[i].op, "subI") == 0 || strcmp(ops[i].op, "divI") == 0 
				|| strcmp(ops[i].op, "lshiftI") == 0 || strcmp(ops[i].op, "rshiftI") == 0)
			{
				assigned = 0;
				for (j = 1; j < k + 1; j++)
				{
					if (livereg[j] == ops[i].reg1)
					{
						//register is already loaded
						assigned = 1;
						break;
					}
				}
				if (assigned == 0)
				{
					for (j = 1; j < k + 1; j++)
					{
						if (livemap[i][livereg[j]] == 0)
						{
							//found dead, register load active value into it
							int m = 1;
							while (offset[m][0] != ops[i].reg1)
							{
								m++;
							}
							
							printf("\tloadAI\tr0, %d\t=>\tr%d\n", -1 * offset[m][1], j);
							offset[m][0] = 0;
							offset[m][1] = 0;
							livereg[j] = ops[i].reg1;
							assigned = 1;
							break;
						}
					}
					if (assigned == 0)
					{
						j = 1;
						while (offset[j][0] != 0)
						{
							j++;
						}
						offset[j][1] = j * -4;
						offset[j][0] = livereg[1];
						printf("\tstoreAI\tr1\t=>\tr0, %d\n", offset[j][1]);
						int m = 1;
						while (offset[m][0] != ops[i].reg1)
						{
							m++;
						}
						
						printf("\tloadAI\tr0, %d\t=>\tr%d\n", -1 * offset[m][1], j);
						offset[m][0] = 0;
						offset[m][1] = 0;
						livereg[1] = ops[i].reg1;
						assigned = 1;
					}
				}

				printf("\t%s\t", ops[i].op);
				j = 1;
				while (livereg[j] != ops[i].reg1)
				{
					j++;
				}
				printf("r%d, %d\t=>", j, ops[i].reg2);
				for (j = 1; j < k + 1; j++)
				{
					if (livemap[i][livereg[j]] == 0)
					{
						//found dead, register load output value into it
						livereg[j] = ops[i].reg3;
						break;
					}
				}
				if (livereg[j] != ops[i].reg3)
				{
					j = 1;
					while (offset[j][0] != 0)
					{
						j++;
					}
					offset[j][1] = j * -4;
					offset[j][0] = livereg[1];
					livereg[1] = ops[i].reg3;
					printf("\tstoreAI\tr1\t=>\tr0, %d\n", offset[j][1]);
				}
				printf("\tr%d\n", j);
			} else if (strcmp(ops[i].op, "add") == 0 || strcmp(ops[i].op, "mult") == 0 
				|| strcmp(ops[i].op, "sub") == 0 || strcmp(ops[i].op, "div") == 0 
				|| strcmp(ops[i].op, "lshift") == 0 || strcmp(ops[i].op, "rshift") == 0)
			{

				int reg1 = 0;
				int reg2 = 0;
				assigned = 0;
				for (j = 1; j < k + 1; j++)
				{
					if (livereg[j] == ops[i].reg1)
					{
						reg1 = j;
					} else if (livereg[j] == ops[i].reg2)
					{
						reg2 = j;
					}
				}
				if (reg1 == 0)
				{
					//reg1 isn't loaded find an empty or dead register and load it, else spill one
					for (j = 1; j < k + 1; j++)
					{
						if (livemap[i][livereg[j]] == 0)
						{
							//found dead, register load active value into it
							int m = 1;
							while (offset[m][0] != ops[i].reg1)
							{
								m++;
							}
							
							printf("\tloadAI\tr0, %d\t=>\tr%d\n", -1 * offset[m][1], j);
							offset[m][0] = 0;
							offset[m][1] = 0;
							livereg[j] = ops[i].reg1;
							reg1 = 1;
							break;
						}
					}
					if (reg1 == 0)
					{
					//no dead register must spill one
						int m;
						j = findOff(ops[i].reg1, offset);
						m = findOff(0, offset);

						while (reg1 != reg2)
						{
							reg1++;
						}
						offset[m][1] = m * -4;
						offset[m][0] = livereg[reg1];
						livereg[reg1] = ops[i].reg3;
						printf("\tstoreAI\tr%d\t=>\tr0, %d\n", reg1, offset[m][1]);
						printf("\tloadAI\tr0, %d\t=>\tr%d\n", -1 * offset[j][1], reg1);
						offset[j][1] = 0;
						offset[j][0] = 0;
					}
				}
				if (reg2 == 0)
				{
					//reg2 isn't loaded find an empty or dead register and load it, else spill one
					for (j = 1; j < k + 1; j++)
					{
						if (livemap[i][livereg[j]] == 0)
						{
							//found dead, register load active value into it
							int m;
							m = findOff(ops[i].reg2, offset);
							printf("\tloadAI\tr0, %d\t=>\tr%d\n", -1 * offset[m][1], j);
							offset[m][0] = 0;
							offset[m][1] = 0;
							livereg[j] = ops[i].reg1;
							reg2 = j;
							break;
						}
					}
					if (reg2 == 0)
					{
					//no dead register must spill one
						int m;
						j = findOff(ops[i].reg2, offset);
						m = findOff(0, offset);

						while (reg1 != reg2)
						{
							reg2++;
						}
						offset[m][1] = m * -4;
						offset[m][0] = livereg[reg2];
						livereg[reg2] = ops[i].reg3;
						printf("\tstoreAI\tr%d\t=>\tr0, %d\n", reg2, offset[m][1]);
						printf("\tloadAI\tr0, %d\t=>\tr%d\n", -1 * offset[j][1], reg2);
						offset[j][1] = 0;
						offset[j][0] = 0;
					}
				}
				int outreg = 0;
				for (j = 1; j < k + 1; j++)
				{
					if (livereg[j] == 0)
					{
						outreg = j;
						livereg[j] = ops[i].reg3;
						break;
					} else if (livemap[i][livereg[j]] == 0)
					{
						outreg = j;
						livereg[j] = ops[i].reg3;
					}
				}
				if (outreg == 0)
				{
					//spill r1
					int m;
					m = findOff(0, offset);
					offset[m][0] = livereg[1];
					offset[m][1] = m * -4;
					printf("\tstoreAI\tr1\t=>\tr0, %d\n", offset[m][1]);
					outreg = 1;
					livereg[1] = ops[i].reg3;
				}
				printf("\t%s\tr%d, r%d\t=>\tr%d\n",ops[i].op, reg1, reg2, outreg);
			} else if (strcmp(ops[i].op, "store") == 0 || strcmp(ops[i].op, "load") == 0)
			{
				int store1 = 0;
				int store2 = 0;
				for (j = 1; j < k + 1; j++)
				{
					if (livereg[j] == ops[i].reg1)
						store1 = j;
					if (livereg[j] == ops[i].reg2)
						store2 = j;

				}

				if (store1 == 0)
				{
					//reg1 isn't loaded
					for (j = 1; j < k + 1; j++)
					{
						if (livemap[i][livereg[j]] == 0)
						{
							//found dead, register load active value into it
							int m;
							m = findOff(ops[i].reg1, offset);
							printf("\tloadAI\tr0, %d\t=>\tr%d\n", -1 * offset[m][1], j);
							offset[m][0] = 0;
							offset[m][1] = 0;
							livereg[j] = ops[i].reg1;
							store1 = j;
							break;
						}
					}
					if (store1 == 0)
					{
						while (store1 != store2)
						{
							store1++;
						}
						int m;
						j = findOff(ops[i].reg1, offset);
						m = findOff(0, offset);

						offset[m][1] = m * -4;
						offset[m][0] = livereg[store1];
						livereg[store1] = ops[i].reg1;
						printf("\tstoreAI\tr%d\t=>\tr0, %d\n", store1, offset[m][1]);
						printf("\tloadAI\tr0, %d\t=>\tr%d\n", -1 * offset[j][1], store1);
						offset[j][1] = 0;
						offset[j][0] = 0;
					}
				}
				if (store2 == 0 && strcmp(ops[i].op, "store") == 0)
				{
					//reg2 isn't loaded
					for (j = 1; j < k + 1; j++)
					{
						if (livemap[i][livereg[j]] == 0)
						{
							//found dead, register load active value into it
							int m;
							m = findOff(ops[i].reg1, offset);
							printf("\tloadAI\tr0, %d\t=>\tr%d\n", -1 * offset[m][1], j);
							offset[m][0] = 0;
							offset[m][1] = 0;
							livereg[j] = ops[i].reg1;
							store1 = j;
							break;
						}
					}
					if (store2 == 0)
					{
						while (store1 != store2)
						{
							store2++;
						}
						int m;
						j = findOff(ops[i].reg2, offset);
						m = findOff(0, offset);

						offset[m][1] = m * -4;
						offset[m][0] = livereg[store2];
						livereg[store2] = ops[i].reg1;
						printf("\tstoreAI\tr%d\t=>\tr0, %d\n", store2, offset[m][1]);
						printf("\tloadAI\tr0, %d\t=>\tr%d\n", -1 * offset[j][1], store2);
						offset[j][1] = 0;
						offset[j][0] = 0;
					}
				} else if (store2 == 0 && strcmp(ops[i].op, "load") == 0)
				{
					for (j = 1; j < k + 1; j++)
					{
						if (livereg[j] == 0)
						{
							livereg[j] = ops[i].reg2;
							store2 = j;
							break;
						}
					}
					if (store2 == 0)
					{
						for (j = 1; j < k + 1; j++)
						{
							if (livemap[i][livereg[j]] == 0)
							{
								//found dead, register load active value into it

								livereg[j] = ops[i].reg2;
								store2 = j;
								break;
							}
						}
					}
					if (store2 == 0)
					{
						while (store1 != store2)
						{
							store2++;
						}
						j = findOff(0, offset);

						offset[j][1] = j * -4;
						offset[j][0] = livereg[store2];
						livereg[store2] = ops[i].reg2;
						printf("\tstoreAI\tr%d\t=>\tr0, %d\n", store2, offset[j][1]);
					}
				}
				if (strcmp(ops[i].op, "store") == 0)
					printf("\t%s\tr%d\t=>\tr%d\n", ops[i].op, store1, store2);
				else
					printf("\t%s\tr%d\t=>\tr%d\n", ops[i].op, store1, store2);
			} else if (strcmp(ops[i].op, "output") == 0)
			{
				printf("\t%s\t%d\n", ops[i].op, ops[i].reg1);
			}
			//check all registers if live, clear if not live	
		}
	}
}
int findSpill(int find, int spills[])
{
	int i = 1;
	while (spills[i] != find)
	{
		i++;
	}
	return i;
}
int findSpilled(int offset[], int find, int spilled)
{
	int i = 1;
	for (i = 1; i < spilled; i++)
	{
		if (offset[i] == find)
			return i;
	}
	return 0;
}
int findLive(int k, int find, int livereg[])
{
	int i = 1;
	for (i = 1; i < k - 1; i++)
	{
		if (livereg[i] == find)
			return i;
	}
	return 0;
}
void simpletop (int k, operator ops[])
{
	int occurences[256];
	int livereg[k + 1];
	int spills[256];
	int i = 0;
	int j = 0;
	int fill = 0;
	int count = 0;
	int spilled = 1;
	reg++;
	for (i = 0; i < reg; i++)
	{
		occurences[i] = 0;
	}
	for (i = 0; i < instruct; i++)
	{
		if (strcmp("loadI", ops[i].op) == 0)
		{
			if (ops[i].reg3 > 0)
				occurences[ops[i].reg3]++;
		}
		if (strcmp("addI", ops[i].op) == 0 || strcmp("multI", ops[i].op) == 0 
		 || strcmp("subI", ops[i].op) == 0 || strcmp("divI", ops[i].op) == 0 
		 || strcmp("lshiftI", ops[i].op) == 0 || strcmp("rshiftI", ops[i].op) == 0)
		{
			if (ops[i].reg1 > 0)
				occurences[ops[i].reg1]++;
			if (ops[i].reg3 > 0)
				occurences[ops[i].reg3]++;
		}
		if (strcmp("add", ops[i].op) == 0 || strcmp("mult", ops[i].op) == 0 
		 || strcmp("sub", ops[i].op) == 0 || strcmp("div", ops[i].op) == 0 
		 || strcmp("lshift", ops[i].op) == 0 || strcmp("rshift", ops[i].op) == 0)
		{
			if (ops[i].reg1 > 0)
				occurences[ops[i].reg1]++;
			if (ops[i].reg2 > 0)
				occurences[ops[i].reg2]++;
			if (ops[i].reg3 > 0)
				occurences[ops[i].reg3]++;
		}		
		if (strcmp("store", ops[i].op) == 0 || strcmp("load", ops[i].op) == 0)
		{
			if (ops[i].reg1 > 0)
				occurences[ops[i].reg1]++;
			if (ops[i].reg2 > 0)
				occurences[ops[i].reg2]++;
		}		
	}
	for (i = 0; i < reg; i++)
	{
		//printf("REG %d: %d\n", i, occurences[i]);
	}
	for (i = 1; i < 256; i++)
	{
		spills[i] = 0;
	}
	for (i = 0; i < (k + 1); i++)
	{
		livereg[i] = 0;
	}
	int largest;
	int location;
	int off = 0;
	for (i = 1; i < (k - 1); i++)
	{
		largest = 0;
		location = 0;
		for (j = 0; j < reg; j++)
		{
			if (largest < occurences[j])
			{
				largest = occurences[j];
				location = j;
			}
		}
		livereg[i] = location;
		occurences[location] = 0;
	}

	printf("\t%s\t%d\t=>\tr%d\n", ops[0].op, ops[0].reg1, ops[0].reg2);

	for (i = 1; i < instruct; i++)
	{
		if (strcmp(ops[i].op, "loadI") == 0)
		{
			if (location = findOcc(ops[i].reg2, livereg, k))
			{
				printf("\t%s\t%d\t=>\tr%d\n", ops[i].op, ops[i].reg1, location);
			} else
			{
				location = k - 1;
				printf("\t%s\t%d\t=>\tr%d\n", ops[i].op, ops[i].reg1, location);
				spills[spilled] = ops[i].reg2;
				off = spilled;
				spilled++;
				printf("\tstoreAI\tr%d\t=>\tr0, %d\n", location, off * -4);
				//spill immed.
			}
		} else if (strcmp(ops[i].op, "addI") == 0 || strcmp(ops[i].op, "multI") == 0 
				|| strcmp(ops[i].op, "subI") == 0 || strcmp(ops[i].op, "divI") == 0 
				|| strcmp(ops[i].op, "lshiftI") == 0 || strcmp(ops[i].op, "rshiftI") == 0)
		{
			int reg1 = 0;
			int outreg = 0;
			int reg1offset = 0;
			int outregoffset = 0;
			reg1 = findLive(k, ops[i].reg1, livereg);
			outreg = findLive(k, ops[i].reg3, livereg);

			if (reg1 == 0)
			{
				reg1 = k - 1;
				reg1offset = findSpilled(spills, ops[i].reg1, spilled);
				//printf("%d\n", reg1offset);
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", reg1offset * -4, reg1);
			}
			if (outreg == 0)
			{
				outreg = k;
				printf("\t%s\tr%d, %d\t=>\tr%d\n", ops[i].op, reg1, ops[i].reg2, outreg);
				outregoffset = findSpilled(spills, ops[i].reg3, spilled);
				if (outregoffset == 0)
				{
					outregoffset = spilled;
					spills[spilled] = ops[i].reg3;
					spilled++;
				}	
				printf("\tstoreAI\tr%d\t=>\tr0, %d\n", outreg, outregoffset * -4);
			} else
			{
				printf("\t%s\tr%d, r%d\t=>\tr%d\n", ops[i].op, reg1, ops[i].reg2, outreg);
			}

		} else if (strcmp(ops[i].op, "add") == 0 || strcmp(ops[i].op, "mult") == 0 
				|| strcmp(ops[i].op, "sub") == 0 || strcmp(ops[i].op, "div") == 0 
				|| strcmp(ops[i].op, "lshift") == 0 || strcmp(ops[i].op, "rshift") == 0)
		{
			int reg1 = 0;
			int reg2 = 0;
			int outreg = 0;
			int reg1offset = 0;
			int reg2offset = 0;
			int outregoffset = 0;
			reg1 = findLive(k, ops[i].reg1, livereg);
			reg2 = findLive(k, ops[i].reg2, livereg);
			outreg = findLive(k, ops[i].reg3, livereg);

			if (reg1 == 0)
			{
				reg1 = k - 1;
				reg1offset = findSpilled(spills, ops[i].reg1, spilled);
				//printf("%d\n", reg1offset);
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", reg1offset * -4, reg1);
			}
			if (reg2 == 0)
			{
				reg2 = k;
				reg2offset = findSpilled(spills, ops[i].reg2, spilled);
				//printf("%d\n", reg2offset);
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", reg2offset * -4, reg2);
			}
			if (outreg == 0)
			{
				outreg = k;
				printf("\t%s\tr%d, r%d\t=>\tr%d\n", ops[i].op, reg1, reg2, outreg);
				outregoffset = findSpilled(spills, ops[i].reg3, spilled);
				if (outregoffset == 0)
				{
					outregoffset = spilled;
					spills[spilled] = ops[i].reg3;
					spilled++;
				}	
				printf("\tstoreAI\tr%d\t=>\tr0, %d\n", outreg, outregoffset * -4);
			} else
			{
				printf("\t%s\tr%d, r%d\t=>\tr%d\n", ops[i].op, reg1, reg2, outreg);
			}
		} else if (strcmp(ops[i].op, "load") == 0)
		{
			int reg1 = 0;
			int reg2 = 0;
			int reg1offset = 0;
			int reg2offset = 0;
			reg1 = findLive(k, ops[i].reg1, livereg);
			reg2 = findLive(k, ops[i].reg2, livereg);
			
			if (reg1 == 0)
			{
				reg1 = k - 1;
				reg1offset = findSpilled(spills, ops[i].reg1, spilled);
				//printf("%d\n", reg1offset);
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", reg1offset * -4, reg1);
			}
			if (reg2 == 0)
			{
				reg2 = k;
			}
			printf("\t%s\tr%d\t=>\tr%d\n", ops[i].op, reg1, reg2);
			if (reg2 == k)
			{
				reg2offset = spilled;
				spills[spilled] = ops[i].reg2;
				spilled++;
				printf("\tstoreAI\tr%d\t=>\tr0, %d\n", reg2, reg2offset * -4);
			}
		} else if (strcmp(ops[i].op, "store") == 0)
		{
			int reg1 = 0;
			int reg2 = 0;
			int reg1offset = 0;
			int reg2offset = 0;
			reg1 = findLive(k, ops[i].reg1, livereg);
			reg2 = findLive(k, ops[i].reg2, livereg);
			
			if (reg1 == 0)
			{
				reg1 = k - 1;
				reg1offset = findSpilled(spills, ops[i].reg1, spilled);
				//printf("%d\n", reg1offset);
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", reg1offset * -4, reg1);
			}
			if (reg2 == 0)
			{
				reg2 = k;
				reg2offset = findSpilled(spills, ops[i].reg2, spilled);
				//printf("%d\n", reg2offset);
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", reg2offset * -4, reg2);
			}
			printf("\t%s\tr%d\t=>r%d\n", ops[i].op, reg1, reg2);
		} else if (strcmp(ops[i].op, "output") == 0)
		{
			printf("\t%s\t%d\n", ops[i].op, ops[i].reg1);
		}
	}
}
int findEmpty(int livemap[], int livereg[], int k)
{
	int i;
	for (i = 1; i < (k - 1); i++)
	{
		if (livereg[i] == 0)
		{
			return i;
		}
	}
	for (i = 1; i < (k - 1); i++)
	{
		if (livemap[livereg[i]] == 0)
		{
			return i;
		}
	}

	return 0;
}
void livetop (int k, operator ops[])
{

	int notdone = 1;
	int i = 0;
	int j = 0;


	int fill = 0;
	int smallest = 0;
	int largest = 0;
	reg++;
	int livemap[instruct + 1][reg];
	int count;
	int occurences[256];
	int livereg[k + 1];
	int spills[256];
	int spilled = 1;
	int tospill = 0;
	for (i = 0; i < k + 1; i++)
	{
		livereg[i] = 0;
	}
	for (i = 0; i < 256; i++)
	{
		spills[i] = 0;
	}
	for (i = 0; i < instruct + 1; i++)
	{	
		for (j = 0; j < reg; j++)
		{
			livemap[i][j] = 0;
		}
	}

	for (i = 1; i < reg; i++)
	{
		count = 0;
		for (j = instruct - 1; j >= 0; j--)
		{
			if (strcmp(ops[j].op, "loadI") == 0)
			{
				if (ops[j].reg2 == i)
				{
					count++;
					livemap[j][i] = count;
				}
			} else if (strcmp(ops[j].op, "addI") == 0 || strcmp(ops[j].op, "multI") == 0 
				|| strcmp(ops[j].op, "subI") == 0 || strcmp(ops[j].op, "divI") == 0 
				|| strcmp(ops[j].op, "lshiftI") == 0 || strcmp(ops[j].op, "rshiftI") == 0)
			{
				if (ops[j].reg1 == i || ops[j].reg3 == i)
				{
					count++;
					livemap[j][i] = count;
				}
			}else
			{
				if (ops[j].reg1 == i || ops[j].reg2 == i || ops[j].reg3 == i)
				{
					count++;
					livemap[j][i] = count;
				}
			}
		}
	}

	for (i = 0; i < reg; i++)
	{
		fill = 0;
		for (j = 0; j < instruct; j++)
		{
			if (livemap[j][i] > 1)
			{
				fill = 1;
				livemap[j][i] = 1;
			} else if (livemap[j][i] == 1)
			{
				fill = 0;
				livemap[j][i] = 0;
			} else if (fill == 1)
			{
				livemap[j][i] = 1;
			}
		}
	}

	for (i = 1; i < reg; i++)
	{
		count = 0;
		for (j = 1; j < instruct; j++)
		{
			if (livemap[j][i] == 1)
			{
				count++;
			}
		}
		livemap[instruct][i] = count;
	}
	int live = 0;
	for (i = 1; i < instruct; i++)
	{
		live = 0;
		for (j = 1; j < reg; j++)
		{
			if (livemap[i][j] == 1)
			{
				live++;
			}
		}
		livemap[i][0] = live;
	}
	/*
	for (i = 0; i < instruct + 1; i++)
	{

		for (j = 0; j < reg; j++)
		{
			printf("%d:::", livemap[i][j]);
		}
		printf("\n");
	}
	*/

	for (i = 0; i < 256; i++)
	{
		occurences[i] = 0;
	}

	for (i = 0; i < instruct; i++)
	{
		if (strcmp("loadI", ops[i].op) == 0)
		{
			if (ops[i].reg3 > 0)
				occurences[ops[i].reg3]++;
		}
		if (strcmp("addI", ops[i].op) == 0 || strcmp("multI", ops[i].op) == 0 
		 || strcmp("subI", ops[i].op) == 0 || strcmp("divI", ops[i].op) == 0 
		 || strcmp("lshiftI", ops[i].op) == 0 || strcmp("rshiftI", ops[i].op) == 0)
		{
			if (ops[i].reg1 > 0)
				occurences[ops[i].reg1]++;
			if (ops[i].reg3 > 0)
				occurences[ops[i].reg3]++;
		}
		if (strcmp("add", ops[i].op) == 0 || strcmp("mult", ops[i].op) == 0 
		 || strcmp("sub", ops[i].op) == 0 || strcmp("div", ops[i].op) == 0 
		 || strcmp("lshift", ops[i].op) == 0 || strcmp("rshift", ops[i].op) == 0)
		{
			if (ops[i].reg1 > 0)
				occurences[ops[i].reg1]++;
			if (ops[i].reg2 > 0)
				occurences[ops[i].reg2]++;
			if (ops[i].reg3 > 0)
				occurences[ops[i].reg3]++;
		}		
		if (strcmp("store", ops[i].op) == 0 || strcmp("load", ops[i].op) == 0)
		{
			if (ops[i].reg1 > 0)
				occurences[ops[i].reg1]++;
			if (ops[i].reg2 > 0)
				occurences[ops[i].reg2]++;
		}
	}



	/*
	for (i = 0; i < instruct; i++)
	{
		printf("MAXLIVE %d\n", livemap[i][0]);
	}
	*/
	while (notdone == 1)
	{
		notdone = 0;
		for (i = 1; i < instruct; i++)
		{
			if (livemap[i][0] > (k - 2))
			{
				notdone = 1;
				//printf("VIOLATION %d %d\n", livemap[i][0], (k - 2));
				int testlive[livemap[i][0]];
				for (j = 0; j < livemap[i][0]; j++)
				{
					testlive[j] = 0;
				}
				count = 0;
				smallest = 99;
				largest = 0;
				for (j = 1; j < reg; j++)
				{
					if (livemap[i][j] == 1)
					{
						//printf("OCCUR %d\n", occurences[j]);
						if (smallest > occurences[j])
							smallest = occurences[j];
					}
				}
				for (j = 1; j < reg; j++)
				{
					if (livemap[i][j] == 1)
					{
						if (smallest == occurences[j])
						{
							testlive[count] = j;
							count++;
						}
					}
				}

				if (count == 1)
				{
					spills[spilled] = testlive[0];
					//printf("SPILLINGss %d\n", testlive[0]);
					for (j = 0; j < instruct; j++)
					{
						if (livemap[j][testlive[0]] == 1)
						{
							livemap[j][testlive[0]] = 0;
							livemap[j][0]--;
						}
					}
					spilled++;
				} else
				{
					//printf("COUNTTTTTT %d\n", count);
					for (j = 0; j < count; j++)
					{
						//printf("Testing %d\n", livemap[instruct][testlive[j]]);
						if (largest < livemap[instruct][testlive[j]])
						{
							largest = livemap[instruct][testlive[j]];
							tospill = j;
						}
					}

					spills[spilled] = testlive[tospill];
					for (j = 0; j < instruct; j++)
					{
						if (livemap[j][testlive[tospill]] == 1)
						{
							livemap[j][testlive[tospill]] = 0;
							livemap[j][0]--;
						}
					}
					//printf("SPILLING aa%d\n", testlive[tospill]);
					spilled++;
				}
			}
		}
	}
	/*
	for (i = 0; i < 256; i++)
	{
		printf("%d : ", spills[i]);
	}
	printf("\n");
	*/
	printf("\t%s\t%d\t=>\tr%d\n", ops[0].op, ops[0].reg1, ops[0].reg2);
	for (i = 1; i < instruct; i++)
	{
		//printf("INSTRUCTION %d OP %s\n", i, ops[i].op);
		if (strcmp(ops[i].op, "loadI") == 0)
		{
			int outreg;
			outreg = findSpilled(spills, ops[i].reg2, spilled);
			if (outreg)
			{
				printf("\t%s\t%d\t=>\tr%d\n", ops[i].op, ops[i].reg1, (k - 1));
				printf("\tstoreAI\tr%d\t=>\tr0, %d\n", (k - 1), (outreg * -4));
			} else
			{
				//put in a register
				outreg = findEmpty(livemap[i], livereg, k);
				printf("\t%s\t%d\t=>\tr%d\n", ops[i].op, ops[i].reg1, outreg);
				livereg[outreg] = ops[i].reg2;
			}
		} else if (strcmp(ops[i].op, "addI") == 0 || strcmp(ops[i].op, "multI") == 0 
				|| strcmp(ops[i].op, "subI") == 0 || strcmp(ops[i].op, "divI") == 0 
				|| strcmp(ops[i].op, "lshiftI") == 0 || strcmp(ops[i].op, "rshiftI") == 0)
		{


		} else if (strcmp(ops[i].op, "add") == 0 || strcmp(ops[i].op, "mult") == 0 
				|| strcmp(ops[i].op, "sub") == 0 || strcmp(ops[i].op, "div") == 0 
				|| strcmp(ops[i].op, "lshift") == 0 || strcmp(ops[i].op, "rshift") == 0)
		{
			//printf("ADD FOUND\n");
			int reg1;
			int reg2;
			int reg3;
			reg1 = findSpilled(spills, ops[i].reg1, spilled);
			reg2 = findSpilled(spills, ops[i].reg2, spilled);
			reg3 = findSpilled(spills, ops[i].reg3, spilled);

			if (reg1 != 0)
			{
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", (reg1 * -4), (k - 1));
				reg1 = k - 1; 	
			} else
			{
				reg1 = findLive(k, ops[i].reg1, livereg);
			}
			if (reg2 != 0)
			{
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", (reg2 * -4), k);
				reg2 = k; 	
			} else
			{
				reg2 = findLive(k, ops[i].reg2, livereg);
			}
			if (reg3 != 0)
			{
				printf("\t%s\tr%d, r%d\t=>\tr%d\n", ops[i].op, reg1, reg2, k);
				printf("\tstoreAI\tr%d=>\tr0, %d\n", k, reg3 * - 4);
			} else
			{
				reg3 = findEmpty(livemap[i], livereg, k);
				livereg[reg3] = ops[i].reg3;
				printf("\t%s\tr%d, r%d\t=>\tr%d\n", ops[i].op, reg1, reg2, reg3);
			}
		} else if (strcmp(ops[i].op, "load") == 0)
		{
			int reg1;
			int reg2;

			reg1 = findSpilled(spills, ops[i].reg1, spilled);
			reg2 = findSpilled(spills, ops[i].reg2, spilled);

			if (reg1)
			{
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", (reg1 * -4), (k - 1));
				reg1 = k - 1; 	
			} else
			{
				reg1 = findLive(k, ops[i].reg1, livereg);
			}

			if (reg2)
			{
				printf("\t%s\tr%d\t=>\tr%d\n", ops[i].op, reg1, k);
				printf("\tstoreAI\tr%d=>\tr0, %d\n", k, reg2 * - 4);
			} else
			{
				reg2 = findEmpty(livemap[i], livereg, k);
				livereg[reg2] = ops[i].reg2;
				printf("\t%s\tr%d\t=>\tr%d\n", ops[i].op, reg1, reg2);
			}
			
		} else if (strcmp(ops[i].op, "store") == 0)
		{
			int reg1;
			int reg2;

			reg1 = findSpilled(spills, ops[i].reg1, spilled);
			reg2 = findSpilled(spills, ops[i].reg2, spilled);

			if (reg1)
			{
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", (reg1 * -4), (k - 1));
				reg1 = k - 1; 	
			} else
			{
				reg1 = findLive(k, ops[i].reg1, livereg);
			}
			if (reg2)
			{
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", (reg2 * -4), k);
				reg2 = k; 
				printf("\t%s\tr%d\t=>\tr%d\n", ops[i].op, reg1, k);
				//printf("\tstoreAI\tr%d=>\tr0, %d\n", k, reg2 * - 4);	
			} else
			{
				reg2 = findLive(k, ops[i].reg2, livereg);
				printf("\t%s\tr%d\t=>\tr%d\n", ops[i].op, reg1, reg2);
			}


		} else if (strcmp(ops[i].op, "output") == 0)
		{
			printf("\t%s\t%d\n", ops[i].op, ops[i].reg1);
		}
	}
}
int findHighest(int livemap[], int live[], int k)
{
	int i = 0;
	int test = 1;
	int largest = 0;
	for (i = 1; i < k - 1; i++)
	{
		if (largest < livemap[live[i]])
		{
			largest = livemap[live[i]];
			test = i;
		}

	}
	return test;
}
int findOn(int k, int find, int livereg[])
{
	int i = 1;
	for (i = 1; i < k - 1; i++)
	{
		if (livereg[i] == find)
			return i;
	}
	return 0;
}
void bup (int k, operator ops[])
{
	int i = 0;
	int j = 0;
	int count = 0;
	int fill = 0;
	int livereg[k + 1];
	for (i = 0; i < k + 1; i++)
	{
		livereg[i] = 0;
	}
	reg++;
	int livemap[instruct + 1][reg];
	int spilled = 1;
	int spills[256];
	for (i = 0; i < instruct; i++)
	{
		for (j = 0; j < reg; j++)
		{
			livemap[i][j] = 0;
		}
	}
	for (i = 0; i < 256; i++)
	{
		spills[i] = 0;
	}
	for (i = 1; i < reg; i++)
	{
		count = 0;
		for (j = instruct - 1; j >= 0; j--)
		{
			if (strcmp(ops[j].op, "loadI") == 0)
			{
				if (ops[j].reg2 == i)
				{
					count++;
					livemap[j][i] = count;
				}
			} else if (strcmp(ops[j].op, "addI") == 0 || strcmp(ops[j].op, "multI") == 0 
				|| strcmp(ops[j].op, "subI") == 0 || strcmp(ops[j].op, "divI") == 0 
				|| strcmp(ops[j].op, "lshiftI") == 0 || strcmp(ops[j].op, "rshiftI") == 0)
			{
				if (ops[j].reg1 == i || ops[j].reg3 == i)
				{
					count++;
					livemap[j][i] = count;
				}
			}else
			{
				if (ops[j].reg1 == i || ops[j].reg2 == i || ops[j].reg3 == i)
				{
					count++;
					livemap[j][i] = count;
				}
			}
		}
	}

	for (i = 0; i < reg; i++)
	{
		fill = 0;
		for (j = 0; j < instruct; j++)
		{
			if (livemap[j][i] > 1)
			{
				fill = 1;
				livemap[j][i] = 1;
			} else if (livemap[j][i] == 1)
			{
				fill = 0;
				livemap[j][i] = 0;
			} else if (fill == 1)
			{
				livemap[j][i] = 1;
			}
		}
	}
	for (i = 1; i < reg; i++)
	{
		count = 0;
		for (j = 1; j < instruct; j++)
		{
			if (livemap[j][i] == 1)
			{
				count++;
			}
		}
		livemap[instruct][i] = count;
	}

	printf("\t%s\t%d\t=>\tr%d\n", ops[0].op, ops[0].reg1, ops[0].reg2);
	for (i = 1; i < instruct; i++)
	{
		if (strcmp(ops[i].op, "loadI") == 0)
		{
			int outreg;
			int livev = 0;
			outreg = findEmpty(livemap[i], livereg, k);
			if (outreg == 0)
			{
				//spill a register
				outreg = findHighest(livemap[instruct], livereg, k);
				spills[spilled] = livereg[outreg];

				printf("\tstoreAI\tr%d\t=>\t\tr0, %d\n", outreg, spilled * -4);
				spilled++;
			}
			livereg[outreg] = ops[i].reg2;
			printf("\t%s\t%d\t=>\tr%d\n", ops[i].op, ops[i].reg1, outreg);
		} else if (strcmp(ops[i].op, "addI") == 0 || strcmp(ops[i].op, "multI") == 0 
				|| strcmp(ops[i].op, "subI") == 0 || strcmp(ops[i].op, "divI") == 0 
				|| strcmp(ops[i].op, "lshiftI") == 0 || strcmp(ops[i].op, "rshiftI") == 0)
		{


		} else if (strcmp(ops[i].op, "add") == 0 || strcmp(ops[i].op, "mult") == 0 
				|| strcmp(ops[i].op, "sub") == 0 || strcmp(ops[i].op, "div") == 0 
				|| strcmp(ops[i].op, "lshift") == 0 || strcmp(ops[i].op, "rshift") == 0)
		{
			int reg1;
			int reg2;
			int reg3;
			int livev = 0;
			reg1 = findOn(k, ops[i].reg1, livereg);
			reg2 = findOn(k, ops[i].reg2, livereg);
			reg3 = findEmpty(livemap[i], livereg, k);
			int reg3offset;

			if (reg1 == 0)
			{
				reg1 = findSpilled(spills, ops[i].reg1, spilled);
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", (reg1 * -4), (k - 1));
				reg1 = k - 1; 	
			} 
			if (reg2 == 0)
			{
				reg2 = findSpilled(spills, ops[i].reg2, spilled);
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", (reg2 * -4), k);
				reg2 = k; 	
			}
			if (reg3 == 0)
			{
				reg3 = findHighest(livemap[instruct], livereg, k);
				reg3offset = findSpilled(spills, livereg[reg3], spilled);
				//livev = findLive(k, reg3, livereg);
				if (reg3offset == 0)
				{
					spills[spilled] = livereg[reg3];
					reg3offset = spilled;
					printf("\tstoreAI\tr%d=>\tr0, %d\n", reg3, reg3offset * - 4);
					spilled++;
				} else
				{
					printf("\tstoreAI\tr%d=>\tr0, %d\n", reg3, reg3offset * - 4);
				}

				printf("\t%s\tr%d, r%d\t=>\tr%d\n", ops[i].op, reg1, reg2, reg3);
				livereg[reg3] = ops[i].reg3;
			} else
			{
				livereg[reg3] = ops[i].reg3;
				printf("\t%s\tr%d, r%d\t=>\tr%d\n", ops[i].op, reg1, reg2, reg3);
			}
		} else if (strcmp(ops[i].op, "load") == 0)
		{
			int reg1;
			int reg2;
			int reg2offset = 0;
			int livev = 0;

			reg1 = findOn(k, ops[i].reg1, livereg);
			reg2 = findEmpty(livemap[i], livereg, k);

			if (reg1 == 0)
			{
				reg1 = findSpilled(spills, ops[i].reg1, spilled);
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", (reg1 * -4), (k - 1));
				reg1 = k - 1; 	
			} 

			if (reg2 == 0)
			{
				reg2 = findHighest(livemap[instruct], livereg, k);
				reg2offset = findSpilled(spills, livereg[reg2], spilled);
				//livev = findLive(k, reg2, livereg);
				if (reg2offset == 0)
				{
					spills[spilled] = livereg[reg2];
					reg2offset = spilled;
					printf("\tstoreAI\tr%d=>\tr0, %d\n", reg2, reg2offset * - 4);
					spilled++;
				} else
				{
					printf("\tstoreAI\tr%d=>\tr0, %d\n", reg2, reg2offset * - 4);
				}

				printf("\t%s\tr%d\t=>\tr%d\n", ops[i].op, reg1, reg2);
				livereg[reg2] = ops[i].reg2;
			} else
			{
				livereg[reg2] = ops[i].reg2;
				printf("\t%s\tr%d\t=>\tr%d\n", ops[i].op, reg1, reg2);
			}
			
		} else if (strcmp(ops[i].op, "store") == 0)
		{
			int reg1;
			int reg2;

			reg1 = findOn(k, ops[i].reg1, livereg);
			reg2 = findOn(k, ops[i].reg2, livereg);

			if (reg1 == 0)
			{
				reg1 = findSpilled(spills, ops[i].reg1, spilled);
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", (reg1 * -4), (k - 1));
				reg1 = k - 1; 	
			}
			if (reg2 == 0)
			{
				reg2 = findSpilled(spills, ops[i].reg2, spilled);
				printf("\tloadAI\tr0, %d\t=>\tr%d\n", (reg2 * -4), k);
				reg2 = k; 
			} 
			printf("\t%s\tr%d\t=>\tr%d\n", ops[i].op, reg1, reg2);


		} else if (strcmp(ops[i].op, "output") == 0)
		{
			printf("\t%s\t%d\n", ops[i].op, ops[i].reg1);
		}
		
	}
}
int main (int argc, char *argv[])
{
	int k;
	char flag;
	char* inputfile;

	k = atoi(argv[1]);

	flag = argv[2][0];
	inputfile = argv[3];

	FILE *fp;
	fp = fopen(inputfile, "r");
	char iloc[100][100];
	int test;
	int i = 0;
	int j = 0;
	char holder[200];
	char holder1[200];
	char holder2[200];
	char *found;
	char *foundhold;
	operator ops[1000];
	row = 0;
	while (fgets(holder, sizeof(holder), fp))
	{
		if (holder[0] != '\n' && holder[0] != '/')
		{
			for (i = 0; i < sizeof(holder); i++)
			{
				if (holder[i] == '\t')
					holder[i] = ' ';
			}
			//printf("%s", holder);
			instruct++;
			ops[row].reg3 = -1;
			found = strtok(holder, " ");
			foundhold = strdup(found);
			ops[row].op = foundhold;
			if (strcmp(ops[row].op, "loadI") == 0)
			{
				found = strtok(NULL, " ");
				ops[row].reg1 = atoi(found);
				found = strtok(NULL, " ");
				found = strtok(NULL, " ");
				foundhold = strdup(found);
				ops[row].reg2 = atoi(strip(foundhold));
				if (reg < ops[row].reg2)
					reg = ops[row].reg2;
			}

			else if (strcmp(ops[row].op, "load") == 0 || strcmp(ops[row].op, "store") == 0)
			{
				found = strtok(NULL, " ");
				foundhold = strdup(found);
				ops[row].reg1 = atoi(strip(foundhold));
				//free(foundhold);
				found = strtok(NULL, " ");
				found = strtok(NULL, " ");
				foundhold = strdup(found);
				ops[row].reg2 = atoi(strip(foundhold));
				if (strcmp(ops[row].op, "load") == 0)
				{
					if (reg < ops[row].reg2)
						reg = ops[row].reg2;
				}
			} else if (strcmp(ops[row].op, "add") == 0 || strcmp(ops[row].op, "sub") == 0 
				|| strcmp(ops[row].op, "mult") == 0 || strcmp(ops[row].op, "div") == 0
				|| strcmp(ops[row].op, "lshift") == 0 || strcmp(ops[row].op, "rshift") == 0)
			{
				found = strtok(NULL, " ");
				foundhold = strdup(found);
				ops[row].reg1 = atoi(strip(foundhold));
				//free(foundhold);
				found = strtok(NULL, " ");
				foundhold = strdup(found);
				ops[row].reg2 = atoi(strip(foundhold));
				//free(foundhold);
				found = strtok(NULL, " ");
				found = strtok(NULL, " ");
				foundhold = strdup(found);
				ops[row].reg3 = atoi(strip(foundhold));
				if (reg < ops[row].reg3)
					reg = ops[row].reg3;

			} else if (strcmp(ops[row].op, "addI") == 0 || strcmp(ops[row].op, "subI") == 0 
				|| strcmp(ops[row].op, "multI") == 0 || strcmp(ops[row].op, "divI") == 0
				|| strcmp(ops[row].op, "lshiftI") == 0 || strcmp(ops[row].op, "rshiftI") == 0)
			{
				found = strtok(NULL, " ");
				foundhold = strdup(found);
				ops[row].reg1 = atoi(strip(foundhold));
				//free(foundhold);
				found = strtok(NULL, " ");
				ops[row].reg2 = atoi(found);
				found = strtok(NULL, " ");
				found = strtok(NULL, " ");
				ops[row].reg3 = atoi(strip(found));
				if (reg < ops[row].reg3)
					reg = ops[row].reg3;
			} else if (strcmp(ops[row].op, "output") == 0)
			{
				found = strtok(NULL, " ");
				ops[row].reg1 = atoi(found);
			}
			row++;
		}
	}

	if (flag == 'b')
	{
		bup(k, ops);
	} else if (flag == 's')
	{
		simpletop(k, ops);
	} else if (flag == 't')
	{
		livetop(k, ops);
	}
}

