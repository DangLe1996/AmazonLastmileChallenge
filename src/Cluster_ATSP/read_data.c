#include "headers.h"
// #include "Score.h"



FILE *open_file(const char *filename, const char *mode) {
	FILE *file;
	if ((file = fopen(filename, mode)) == NULL) {
		fprintf(stderr, "ERROR: Unable to open file.\n");
	}
	return file;
}


void Initialize_memory(int s)
{

	route_HQ[s].time = create_double_matrix(route_HQ[s].dim, route_HQ[s].dim);
	route_HQ[s].norm_time = create_double_matrix(route_HQ[s].dim, route_HQ[s].dim);
	//route_HQ[s].zone = (char **)calloc(route_HQ[s].dim, sizeof(char *));
	//for (i = 0; i < route_HQ[s].dim; i++)
	route_HQ[s].zone = create_int_vector(route_HQ[s].dim);
	route_HQ[s].start_time = create_double_vector(route_HQ[s].dim);
	route_HQ[s].end_time = create_double_vector(route_HQ[s].dim);
	route_HQ[s].service_time = create_double_vector(route_HQ[s].dim);
	route_HQ[s].arrival_time = create_double_vector(route_HQ[s].dim);
	route_HQ[s].TW = create_int_vector(route_HQ[s].dim);
	route_HQ[s].x_obs = create_int_vector(route_HQ[s].A);
	route_HQ[s].x_opt = create_int_vector(route_HQ[s].A);
	route_HQ[s].sec_obs = create_int_vector(route_HQ[s].dim+1);
	route_HQ[s].sec_opt = create_int_vector(route_HQ[s].dim+1);
	route_HQ[s].stop_seq_index = create_int_vector(route_HQ[s].dim + 1);
	route_HQ[s].index_a = create_int_matrix(route_HQ[s].dim, route_HQ[s].dim);
	route_HQ[s].index_i = create_int_vector(route_HQ[s].A);
	route_HQ[s].index_j = create_int_vector(route_HQ[s].A);
}


void free_memoryIC(void)
{
	int s, i;

	for (s = 0; s < S; s++) {
		for (i = 0; i < route_HQ[s].dim; i++) {
			free(route_HQ[s].time[i]);
			free(route_HQ[s].norm_time[i]);
			//free(route_HQ[s].zone[i]);
			free(route_HQ[s].index_a[i]);
		}

		free(route_HQ[s].time);
		free(route_HQ[s].norm_time);
		free(route_HQ[s].zone);
		free(route_HQ[s].start_time);
		free(route_HQ[s].end_time);
		free(route_HQ[s].service_time);
		free(route_HQ[s].arrival_time);
		free(route_HQ[s].TW);
		free(route_HQ[s].x_obs);
		free(route_HQ[s].x_opt);
		free(route_HQ[s].sec_obs);
		free(route_HQ[s].sec_opt);
		free(route_HQ[s].stop_seq_index);
		free(route_HQ[s].index_a);
		free(route_HQ[s].index_i);
		free(route_HQ[s].index_j);
		free(route_HQ[s].sec_zone_obs);
		free(route_HQ[s].sec_zone_opt);
		for (i = 0; i < route_HQ[s].Z + 1; i++) {
			free(route_HQ[s].zoneid[i].stops);
			free(route_HQ[s].time_zoneids[i]);
		}
		free(route_HQ[s].zoneid);
		free(route_HQ[s].time_zoneids);
	}
	free(route_HQ);
	free(zone_interac.freq);
	for (i = 0; i < count_zones; i++)
		free(zone_interac.index_a[i]);
	free(zone_interac.index_a);
	free(zone_interac.index_k);
	free(zone_interac.index_m);
	free(same_zone);
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

int* increase_int_vector(int *ptr0, int cells) {
	int* ptr = (int*)realloc(ptr0, cells*sizeof(int));
	if (ptr == NULL) {
		fprintf(stderr, "ERROR: Unable to increase memory for int_vector\n");
	}
	return ptr;
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

