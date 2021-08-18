
#define GetCurrentDir _getcwd
#include "headers.h"
 void runInstance(char* filename, int sequence);



 int add(int i, int j) {
	return i + j;
}



 float dll00Func00(float** ppArr) {
	return ppArr[5][5];
}


 void runInstNew(int* sequence, double** travelCost, int n_stops) {
	clock_t  start, end;
	double opt_value;
	double cputime;
	n_conn_comp = 0;
	old_objval = 0;
	count_same_node = 0;
	n_int_feas_subproblems = 0;
	n_frac_feas_subproblems = 0;
	n_feas_subproblems = 0;

	/*
	printf("travel cost is %f",travelCost[1][2]);
	printf("N is %d", n_stops);*/
	initialize_Instance_Amazon_New(travelCost, n_stops);
	////start = clock();
	/*printf("Solving TSP with B&C algorithm: \n");*/

	opt_value = solve_TSP(sequence);
	//printf("Optimal travel time is :  %f \n", opt_value);
	end = clock();
	//cputime = (double)(end - start) / (double)CLOCKS_PER_SEC;   //Compute CPU time

	free_memory();
}



