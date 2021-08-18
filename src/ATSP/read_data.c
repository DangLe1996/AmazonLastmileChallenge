#include "headers.h"


void read_INSTANCE_AMAZON(const char *name) {

	int i, j, e;
	int status = 0;
	double aux;
	FILE *input;
	char text[100];
	char text1[10] = "routeID";

	N = 0;
	input = open_file(name, "r");
	fscanf(input, "%d ", &N);
	
	//fscanf(input, "%s ", &text);
	//while (strcmp(text,text1) != 0) {
	for (i = 0; i < N; i++)
		fscanf(input, "%s ", &text);
	//}
	
	E = (N*(N - 1));

	Initialize_memory();

	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			if (fscanf(input, "%lf", &aux) != YES) {
				fprintf(stderr, "ERROR: Unable to read dist matrix (%d,%d)\n", i, j);
			}
			else {
				C[j][i] = ((double)(aux));
			}

		}
		//fscanf(input, "%s ", &text);
	}
	fclose(input);

	for (i = 0; i < N; i++)
		index_e[i][i] = NONE;
	e = 0;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			if (i != j) {
				index_i[e] = i;
				index_j[e] = j;
				index_e[i][j] = e;
				e++;
			}
		}
	}
	assert(e == E);

}


void initialize_Instance_Amazon_New(double** travelCost, int n_stops) {

	int i, j, e;
	N = n_stops;
	E = (N * (N - 1));
	Initialize_memory();

	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			C[i][j] = travelCost[i][j];
		}
	}
	for (i = 0; i < N; i++)
		index_e[i][i] = NONE;
	e = 0;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			if (i != j) {
				index_i[e] = i;
				index_j[e] = j;
				index_e[i][j] = e;
				e++;
			}
		}
	}
	assert(e == E);

}


void read_INSTANCE(const char *name) {
	
	int i,j,e;
	int status = 0;
	long long int aux;
    FILE *input;
	
	input = open_file(name, "r");

	fscanf(input, "%d ", &N);
	E = (N*(N - 1)) / 2;

	Initialize_memory();

    for (i=0; i<N; i++) {
        for (j=0; j<N; j++) {
            if (fscanf(input, "%lld", &aux) != YES) {
		        fprintf(stderr, "ERROR: Unable to read dist matrix (%d,%d)\n",i,j);
	        } else {
				C[i][j] = ((double)(aux));
			}

		}
	}
	fclose(input);

	for (i=0; i<N; i++)
		index_e[i][i] = NONE;
	e=0;
	for (i=0; i<N-1; i++) {
		for (j=i+1; j<N; j++) {
			index_i[e] = i;
			index_j[e] = j;
			index_e[i][j] = e;
			index_e[j][i] = e;
			e++;
		}
	}
	assert(e == E);

}


FILE *open_file(const char *filename, const char *mode) {
	FILE *file;
	if ((file = fopen(filename, mode)) == NULL) {
		fprintf(stderr, "ERROR: Unable to open file.\n");
	}
	return file;
}


void Initialize_memory(void)
{
	int i;

	C = create_double_matrix(N, N);
	index_i = create_int_vector(E);
	index_j = create_int_vector(E);
	index_e = create_int_matrix(N, N);
	sepcut.cutind = create_int_vector(E);
	sepcut.cutval = create_double_vector(E);
	cutset = (CUTSET *) calloc(N-1, sizeof(CUTSET));
	for (i = 0; i < N-1; i++)
	  cutset[i].S = create_int_vector(N);
	capacity = create_double_matrix(N, N);
	visited = create_int_vector(N);
	queue = create_int_vector(N);
	tree = create_int_vector(N);
	currentFlow = create_double_matrix(N, N);
	previous = create_int_vector(N);
	alpha = create_double_vector(E);

}


void free_memory(void)
{
	int i;

	for (i = 0; i < N; i++){
		free(index_e[i]);
		free(C[i]);
		free(currentFlow[i]);
	}
	free(C);
	free(index_i);
	free(index_j);
	free(index_e);
	free(sepcut.cutind);
	free(sepcut.cutval);
	for (i = 0; i < N-1; i++)
	   free(cutset[i].S);
	free(capacity);
	free(visited);
	free(queue);
	free(tree);
	free(currentFlow);
	free(previous);
	free(alpha);
}


// CPLEX functions to allocate memeory to arrays

void i_vector(int **vector, int n, char *s)
{
	if ((*vector = (int *)calloc(n, sizeof(int))) == NULL)
		//error(s);
		printf("Error: Insuficient memory \n");
	return;
}

void d_vector(double **vector, int n, char *s)
{
	if ((*vector = (double *)calloc(n, sizeof(double))) == NULL)
		// error(s);
		printf("Error: Insuficient memory \n");
	return;
}

void c_vector(char **vector, int n, char *s)
{
	if ((*vector = (char *)calloc(n, sizeof(char))) == NULL)
		//error(s);
		printf("Error: Insuficient memory \n");
	return;
}

char *create_char_vector(int cells) {
	char *ptr = (char *)calloc(cells, sizeof(char));
	if (ptr == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate memory for char_vector\n");
	}
	return ptr;
}
void free_char_vector(char **ptr) {
	if (*ptr == NULL) {
		fprintf(stderr, "ERROR: Unable to free memory from char_vector\n");
	}
	else {
		free(*ptr);
		*ptr = NULL;
	}
}

double *create_double_vector(int cells) {
	double *ptr = (double *)calloc(cells, sizeof(double));
	if (ptr == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate memory for double_vector\n");
	}
	return ptr;
}
void free_double_vector(double **ptr) {
	if (*ptr == NULL) {
		fprintf(stderr, "ERROR: Unable to free memory from double_vector\n");
	}
	else {
		free(*ptr);
		*ptr = NULL;
	}
}

int *create_int_vector(int cells) {
	int *ptr = (int *)calloc(cells, sizeof(int));
	if (ptr == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate memory for int_vector\n");
	}
	return ptr;
}
void free_int_vector(int **ptr) {
	if (*ptr == NULL) {
		fprintf(stderr, "ERROR: Unable to free memory from int_vector\n");
	}
	else {
		free(*ptr);
		*ptr = NULL;
	}
}

double **create_double_matrix(int rows, int columns) {
    int i;
    double **ptr = (double **) calloc(rows, sizeof(double *));
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: Unable to allocate memory for double_matrix\n");
    } else {
		for (i=0; i<rows; i++) { ptr[i] = create_double_vector(columns); }
	}
    return ptr;
}
void free_double_matrix(double ***ptr, int rows) {
	int i;
	if (*ptr == NULL) {
		fprintf(stderr, "ERROR: Unable to free memory from double_matrix\n");
	} else {
		for (i=0; i<rows; i++) { free_double_vector( &(*ptr)[i] ); }
		*ptr = NULL;
	}
}

int **create_int_matrix(int rows, int columns) {
    int i;
    int **ptr = (int **) calloc(rows, sizeof(int *));
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: Unable to allocate memory for int_matrix\n");
    } else {
		for (i=0; i<rows; i++) { ptr[i] = create_int_vector(columns); }
	}
    return ptr;
}

int ***create_int_matrix3d(int rows, int columns, int col2) {
	int i;
	int ***ptr = (int ***)calloc(rows, sizeof(int **));
	if (ptr == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate memory for int_matrix\n");
	}
	else {
		for (i = 0; i<rows; i++) { ptr[i] = create_int_matrix(columns,col2); }
	}
	return ptr;
}

void free_int_matrix(int ***ptr, int rows) {
	int i;
	if (*ptr == NULL) {
		fprintf(stderr, "ERROR: Unable to free memory from int_matrix\n");
	} else {
		for (i=0; i<rows; i++) { free_int_vector( &(*ptr)[i] ); }
		*ptr = NULL;
	}
}

