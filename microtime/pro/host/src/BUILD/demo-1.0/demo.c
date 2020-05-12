// --------------------------------------------------------------------
//
//   Title     :  MAIN
//             :
//   Library   :
//             :
//   Developers:  MICROTIME MDS group
//             :
//   Purpose   :
//             :
//   Limitation:
//             :
//   Note      :
//             :
// --------------------------------------------------------------------
//   modification history :
// --------------------------------------------------------------------
//   Version| mod. date: |
//   V1.0   | 04/15/2006 | First release
// --------------------------------------------------------------------
//
// Note:
//
//       MICROTIME COMPUTER INC.
//
// --------------------------------------------------------------------


/*************************************************************************
Include files
*************************************************************************/
#include <stdio.h>

typedef struct 
{
	int iVar_a;
	char cVar_b;
	long lVar_c;
}StructType;

int g_iVar_a;
char g_cVar_b;
long g_lVar_c;
StructType g_struct;

/***************************************************/
void test_BreakPoint(void)
{
	int i, j;
	printf("enter test_BreakPoint()\n\r");

	j=0;
	for(i=0; i<10; i++)
	{
		j+=i;
		printf("j=%d\n\r", j);
	}
	printf("\n\r");
}
/***************************************************/
void test_RunStep_1(void)
{
	printf("enter test_RunStept_1()\n\r");
}
void test_RunStep(void)
{
	int i;
	printf("enter test_Runstep()\n\r");
	test_RunStep_1();
	printf("\n\r");
}

/***************************************************/
void test_WatchList(void)
{
	int iVar_a;
	char cVar_b;
	long lVar_c;

	printf("enter test_WatchList()\n\r");

	iVar_a=1;
	cVar_b='a';
	lVar_c=2;
	printf("iVar_a=%d, cVar_b=%c, lVar_c=%d\n\r",iVar_a, cVar_b, lVar_c);

	g_iVar_a=iVar_a*2;
	g_cVar_b='b';
	g_lVar_c=lVar_c*2;
	printf("g_iVar_a=%d, g_cVar_b=%c, g_lVar_c=%d\n\r",g_iVar_a, g_cVar_b, g_lVar_c);
	
	g_struct.iVar_a=iVar_a*3;
	g_struct.cVar_b='c';
	g_struct.lVar_c=lVar_c*3;
	printf("g_struct.iVar_a=%d, g_struct.cVar_b=%c, g_struct.lVar_c=%d\n\r",g_struct.iVar_a, g_struct.cVar_b, g_struct.lVar_c);

	printf("\n\r");
}

/***************************************************/
void test_MemoryView(void)
{
	int i;

	printf("enter test_MemoryView()\n\r");
	for(i=0; i<10; i++)
	{
		printf("timnes=%d\n\r", i);
	}
	printf("\n\r");
}


/*************************************************************************
MAIN Program 
*************************************************************************/

int main(int argc, char *argv[])
{

 	printf("enter main()\n\r");

	test_BreakPoint();
	test_RunStep();
	test_WatchList();
	test_MemoryView();
	
    return(0);
}
