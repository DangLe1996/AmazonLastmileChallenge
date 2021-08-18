#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
// #include "cplex.h"
// #include "ilcplex\cplexcheck.h"
// #include "LKHmain.h"

/// C PREPROCESSOR CODE ////////////////////////////////////////////////////////

// Boolean values simulated with signed integers
#define ALL		2
#define YES     1	// True
#define NO      0	// False
#define NONE   -1	// Unknown

// General precision for dealing with non integer comparisons
#define EPSILON 0.0001

// Some basic functions:
#define ABS(x) (((x) > 0 ) ? (x) : -(x))	
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))
#define SIZE(x) (sizeof (x) / sizeof (*(x)))
////////////////////////////////////////////////////////////////////////////////

/// Typedefs & structs /////////////////////////////////////////////////////////

typedef struct LINE {
	int		dim; //size of line
	int		start;	// first node of line
	int  	*sequence; // sequence of nodes in line
} LINES;


typedef struct CUTSET {
	int		dim; //size of cut
	int  	*S; // set of vertices in cut
} CUTSET;

struct cutinfo {
	// CPXLPptr lp;
	int      numcols;
	int      num;
	double   *x;
	int      *beg;
	int      *ind;
	double   *val;
	double   *rhs;
	int      nodeid;
	double   nodeobjval;
	int      objsen;
};
typedef struct cutinfo CUTINFO, *CUTINFOptr;


typedef struct ZONEINFO {
	int A_z;
	int **index_a;
	int *index_k;
	int *index_m;
	int *freq;
} ZONEINFO;

typedef struct ZONEID {
	int	 dim; //number of stops in zoneid
	int  uzoneid; //unique zoneid number among all routes
	int* stops; // set of stops in zoneid
} ZONEID;


typedef struct BLOCKS {
	int	 count; //number of stops in zoneid
	int  zoneid; //unique zoneid
} BLOCKS;

typedef struct TWC {
	int	 stop;
	int pos_seq;
	int EoL;
	double  violation;
} TWC;

typedef struct INFOINSTANCE {
	int dim;
	int A;
	int depot;
	int Z;
	double **time;
	double **norm_time;
	double** time_zoneids;
	int *zone;
	double* start_time;
	double* end_time;
	int* TW;
	double* service_time;
	double* arrival_time;
	int *x_obs;
	int *x_opt;
	int *sec_obs;
	int *sec_opt;
	int *sec_zone_obs;
	int *sec_zone_opt;
	int* stop_seq_index;
	ZONEID *zoneid;
	int a_same_zone_obs;
	int a_same_zone_opt;
	int a_dn_zone_obs;
	int a_dn_zone_opt;
	int a_ds_zone_obs;
	int a_ds_zone_opt;
	double sumt_same_zone_obs;
	double sumt_same_zone_opt;
	double sumt_ds_zone_obs;
	double sumt_dn_zone_obs;
	double sumt_ds_zone_opt;
	double sumt_dn_zone_opt;
	int **index_a;
	int *index_i;
	int *index_j;
} INFOINSTANCE;

typedef struct SCUTS {
	double value;
	int cutnz;
	int *cutind;
	double *cutval;
} SCUTS;

typedef struct QUAD {
	int k;
	int l;
	int p;
	int q;
} QUAD;

typedef struct ARC {
	int head;
	int tail;
	double cost;
} ARC;

typedef struct SOLUTION {
	int N;
	int A;
	int depot;
	double value;
	int* sequence;
	int* x_best;
	int* predecessor;
	int* successor;
	ARC* a;
	int** index_a;
	int* index_i;
	int* index_j;
} SOLUTION;



typedef struct ORDCOSTS {
	int j;
	double c;
} ORDCOSTS;

typedef struct ORDROWS {
	int i;
	ORDCOSTS* A;
} ORDROWS;

int		N, E;
int     *cut_set;
int		*index_i;
int		*index_j;
int		**index_e;
double	**capacity;
SCUTS   sepcut;
CUTSET  *cutset;
int		*visited;
int		*queue;
int		*tree;
double  *alpha;
double  **currentFlow;
int		*previous;
int		n_conn_comp;
double  old_objval, pi_0;
int     count_same_node;
int     n_int_feas_subproblems;
int     n_frac_feas_subproblems;
int     n_feas_subproblems;
int     S;
INFOINSTANCE *route_HQ;
//INFOINSTANCE *route_MQ;
//INFOINSTANCE *route_LQ;
ZONEINFO zone_interac;
int* same_zone;
//double  **C;
int     max_dim;
int count_zones;
double** p;
double*** CSD;


double solve_SDTSP(int, double**, double***);
void Route_Refinement_Step(int);
void Move_Stop_Forward_Position(int, int*, int, int, int);
void Move_Stop_Backward_Position(int, int*, int, int, int);
void Compute_Arrival_Times(int , int* , double* , double* , double* , int* );
void Construct_List_Violations_TWC(int , TWC* , int* , double *);
void Construct_List_Violations_TWC_Obs_Routes(int, TWC*, int*, double*);
double Lin_Kernighan_Heuristic_ATSP(int , double** , int* );
int Sequential_Primary_Change(SOLUTION , ORDROWS* , double**);
void Update_Current_Solution(SOLUTION, double**, int, int, int, int, int, int);
void Populate_Solution_Structue(SOLUTION , double** , double *);
int Compare_Cost_Value(const void* , const void* );
int Compare_Cost_Value2(const void* , const void* );
int Compare_Violation(const void* , const void* );
double	min_cap_cuts(double **);
void	connected_components(double **);
double	maxFlowAndCut(int , int , double **);
double exact_separation_SEC(int, double *);
static void free_and_null(char **);

double solve_TSP(int *);
double solve_ATSP(int);
//double solve_Clustered_ATSP(int);
int Triple_Change(int, SOLUTION, double**, int*);
double Heuristic_Clustered_ATSP(int, int, int, double**, int*, int*, int*);
double solve_Clustered_ATSP(int, int, int, double**, int*, int*, int*);
void Solve_ATSP_Inverse_Model_Eps_st(void);
void Solve_ATSP_Inverse_Model_Max_Eps_s(void);
void Compute_Cost_Coefficients(double *, int, int);
void Compute_Cost_Coefficients_Clustered(int);
void Compute_Cost_Coefficients_SDTSP(int, double**, double***);
void Compute_Constraint_Coefficients(int);
void Generate_Solution_Structure(int, int*);
FILE	*open_file(const char *filename, const char *mode);
void Read_Amazon_Routes(const char *);
char	*create_char_vector(int cells);
void	free_char_vector(char **ptr);
double	*create_double_vector(int cells);
void	free_double_vector(double **ptr);
int		*create_int_vector(int cells);
int* increase_int_vector(int* ptr0, int cells);
void	free_int_vector(int **ptr);
double	**create_double_matrix(int rows, int columns);
void	free_double_matrix(double ***ptr, int rows);
int		**create_int_matrix(int rows, int columns);
int ***create_int_matrix3d(int rows, int columns, int col2);
void	free_int_matrix(int ***ptr, int rows);
void i_vector(int **vector, int n, char *s);
void d_vector(double **vector, int n, char *s);
void c_vector(char **vector, int n, char *s);
void Initialize_memory(int);
void free_memoryIC(void);
void free_and_null(char **);

///////////////////////////////////////////////////////////////////////////////