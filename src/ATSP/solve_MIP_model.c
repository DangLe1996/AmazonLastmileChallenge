#include "headers.h"

double solve_TSP(int* sequence)
{
	int i, e;
	int index, index1;  // auxiliar indices to fill in the constraint matrix
	double best_upper_bound, best_lower_bound;
	int nodecount;
	//Variables to call cplex
	CPXLPptr  lp;      // data strucutre to store a problem in cplex ...................
	CPXENVptr env;     // cplex environment.............................................
	int       numcols; // number of variables ..........................................
	int       numrows; // number of constraints.........................................
	int       numnz;   // number of non-zero elements in the matrix ....................
	int       objsen;  // optimization sense (min:1, max:-1 ) ..........................
	double    *obj;    // objective function coefficients ..............................
	double    *rhs;    // right and side of constraints ................................
	char      *sense;  // constraints sense (<=: 'L', =:'E', >=:'G') ...................
	int       *matbeg; // index of first non-zero element in each row...................
	int       *matind; // associated column of each non-zelo element ...................
	double    *matval; // coefficient values fo the non-zero elements of constraints....
	double    *lb;     // lower bounds of variables.....................................
	double    *ub;     // upper bounds of variables.....................................
	int       status;  // optimization status......................... .................
	double    *x;      // solution vector (double, even if the problem is integer) .....
	char probname[16]; // problem name for cplex .......................................
	char      *ctype;  // variable type ('C', 'I', 'B') only if integer.................
	double    value;   // objevtive value of solution ..................................
	//int       *pos_y;
	char	  **colname;
	int cur_numcols;
	CUTINFO cutinfo;
	cutinfo.x = NULL;
	cutinfo.beg = NULL;
	cutinfo.ind = NULL;
	cutinfo.val = NULL;
	cutinfo.rhs = NULL;


	//Start preprocesing to remove x_kij variables
	//pos_y = create_int_vector(E);

	count_same_node = 0;
	objsen = 1; //min
	//Initialize CPLEX environment
	env = CPXopenCPLEX(&status);
	if (env == NULL) {
		char  errmsg[1024];
		printf("Could not open CPLEX. \n");
		CPXgeterrorstring(env, status, errmsg);
		printf("%s", errmsg);
	}

	// Create the problem in CPLEX 
	strcpy(probname, "TSP");
	lp = CPXcreateprob(env, &status, probname);
	if (env == NULL) {
		char  errmsg[1024];
		printf("Could not create LP. \n");
		CPXgeterrorstring(env, status, errmsg);
		printf("%s", errmsg);
	}


	//Define y_ij variables
	numcols = E;
	colname = (char**)calloc(numcols, sizeof(char*));
	for (i = 0; i < numcols; i++){
		colname[i] = (char*)calloc(255, sizeof(char));
	}
	d_vector(&obj, numcols, "open_cplex:1");
	d_vector(&lb, numcols, "open_cplex:8");
	d_vector(&ub, numcols, "open_cplex:9");
	c_vector(&ctype, numcols, "open_cplex:0");

	for (e = 0; e<E; e++){
		//pos_y[e] = index1;
		obj[e] = C[index_i[e]][index_j[e]];
		lb[e] = 0;
		ub[e] = 1;
		ctype[e] = 'B';
		sprintf(colname[e], "y_%d,%d ", index_i[e], index_j[e]);
	}
	status = CPXnewcols(env, lp, E, obj, lb, ub, ctype, colname);
	if (status)
		fprintf(stderr, "CPXnewcols failed.\n");
	free(obj);
	free(lb);
	free(ub);
	free(colname);
	free(ctype);


	// Add node degree constraints (outgoing flow)
	numrows = N;
	numnz =  N*(N - 1);
	d_vector(&rhs, numrows, "open_cplex:2");
	c_vector(&sense, numrows, "open_cplex:3");
	i_vector(&matbeg, numrows, "open_cplex:4");
	i_vector(&matind, numnz, "open_cplex:6");
	d_vector(&matval, numnz, "open_cplex:7");

	index = 0;
	index1 = 0;
	for (i = 0; i < N; i++){
		sense[index1] = 'E';
		rhs[index1] = 1;
		matbeg[index1++] = index;
		for (e = 0; e < E; e++){
			if (index_i[e] == i){
				matind[index] = e;
				matval[index++] = 1;
			}
		}
	}
	status = CPXaddrows(env, lp, 0, index1, index, rhs, sense, matbeg, matind, matval, NULL, NULL);
	if (status)
		fprintf(stderr, "CPXaddrows failed.\n");
	free(matbeg);
	free(matind);
	free(matval);
	free(sense);
	free(rhs);

	// Add node degree constraints (incoming flow)
	numrows = N;
	numnz = N*(N - 1);
	d_vector(&rhs, numrows, "open_cplex:2");
	c_vector(&sense, numrows, "open_cplex:3");
	i_vector(&matbeg, numrows, "open_cplex:4");
	i_vector(&matind, numnz, "open_cplex:6");
	d_vector(&matval, numnz, "open_cplex:7");

	index = 0;
	index1 = 0;
	for (i = 0; i < N; i++) {
		sense[index1] = 'E';
		rhs[index1] = 1;
		matbeg[index1++] = index;
		for (e = 0; e < E; e++) {
			if (index_j[e] == i) {
				matind[index] = e;
				matval[index++] = 1;
			}
		}
	}
	status = CPXaddrows(env, lp, 0, index1, index, rhs, sense, matbeg, matind, matval, NULL, NULL);
	if (status)
		fprintf(stderr, "CPXaddrows failed.\n");
	free(matbeg);
	free(matind);
	free(matval);
	free(sense);
	free(rhs);

	//CPXwriteprob(env,lp,"modelTSP.lp",NULL);                          //write the model in .lp format if needed (to debug)

	CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF); //output display
	//CPXsetintparam(env,CPX_PARAM_INTSOLLIM,1);    //stops after finding first integer sol.
	CPXsetintparam(env, CPX_PARAM_MIPDISPLAY, 0); //different levels of output display
	//CPXsetintparam(env,CPX_PARAM_MIPEMPHASIS,1);//0:balanced; 1:feasibility; 2:optimality,3:bestbound, 4:hiddenfeas
	//CPXsetdblparam(env, CPX_PARAM_TILIM, 60); // time limit
	//CPXsetdblparam(env,CPX_PARAM_TRELIM, 14000); // B&B memory limit
	//CPXsetdblparam(env,CPX_PARAM_EPGAP, 0.0000000001); // e-optimal solution (%gap)
	//CPXsetdblparam(env,CPX_PARAM_EPAGAP, 0.0000000001); // e-optimal solution (absolute value)
	//CPXsetdblparam(env,CPX_PARAM_EPINT, 0.0000000001); // integer precision
	//CPXsetintparam(env,CPX_PARAM_THREADS, 1); // Number of threads to use
	//CPXsetdblparam(env,CPX_PARAM_EPRHS, 0.0000001);
	//CPXsetintparam(env,CPX_PARAM_REDUCE, 0);  // only needed when adding lazy constraints
	//CPXsetintparam(env,CPX_PARAM_HEURFREQ, -1); //heuristic frequency and intensisty 
	//CPXsetdblparam(env,CPX_PARAM_CUTSFACTOR, 1.0);  //limit the number of cuts added by cplex 1.0002
	//CPXsetdblparam(env,CPX_PARAM_CUTUP,UpperBound+.01); // provide an initial upper bound
	CPXsetintparam(env, CPX_PARAM_MIPEMPHASIS, CPX_MIPEMPHASIS_OPTIMALITY);  // MIP emphasis: optimality, feasibility, moving best bound
	//CPXsetintparam(env,CPX_PARAM_PARALLELMODE, 1); 
	//CPXsetintparam(env,CPX_PARAM_PREIND,0);
	//CPXsetintparam(env,CPX_PARAM_MIPORDIND,CPX_ON); // Turn on or off the use of priorities on bracnhing variables
	//CPXsetintparam(env,CPX_PARAM_MIPEMPHASIS,CPX_MIPEMPHASIS_BESTBOUND);  // MIP emphasis: optimality, feasibility, moving best bound
	//CPXsetintparam(env, CPX_PARAM_MIPSEARCH, CPX_MIPSEARCH_TRADITIONAL);

	/* Assure linear mappings between the presolved and original
	models */
	status = CPXsetintparam(env, CPX_PARAM_PRELINEAR, 0);
	/* Turn on traditional search for use with control callbacks */
	status = CPXsetintparam(env, CPX_PARAM_MIPSEARCH, CPX_MIPSEARCH_TRADITIONAL);
	/* Let MIP callbacks work on the original model */
	status = CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF);

	cur_numcols = CPXgetnumcols(env, lp);
	cutinfo.lp = lp;
	cutinfo.numcols = cur_numcols;
	//printf("Columns loaded in Cplex: %d \n", cur_numcols);
	cutinfo.x = (double *)malloc(cur_numcols * sizeof (double));
	/* Set up to use MIP callback */
	status = CPXsetusercutcallbackfunc(env, mycutcallback, &cutinfo);
	status = CPXsetlazyconstraintcallbackfunc(env, mycutcallback, &cutinfo);

	status = CPXmipopt(env, lp);  //solve the integer program
	if (status) fprintf(stderr, "Failed to optimize LP.\n");

	// retrive solution values
	CPXgetmipobjval(env, lp, &value);
	//printf("Upper bound: %.2f   ",value);
	best_upper_bound = value;
	CPXgetbestobjval(env, lp, &value);  //best lower bound in case the problem was not solved to optimality
	best_lower_bound = value;
	//printf("Lower bound: %.2f  \n",value);

	nodecount = CPXgetnodecnt (env, lp);
	//printf("Number of BB nodes : %ld  \n",nodecount);

	numcols = CPXgetnumcols(env, lp);
	d_vector(&x, numcols, "open_cplex:0");
	CPXgetmipx(env, lp, x, 0, numcols - 1);  // obtain the values of the decision variables
	


	for (e = 0; e < E; e++){
		if (x[e] > 0.01) { //check what variable equals to 1 
			//i is a stop and j is a stop. y[3][5] means 3 going before 5. 
			sequence[index_i[e]] = index_j[e] ;
			//printf("y[%d][%d]= 1 \n", index_i[e] , index_j[e] );
		}
	}

	for (i = 0; i < N; i++)
		sequence[i] = -1;
	int t = 0;
	sequence[t++] = 0;
	while (t < N) {
		for (int j = 0; j < N; j++) {
			if (x[index_e[sequence[t - 1]][j]] > 0.01) {
				sequence[t++] = j;
				break;
			}
		}
	}
	//printf("Optimal ATSP sequence: ");
	//for (i = 0; i < N; i++)
	//	printf("%d ", sequence[i] + 1);
	//printf("\n");


	//// TERMINATE:

	if (lp != NULL) {
		status = CPXfreeprob(env, &lp);
		if (status) {
			fprintf(stderr, "CPXfreeprob failed, error code %d.\n", status);
		}
	}
	if (env != NULL) {
		status = CPXcloseCPLEX(&env);
		if (status) {
			char  errmsg[1024];
			fprintf(stderr, "Could not close CPLEX environment.\n");
			CPXgeterrorstring(env, status, errmsg);
			fprintf(stderr, "%s", errmsg);
		}
	}

	free(x);
	free_and_null((char **)&cutinfo.x);
	free_and_null((char **)&cutinfo.beg);
	free_and_null((char **)&cutinfo.ind);
	free_and_null((char **)&cutinfo.val);
	free_and_null((char **)&cutinfo.rhs);


	return best_lower_bound;
}



static int CPXPUBLIC
mycutcallback(CPXCENVptr env,
void       *cbdata,
int        wherefrom,
void       *cbhandle,
int        *useraction_p)
{
	int status = 0;


	CUTINFOptr cutinfo = (CUTINFOptr)cbhandle;

	int      numcols = cutinfo->numcols;
	int      numcuts = cutinfo->num;
	double   *x = cutinfo->x;
	int      *beg = cutinfo->beg;
	int      *ind = cutinfo->ind;
	double   *val = cutinfo->val;
	double   *rhs = cutinfo->rhs;
	int      *cutind = NULL;
	double   *cutval = NULL;
	int      *feas = NULL;
	int      addcuts = 0;
	int      i, j, n, e;
	double   objval;
	double   new_lower_bound;
	int      stop_cutgen, int_feas;
	int      count_violcuts = 0;
	int      flag_solve_SP = 0;
	double   tolerance_sep;
	double   lhs, SEC_viol;
	int      count_added, depth;
	double   EPSOBJ = 0.1;
	double   epsilon_LP = 0.0001;
	int      oldnodeid = cutinfo->nodeid;
	double   oldnodeobjval = cutinfo->nodeobjval;
	double   FENCHEL_viol;

	*useraction_p = CPX_CALLBACK_DEFAULT;



	// Get current depth in BB
	status = CPXgetcallbacknodeinfo(env, cbdata, wherefrom, 0, CPX_CALLBACK_INFO_NODE_DEPTH, &depth);
	if (status) {
		fprintf(stdout, "Can't get depth for node.");
		goto TERMINATE;
	}

	//if (wherefrom == CPX_CALLBACK_MIP_CUT_LOOP ||
	// wherefrom == CPX_CALLBACK_MIP_CUT_LAST) {
	// int    oldnodeid = cutinfo->nodeid;
	// double oldnodeobjval = cutinfo->nodeobjval;

	/* Retrieve nodeid and node objval of the current node */

	status = CPXgetcallbacknodeinfo(env, cbdata, wherefrom, 0,
		CPX_CALLBACK_INFO_NODE_SEQNUM,
		&cutinfo->nodeid);
	if (status) {
		fprintf(stderr, "Failed to get node id.\n");
		goto TERMINATE;
	}

	if (oldnodeid == cutinfo->nodeid){
		count_same_node++;
	}
	else{
		count_same_node = 0;
	}

	// status = CPXgetcallbacknodeinfo(env, cbdata, wherefrom, 0,
	//  CPX_CALLBACK_INFO_NODE_OBJVAL,
	//  &cutinfo->nodeobjval);
	// if (status) {
	//  fprintf(stderr, "Failed to get node objval.\n");
	//  goto TERMINATE;
	// }

	// /* Abort the cut loop if we are stuck at the same node
	// as before and there is no progress in the node objval */

	// if (oldnodeid == cutinfo->nodeid && depth % 5 == 0) {
	//  double objchg = (cutinfo->nodeobjval - oldnodeobjval);
	//  /* Multiply objchg by objsen to normalize
	//  the change in the objective function to
	//  the case of a minimization problem */
	//  objchg *= cutinfo->objsen;
	//  if (objchg <= EPSOBJ) {
	//   *useraction_p = CPX_CALLBACK_ABORT_CUT_LOOP;
	//   goto TERMINATE;
	//  }
	// }
	//}

	/* If we reached this point, we are
	.. in a lazyconstraint callback, or
	.. in a user cut callback, and cuts seem to help
	improving the node objval.
	In both cases, we retrieve the x solution and
	look for violated cuts. */



	//Get lower bound at current node
	status = CPXgetcallbacknodeinfo(env, cbdata, wherefrom, 0, CPX_CALLBACK_INFO_NODE_OBJVAL, &new_lower_bound);
	if (status) {
		fprintf(stdout, "Can't get depth for node.");
		goto TERMINATE;
	}

	status = CPXgetcallbacknodeobjval(env, cbdata, wherefrom, &objval);
	// status = CPXgetcallbackgloballb (env, cbdata, wherefrom, &objval, 0, 0); 
	if (status) {
		fprintf(stdout, "Can't get objective value for node.");
		goto TERMINATE;
	}

	// We must do something in every integer solution:
	tolerance_sep = -0.0001;
	//tolerance_sep = -0.01;
	flag_solve_SP = 0;
	int_feas = NO;
	if (wherefrom == CPX_CALLBACK_MIP_CUT_FEAS){
		flag_solve_SP = 1;
		tolerance_sep = -0.000000000001;
		int_feas = YES;
		//tolerance_sep = -0.001;
	}
	else{
		if (depth == 0 && ABS(objval - old_objval) > epsilon_LP) {
			flag_solve_SP = 1;
			old_objval = objval;
		}
		else{
			if (depth > 0 && depth % 10 == 0 && count_same_node < 2){
				flag_solve_SP = 1;
				old_objval = objval;

			}
		}
	}

	count_added = 0;
	if (flag_solve_SP == 1){
		status = CPXgetcallbacknodex(env, cbdata, wherefrom, x, 0, numcols - 1);
		if (status) {
			fprintf(stderr, "Failed to get node solution.\n");
			goto TERMINATE;
		}
		SEC_viol = exact_separation_SEC(int_feas,x);
		if (SEC_viol < tolerance_sep){
			if (int_feas == YES){
				sepcut.cutnz = 0;
				for (i = 0; i < cutset[0].dim; i++) {
					for (j = 0; j < cutset[0].dim; j++) {
						if (i != j) {
							sepcut.cutind[sepcut.cutnz] = index_e[cutset[0].S[i]][cutset[0].S[j]];
							sepcut.cutval[sepcut.cutnz++] = 1;
						}
					}
				}
				status = CPXcutcallbackadd(env, cbdata, wherefrom, sepcut.cutnz, cutset[0].dim - 1, 'L', sepcut.cutind, sepcut.cutval, CPX_USECUT_PURGE);
				count_added++;
				if (n_conn_comp > 2){
					for (n = 1; n < n_conn_comp; n++){
						sepcut.cutnz = 0;
						for (i = 0; i < cutset[n].dim; i++) {
							for (j = 0; j < cutset[n].dim; j++) {
								if (i != j) {
									sepcut.cutind[sepcut.cutnz] = index_e[cutset[n].S[i]][cutset[n].S[j]];
									sepcut.cutval[sepcut.cutnz++] = 1;
								}
							}
							status = CPXcutcallbackadd(env, cbdata, wherefrom, sepcut.cutnz, cutset[n].dim - 1, 'L', sepcut.cutind, sepcut.cutval, CPX_USECUT_PURGE);
							count_added++;
						}
					}
				}
			}
			else{
				for (n = 0; n < N - 1; n++){
					sepcut.cutnz = 0;
					lhs = 0;
					if (cutset[n].dim > 2){
						for (i = 0; i < cutset[n].dim ; i++) {
							for (j = 0; j < cutset[n].dim; j++) {
								if (i != j) {
									sepcut.cutind[sepcut.cutnz] = index_e[cutset[n].S[i]][cutset[n].S[j]];
									sepcut.cutval[sepcut.cutnz++] = 1;
									lhs += x[index_e[cutset[n].S[i]][cutset[n].S[j]]];
								}
							}
						}
						if (lhs - (cutset[n].dim - 1) > 0.0001){
							status = CPXcutcallbackadd(env, cbdata, wherefrom, sepcut.cutnz, cutset[n].dim - 1, 'L', sepcut.cutind, sepcut.cutval, CPX_USECUT_PURGE);
							count_added++;
						}
					}
				}
			}
		}
	}

	if (count_added > 0){
		stop_cutgen = 0;
	}
	else
		stop_cutgen = 1;

	//if (depth == 0 && count_added == 0)
	//	LP_lower_bound = cutinfo->nodeobjval;

	/* Tell CPLEX that cuts have been created */

	if (stop_cutgen == 0)
		*useraction_p = CPX_CALLBACK_SET;

TERMINATE:

	return (status);

} /* END mycutcallback */


double exact_separation_SEC(int INTEGER, double *y) {

	int	i, j;
	int status = 0;
	double min_flow;

	for (i = 0; i<N; i++) {
		for (j = 0; j<N; j++) {
			if (i != j)
				capacity[i][j] = y[index_e[i][j]];
			else
				capacity[i][i] = 0.0;
		}
	}
	n_feas_subproblems++;

	if (INTEGER == YES) { 
		connected_components(capacity);
		n_int_feas_subproblems++;
		if (n_conn_comp > 1)
			return -2.0;
		else
			return 0.0;
	}
	else { 
		min_flow = min_cap_cuts(capacity);
		n_frac_feas_subproblems++;
		return min_flow - 2;
	}
}



////////////////////////////////////////////////////////////////////////////////
// Returns up to N-1 cuts represented in the binary NxN-matrix cut_sets
void connected_components(double **capacity) {
	int	i, j;
	int	queueSize, current;
	int	starting_point;

	for (i = 0; i < N - 1; i++){
		cutset[i].dim = 0;
		for (j = 0; j < N; j++)
			cutset[i].S[j] = 0;
	}

	// Init everything:
	for (i = 0; i<N; i++) 
		visited[i] = NO;
	queueSize = 0;
	starting_point = 0;
	n_conn_comp = 0;

	// While not all nodes visited:
	while (queueSize<N) {
		// look for an unvisited node and add it to the queue:
		for (i = starting_point; i<N; i++){
			if (visited[i] == NO) {
				starting_point = i + 1;
				visited[i] = YES;
				current = queueSize;
				queue[queueSize++] = i;
				break;
			}
		}
		// We want to rememenber the connected component of this element:
		for (i = 0; i<N; i++) 
			cutset[n_conn_comp].S[cutset[n_conn_comp].dim] = 0;
		// Lets visit a whole connected component using a BFS:
		while (queueSize>current) {
			i = queue[current++];
			cutset[n_conn_comp].S[cutset[n_conn_comp].dim++] = i;
			for (j = 0; j<N; j++) {
				if (visited[j] == NO && capacity[i][j] > 0.5) {	// we are assuming integer capacities {0.0, 1.0}
					visited[j] = YES;
					queue[queueSize++] = j;
				}
			}
		}
		n_conn_comp++;
	}
}
////////////////////////////////////////////////////////////////////////////////

// Returns up to N-1 cuts represented in the binary NxN-matrix cut_sets
// Uses a simplified variant of the Gusfield's Gomory-Hu-Tree routine.
double min_cap_cuts(double **capacity){
	int		i, j;							// index variables for loops
	int		s, t;							// source and sink nodes
	double	min_flow;

	// Compute dummy min_flow:
	min_flow = 0.0;
	for (i = 1; i<N; i++) { 
		min_flow += capacity[0][i]; 
	}

	for (i = 0; i<N-1; i++) {
		cutset[i].dim = 0;
		for (j = 0; j<N; j++) {
			cutset[i].S[j] = 0;
		}
	}

	for (i = 0; i<N; i++) 
		tree[i] = 0;

	for (s = 1; s < N; s++) {	// visit the other nodes and update 'tree'

		// in this iteration we are going to update the edge 's-tree[s]'
		t = tree[s];
		min_flow = MIN(min_flow, maxFlowAndCut(s, t, capacity));

		// we have to update all the nodes connected with 't' that are on the 's' side
		for (i = 0; i < cutset[s - 1].dim; i++) {
			if (cutset[s - 1].S[i] != s  &&  tree[cutset[s - 1].S[i]] == t)
				tree[cutset[s - 1].S[i]] = s;
		}
	}

		// if the predecessor of 't' is in the connected component of 's' (='X')
		// we must swich the direction of the 's'-'t' link!
	for (i = 0; i < cutset[s - 1].dim; i++) {
		if (cutset[s - 1].S[i] == tree[t]) {
			tree[s] = tree[t];
			tree[t] = s;
			break;
		}
	}

	return min_flow;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Performs the Edmonds-Karp Algorithm and returns the maximum flow between
// 's' and 't'. The algorithm also remembers the connected component that
// contains 's' in the binary array 'visited'
double maxFlowAndCut(int s, int t, double **Capacity){
	int		i, j;							// index variables for loops
	int		u, v;                            // index variables for nodes
	int		queueIndex, queueSize;          // just for use an array as a queue
	double	flow;							// the augmenting flow
	double	maxFlow = 0.0;                  // the total amount of flow

	//double  **currentFlow = create_double_matrix(N, N);
	//int		*previous = create_int_vector(N);
//	int		*queue = create_int_vector(N);	// basic queue

	int DEBUG = s<0 ? YES : NO;
	s = ABS(s);

	// initialize the 'currentFlow'    
	for (i = 0; i < N; i++) {
		for (j = 0; j<N; j++) {
			currentFlow[i][j] = 0.0;
		}
	}
	cutset[s - 1].dim = 0;

	// perform as many flow augmentations as you can!
	flow = 1.0;                                     // this is just to start the loop
	while (flow > 0.0) {
		flow = 0.0;
		// initialize everything else!
		for (i = 0; i<N; i++) 
			visited[i] = NO;     // nobody has been visited yet
		visited[s] = YES;                           //  ...nobody but 's'
		queueSize = 0;                              // the queue contains 0 elements
		queue[queueSize++] = s;                     // now it contains s
		queueIndex = 0;                             // next element to leave the queue

		// perform a BFS to find a augmenting path between 's' and 't'
		while (queueIndex < queueSize){
			u = queue[queueIndex++];                // get the next element
			// now look if you can extend the augmenting path from 'u' to any other 'v'
			for (v = 0; v<N; v++) {
				if (visited[v] == NO && Capacity[u][v] - currentFlow[u][v] > 0.0) {
					visited[v] = YES;               // if you can, visit the new node
					queue[queueSize++] = v;           // put it in the queue
					previous[v] = u;                // remeber where you come from
					if (v == t) break;                // and avoid do extra work! 
				}
			}

			// now, before continue, check if we have finished the BFS!
			if (visited[t] == YES) {
				// we are going to trace back our steps looking for how much flow we can send
				v = t;
				u = previous[t];
				flow = Capacity[u][v] - currentFlow[u][v];
				while (u != s) {
					v = u;
					u = previous[v];
					if (flow > Capacity[u][v] - currentFlow[u][v])
						flow = Capacity[u][v] - currentFlow[u][v];
				}
				// now we can update the currentFlowMatrix with that amount of flow
				v = t;
				u = previous[t];
				currentFlow[u][v] += flow; // we add the flow in one direction
				currentFlow[v][u] -= flow; // and remove it from the other one!
				while (u != s) {
					v = u;
					u = previous[v];
					currentFlow[u][v] += flow; // we add the flow in one direction
					currentFlow[v][u] -= flow; // and remove it from the other one!
				}
				maxFlow += flow;                    // we have improved so we update
				break;                              // avoid do extra work!
			}
		}
	}

	for (i = 0; i < N; i++){
		if (visited[i] == YES)
			cutset[s - 1].S[cutset[s - 1].dim++] = i;
	}

	return maxFlow;
}


static void free_and_null(char **ptr)
{
	if (*ptr != NULL) {
		free(*ptr);
		*ptr = NULL;
	}
} /* END free_and_null */









