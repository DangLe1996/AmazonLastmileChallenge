#include "headers.h"
// #include "Score.h"

double Heuristic_Clustered_ATSP(int depot, int dim_N, int dim_Z, double** c, int* zone_dim, int* list_all_stops, int* final_sequence){

	int i, j, a, z, Z, count_stops, count_added, best_stop, flag;
	double min_cost;
	int *visited;
	int* pos_zone;
	ZONEID* zoneid;
	SOLUTION solution;

	solution.N = dim_N;
	solution.A = solution.N * (solution.N - 1);
	solution.depot = depot;
	Z = dim_Z;
	solution.value = 0;

	solution.sequence = create_int_vector(solution.N + 1);
	solution.a = (ARC*)calloc(solution.N, sizeof(ARC));
	solution.predecessor = create_int_vector(solution.N);
	solution.successor = create_int_vector(solution.N);
	solution.index_a = create_int_matrix(solution.N, solution.N);
	solution.index_i = create_int_vector(solution.A);
	solution.index_j = create_int_vector(solution.A);
	solution.x_best = create_int_vector(solution.A);
	visited = create_int_vector(solution.N);
	pos_zone = create_int_vector(Z + 1);
	zoneid = (ZONEID*)calloc(Z + 1, sizeof(ZONEID));
	for (z = 0; z < Z + 1; z++)
		zoneid[z].stops = create_int_vector(solution.N);

	count_stops = 0;
	for (z = 0; z < Z + 1; z++) {
		pos_zone[z] = count_stops;
		zoneid[z].dim = zone_dim[z]; //get is as argument
		//sec_zone[z] = route_HQ[s].sec_zone_obs[z];
		for (i = 0; i < zoneid[z].dim; i++)
			zoneid[z].stops[i] = list_all_stops[count_stops++]; //get is as argument
	}

	for (i = 0; i < solution.N; i++)
		solution.index_a[i][i] = NONE;
	a = 0;
	for (i = 0; i < solution.N; i++) {
		for (j = 0; j < solution.N; j++) {
			if (i != j) {
				solution.index_i[a] = i;
				solution.index_j[a] = j;
				solution.index_a[i][j] = a;
				solution.x_best[a++] = 0;
			}
		}
	}

	//Contruct initial sequence of stops using a greedy heuristic (for each zoneid, use cheapest insertion)
	count_stops = 0;
	//solution.sequence[count_stops++] = depot;
	for (z = 0; z < Z + 1; z++) {
		if (zoneid[z].dim == 1) {
			solution.sequence[count_stops++] = zoneid[z].stops[0];
		}
		else{
			for (i = 0; i < zoneid[z].dim; i++)
				visited[i] = 0;
			count_added = 0;
			do {
				min_cost = 1000000;
				best_stop = -1;
				for (i = 0; i < zoneid[z].dim; i++) {
					if (visited[i] == 0 && c[solution.sequence[count_stops - 1]][zoneid[z].stops[i]] < min_cost) {
						best_stop = i;
						min_cost = c[solution.sequence[count_stops - 1]][zoneid[z].stops[i]];
					}
				}
				visited[best_stop] = 1;
				solution.sequence[count_stops++] = zoneid[z].stops[best_stop];
				count_added++;
			} while (count_added < zoneid[z].dim);
		}
	}
	//solution.sequence[solution.N] = depot;
	for (i = 0; i < solution.N; i++)
		solution.x_best[solution.index_a[solution.sequence[i]][solution.sequence[i + 1]]] = 1;
	Populate_Solution_Structue(solution, c, &solution.value);

	//Apply a local search where for each zoneid, we perform as many 3-changes and 4-changes as long as we keep improving
	for (z = 1; z < Z; z++) {
		if (zoneid[z].dim > 1) {
			flag = 1;
			while (flag) {
				flag = Triple_Change(z, solution, c, pos_zone);
				/*if (flag == 0) {
					flag = Quad_Change(solution, D, c);
				}*/
			}
		}
	}

	for (i = 0; i < solution.N + 1; i++)
		final_sequence[i] = solution.sequence[i];
	solution.value = 0;
	for (i = 0; i < solution.N - 1; i++)
		solution.value += c[final_sequence[i]][final_sequence[i + 1]];
	solution.value += c[final_sequence[solution.N - 1]][final_sequence[0]];


	for (z = 0; z < Z + 1; z++)
		free(zoneid[z].stops);
	free(zoneid);
	free(visited);
	free(pos_zone);
	free(solution.sequence);
	free(solution.predecessor);
	free(solution.successor);
	free(solution.a);
	for (i = 0; i < solution.N; i++)
		free(solution.index_a[i]);
	free(solution.index_a);
	free(solution.index_i);
	free(solution.index_j);
	free(solution.x_best);

	return solution.value;;

}



int Triple_Change(int z, SOLUTION solution, double** c, int *pos_zone) {

	int i, j, a, flag, terminate, selected;
	int k, l, q, p, w, r, best_k, best_l, best_q, best_p, best_r, best_w;
	double best_gain, gain;

	//Each 3-change is encoded as
	//x_1 = (k,l) we need to select it
    //x_2 = (p,q) we need to select it
	//x_3 = (w,r) we need to select it
	//y_1 = (k,q) uniquely determined by x_1 and x_2
	//y_2 = (p,r) uniquely determined by x_3 and x_2
	//y_3 = (w,l) uniquely determined by x_3 and x_1
	flag = 0;
	best_gain = 0;
	for (i = pos_zone[z]-1; i < pos_zone[z + 1] - 2; i++) {
		k = solution.sequence[i];
		l = solution.sequence[i + 1];
		for (j = i + 1; j < pos_zone[z + 1] - 1; j++) {
			p = solution.sequence[j];
			q = solution.sequence[j + 1];
			for (a = j + 1; a < pos_zone[z + 1]; a++) {
				w = solution.sequence[a];
				r = solution.sequence[a + 1];
				gain = (c[k][l] + c[p][q] + c[w][r] - c[k][q] - c[p][r] - c[w][l]);
				//Store the most favorable 3-change move found for zone z
				if (gain > best_gain) {
					best_gain = gain;
					best_k = k;
					best_l = l;
					best_p = p;
					best_q = q;
					best_w = w;
					best_r = r;
				}
			}
		}
	}
	//If an improved tour is found, update the current solution and stop (first improvement strategy)
	if (best_gain > 0.01) {
		Update_Current_Solution(solution, c, best_k, best_l, best_p, best_q, best_w, best_r);
		flag = 1;
	}
	return flag;
}


double Lin_Kernighan_Heuristic_ATSP(int V, double** c, int* initial_sequence) {

	int i, j, a, flag;
	ORDROWS* D;
	SOLUTION solution;

	//Memory allocation, presort of costs, and initialization of solution structure
	solution.N = V;
	solution.A = solution.N * (solution.N - 1);
	solution.value = 0;
	D = (ORDROWS*)calloc(solution.N, sizeof(ORDROWS));
	for (i = 0; i < solution.N; i++)
		D[i].A = (ORDCOSTS*)calloc(solution.N, sizeof(ORDCOSTS));
	for (i = 0; i < solution.N; i++) {
		for (j = 0; j < solution.N; j++) {
			D[i].A[j].c = c[i][j];
			D[i].A[j].j = j;
		}
		qsort((ORDCOSTS*)D[i].A, solution.N, sizeof(D[i].A[0]), Compare_Cost_Value);
	}
	solution.a = (ARC*)calloc(solution.N, sizeof(ARC));
	solution.sequence = create_int_vector(solution.N + 1);
	solution.predecessor = create_int_vector(solution.N);
	solution.successor = create_int_vector(solution.N);
	solution.index_a = create_int_matrix(solution.N, solution.N);
	solution.index_i = create_int_vector(solution.A);
	solution.index_j = create_int_vector(solution.A);
	solution.x_best = create_int_vector(solution.A);
	
	for (i = 0; i < solution.N; i++)
		solution.index_a[i][i] = NONE;
	a = 0;
	for (i = 0; i < solution.N; i++) {
		for (j = 0; j < solution.N; j++) {
			if (i != j) {
				solution.index_i[a] = i;
				solution.index_j[a] = j;
				solution.index_a[i][j] = a;
				solution.x_best[a++] = 0;
			}
		}
	}
	for (i = 0; i < solution.N+1; i++)
		solution.sequence[i] = initial_sequence[i];
	solution.depot = solution.sequence[0];
	//for (a = 0; a < solution.A; a++)
	//	solution.x_best[a] = 0;
	for (i = 0; i < solution.N; i++) //There is a difference with respect to line 103 on read_data.c
		solution.x_best[solution.index_a[solution.sequence[i]][solution.sequence[i + 1]]] = 1;
	
	Populate_Solution_Structue(solution, c, &solution.value);

	/*printf("Initial sequence with value: %0.2f \n", solution.value);
	for (i = 0; i < N + 1; i++)
		printf("%d ", solution.sequence[i]);
	printf("\n");*/


	//Main loop of Local Search
	flag = 1;
	while (flag) {
		flag = Sequential_Primary_Change(solution, D, c);
		/*if (flag == 0) {
			flag = Quad_Change(solution, D, c);
		}*/
	}

	//memcpy(initial_sequence, solution.sequence, solution.N + 1);
	for (i = 0; i < solution.N+1; i++)
		initial_sequence[i] = solution.sequence[i];
	solution.value = 0;
	for (i = 0; i < solution.N - 1; i++)
		solution.value += c[initial_sequence[i]][initial_sequence[i + 1]];
	solution.value += c[initial_sequence[solution.N - 1]][initial_sequence[0]];

	free(D);
	free(solution.sequence);
	free(solution.predecessor);
	free(solution.successor);
	free(solution.a);
	for (i = 0; i < solution.N; i++)
		free(solution.index_a[i]);
	free(solution.index_a);
	free(solution.index_i);
	free(solution.index_j);
	free(solution.x_best);

	return solution.value;

}

//
//int Quad_Change(SOLUTION solution, ORDROWS* D, double** c) {
//
//	int i, i_star, j, a, flag, terminate, selected;
//	int k, l, q, p, w, r, v, z, best_k, best_p, best_w, best_v;
//	double value1, value2, best_value;
//	QUAD *Y;
//
//	flag = 0;
//	best_value = 0;
//	for (k = 0; k < solution.N; k++) {
//		l = k + 1;
//		for (p = k + 4; p < k - 2; p++) {
//			q = p + 1;
//			value1 = (c[solution.sequence[k]][solution.sequence[l]] + c[solution.sequence[p]][solution.sequence[q]] - c[solution.sequence[k]][solution.sequence[q]] - c[solution.sequence[l]][solution.sequence[p]]);
//			if (value1 > 0) {
//				for (w = q + 1; w < k - 2; w++) {
//					for (v = l + 1; v < p - 1; v++) {
//						value2 = (c[solution.sequence[w]][solution.sequence[r]] + c[solution.sequence[v]][solution.sequence[z]] - c[solution.sequence[v]][solution.sequence[r]] - c[solution.sequence[w]][solution.sequence[z]]);
//						if (value1 + value2 > best_value) {
//							best_value = value1 + value2;
//							best_k = k;
//							best_p = p;
//							best_w = w;
//							best_v = v;
//						}
//					}
//				}
//			}
//		}
//	}
//
//	if (best_value > 0.01) {
//		Update_Current_Solution_Quad(solution, c, k, l, best_k, best_p, best_w, best_v);
//		flag = 1;
//	}
//
//	return flag;
//
//}

int Sequential_Primary_Change(SOLUTION solution, ORDROWS* D, double** c) {

	int i, i_star, j, a, flag, terminate, selected;
	int k, l, q, p, w, r, best_q, best_p, best_r, best_w;
	double G_star, G_tau_star, best_G_tau_star, SP;
	double* G;
	/*int** X;
	int** Y;
	int* x;
	int* y;*/
	int y_star;

	/*X = create_int_matrix(N, N);
	Y = create_int_matrix(N, N);
	x = create_int_vector(N);
	y = create_int_vector(N);*/
	G = create_double_vector(solution.N);

	//Each 3-change is encoded as
	//x_1 = (k,l) given as an input at every iteration (either from the initial primary change or previous primary change)
	//y_1 = (k,q) we need to select it
	//x_2 = (p,q) uniquely determined by y_1
	//y_2 = (p,r) we need to select it
	//x_3 = (w,r) uniquely determined by y_2
	//y_star = (w,l) uniquely determined by x_3 and x_1
	flag = 0;
	//For each ordered arc a in the current solution, try to find a favorable sequential primary change starting with x1 = a
	for (a = 0; a < solution.N; a++) {
		//Step 1: Initialization
		best_G_tau_star = 0;
		G[0] = 0;
		G_star = 0;
		i_star = 0;
		i = 1;
		k = solution.a[a].tail;
		l = solution.a[a].head;
		//Step 2.1: Find best y_1
		for (j = 0; j < solution.N; j++) {
			if (D[k].A[j].j != k) {
				q = D[k].A[j].j;
				if (q != l && G[i - 1] + c[k][l] - c[k][q] > 0) {
					p = solution.predecessor[q];
					r = solution.successor[q]; // begining of cycle
					terminate = 0;
					//Step 2.2: Find y_2 to break cycle and generate feasible solution by removing x_3 and adding y_star
					do {
						G_tau_star = (c[k][l] + c[p][q] + c[solution.predecessor[r]][r] - c[k][q] - c[p][r] - c[solution.predecessor[r]][l]);
						//Store the most favorable 3-change move found for x1 = a
						if (G_tau_star > best_G_tau_star) {
							best_G_tau_star = G_tau_star;
							best_r = r;
							best_w = solution.predecessor[r];
							best_q = q;
							best_p = p;
						}
						if (r == k)
							terminate = 1;
						else
							r = solution.successor[r];
					} while (terminate != 1); //end of cycle 
				}
			}
		}
		//If an improved tour is found, update the current solution and stop (first improvement strategy)
		if (best_G_tau_star > 0.01) {
			Update_Current_Solution(solution, c, k, l, best_p, best_q, best_w, best_r);
			flag = 1;
			break;
		}
	}

	/*for (k = 0; k < N; k++) {
		free(X[k]);
		free(Y[k]);
	}
	free(X);
	free(Y);
	free(x);
	free(y);*/
	free(G);

	return flag;
}

void Update_Current_Solution_Quad(SOLUTION solution, double** c, int k, int p, int w, int v) {

	int i, j, t;

	//Remove x_1, x_2, x_3, and x_4 arcs
	solution.x_best[solution.index_a[solution.sequence[k]][solution.sequence[k + 1]]] = 0;
	solution.x_best[solution.index_a[solution.sequence[p]][solution.sequence[p + 1]]] = 0;
	solution.x_best[solution.index_a[solution.sequence[w]][solution.sequence[w + 1]]] = 0;
	solution.x_best[solution.index_a[solution.sequence[v]][solution.sequence[v + 1]]] = 0;
	//Add y_1, y_2, y_3, and y_4 arcs
	solution.x_best[solution.index_a[solution.sequence[k]][solution.sequence[p + 1]]] = 1;
	solution.x_best[solution.index_a[solution.sequence[p]][solution.sequence[k + 1]]] = 1;
	solution.x_best[solution.index_a[solution.sequence[v]][solution.sequence[w + 1]]] = 1;
	solution.x_best[solution.index_a[solution.sequence[w]][solution.sequence[v + 1]]] = 1;

	//Update sequence
	for (i = 0; i < solution.N; i++)
		solution.sequence[i] = -1;
	t = 0;
	solution.sequence[t++] = solution.depot;
	//printf("%d, ", route_HQ[s].sec_opt[t-1]);
	while (t < solution.N + 1) {
		for (j = 0; j < solution.N; j++) {
			if (solution.sequence[t - 1] != j && solution.x_best[solution.index_a[solution.sequence[t - 1]][j]] == 1) {
				solution.sequence[t++] = j;
				break;
			}
		}
	}
	Populate_Solution_Structue(solution, c, &solution.value);
}

void Update_Current_Solution(SOLUTION solution, double **c, int k, int l, int p, int q, int w, int r) {

	int i, j, t;

	//Remove x_1, x_2, and x_3 arcs
	solution.x_best[solution.index_a[k][l]] = 0;
	solution.x_best[solution.index_a[p][q]] = 0;
	solution.x_best[solution.index_a[w][r]] = 0;
	//Add y_1, y_2, and y_star arcs
	solution.x_best[solution.index_a[k][q]] = 1;
	solution.x_best[solution.index_a[p][r]] = 1;
	solution.x_best[solution.index_a[w][l]] = 1;
	//Update sequence
	for (i = 0; i < solution.N; i++)
		solution.sequence[i] = -1;
	t = 0;
	solution.sequence[t++] = solution.depot;
	//printf("%d, ", route_HQ[s].sec_opt[t-1]);
	while (t < solution.N + 1) {
		for (j = 0; j < solution.N; j++) {
			if (solution.sequence[t - 1] != j && solution.x_best[solution.index_a[solution.sequence[t - 1]][j]] == 1) {
				solution.sequence[t++] = j;
				break;
			}
		}
	}
	Populate_Solution_Structue(solution, c, &solution.value);

	/*printf("Improved sequence with value: %0.2f \n", solution.value);
	for (i = 0; i < N + 1; i++)
		printf("%d ", solution.sequence[i]);
	printf("\n");*/
}


void Populate_Solution_Structue(SOLUTION solution, double **c, double *value) {

	int i, a;

	for (i = 1; i < solution.N - 1; i++) {
		solution.predecessor[solution.sequence[i]] = solution.sequence[i - 1];
		solution.successor[solution.sequence[i]] = solution.sequence[i + 1];
	}
	solution.predecessor[solution.sequence[0]] = solution.sequence[solution.N - 1];
	solution.successor[solution.sequence[0]] = solution.sequence[1];
	solution.predecessor[solution.sequence[solution.N - 1]] = solution.sequence[solution.N - 2];
	solution.successor[solution.sequence[solution.N - 1]] = solution.sequence[0];
	
	//printf("Sequence, predecessor, successor: \n");
	//for (i = 0; i < solution.N; i++)
	//	printf("(%d, %d, %d, %d) \n", i, solution.sequence[i], solution.predecessor[solution.sequence[i]], solution.successor[solution.sequence[i]]);
	////printf("\n");



	*value = 0;
	for (i = 0; i < solution.N - 1; i++)
		*value += c[solution.sequence[i]][solution.sequence[i + 1]];
	*value += c[solution.sequence[solution.N - 1]][solution.sequence[0]];
	for (a = 0; a < solution.N; a++) {
		solution.a[a].tail = solution.sequence[a];
		if (a < solution.N - 1) {
			solution.a[a].head = solution.sequence[a + 1];
			solution.a[a].cost = c[solution.sequence[a]][solution.sequence[a + 1]];
		}
		else {
			solution.a[a].head = solution.sequence[0];
			solution.a[a].cost = c[solution.sequence[a]][solution.sequence[0]];
		}
	}
	qsort((ARC*)solution.a, solution.N, sizeof(solution.a[0]), Compare_Cost_Value2);

}




void Compute_Cost_Coefficients_SDTSP(int s, double** p, double*** CSD) {

	int i, j, z, zp, t;
	int Z, count_arcs, NA;
	double value;
	ARC* arcs_zoneids;
	double percent_used = 0.95;
	int min_arcs = 20;

	Z = route_HQ[s].Z;

	arcs_zoneids = (ARC*)calloc(route_HQ[s].A, sizeof(ARC));

	for (z = 0; z < Z; z++) {
		for (zp = 0; zp < Z; zp++) {
			if (z != zp) {
				count_arcs = 0;
				for (i = 0; i < route_HQ[s].zoneid[z].dim; i++) {
					for (j = 0; j < route_HQ[s].zoneid[zp].dim; j++) {
						arcs_zoneids[count_arcs].head = j;
						arcs_zoneids[count_arcs].tail = i;
						arcs_zoneids[count_arcs++].cost = route_HQ[s].time[route_HQ[s].zoneid[z].stops[i]][route_HQ[s].zoneid[zp].stops[j]];
					}
				}
				qsort((ARC*)arcs_zoneids, count_arcs, sizeof(arcs_zoneids[0]), Compare_Cost_Value2);
				if (count_arcs < min_arcs)
					NA = count_arcs;
				else {
					NA = (int)ceil(count_arcs * percent_used);
					if (NA < min_arcs)
						NA = min_arcs;
				}
				value = 0;
				for (i = 0; i < NA; i++)
					value += arcs_zoneids[i].cost;
				route_HQ[s].time_zoneids[z][zp] = (double)(value / NA);
				//printf("(%d, %d): %0.2f ", z, zp, route_HQ[s].time_zoneids[z][zp]);
				for (t = 1; t < Z; t++)
					CSD[z][zp][t] = route_HQ[s].time_zoneids[z][zp];
			}
		}
	}

	for (i = 0; i < Z; i++) {
		for (t = 0; t < Z; t++) {
			p[i][t] = 0;
		}
	}

	free(arcs_zoneids);

}

int Compare_Cost_Value(const void* a, const void* b)
{
	if (((ORDCOSTS*)a)->c > ((ORDCOSTS*)b)->c)
		return 1;
	if (((ORDCOSTS*)a)->c < ((ORDCOSTS*)b)->c)
		return -1;
	return 0;
}


int Compare_Cost_Value2(const void* a, const void* b)
{
	if (((ARC*)a)->cost < ((ARC*)b)->cost)
		return 1;
	if (((ARC*)a)->cost > ((ARC*)b)->cost)
		return -1;
	return 0;
}


int Compare_Violation(const void* a, const void* b)
{
	if (((TWC*)a)->violation > ((TWC*)b)->violation)
		return 1;
	if (((TWC*)a)->violation < ((TWC*)b)->violation)
		return -1;
	return 0;
}