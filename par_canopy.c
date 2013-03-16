#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <omp.h>

int **data;
int **canopies;
int *canopy_size;

typedef struct{
	int size;
	int tail;
	int *values;
}Stack;

/*stack methods*/
int pop(Stack *stack){
	int val = -1;
	if (stack->size > 0){
		val = stack->values[stack->tail--];
		stack->size--;
	}
	return val;
}

void push(Stack *stack, int val){
	stack->tail++;
	stack->size++;
	stack->values[stack->tail] = val;
}

void empty(Stack *stack){
	stack->tail = -1;
	stack->size = 0;
}
	

/*
 * generate test data from the range
 *   0 to max and set up global vars
 * int n: number of data points
 * int max: all data is within the (0, max) range
 * int k: number of clusters
 */
void setup(int n, int max){
        int i;
        data = (int **)malloc(n*sizeof(int *));

        //generate random data for testing
        for (i = 0; i < n; i++){
                data[i] = (int *)malloc(2*sizeof(int));
                data[i][0] = (rand()%max);
                data[i][1] = (rand()%max);
        }

}

void canopy_cluster(int n, int x_1, int x_2, int **canopies, int *canopy_size, int idx_0, int idx_f, int **data){
	int c_idx, size, center, cur, i, dist_est;
	int my_num = omp_get_thread_num();
	//printf("Thread %d: [%d, %d]\n", my_num, idx_0, idx_f);
	
	Stack *data_stack, *temp_stack, *t;
	data_stack = (Stack*)malloc(sizeof(Stack));
	data_stack->tail = -1;
	data_stack->size = 0;
	data_stack->values = (int *)malloc(n*sizeof(int));
	temp_stack = (Stack*)malloc(sizeof(Stack));
	temp_stack->tail = -1;
	temp_stack->size = 0;
	temp_stack->values = (int *)malloc(n*sizeof(int));
	
	//canopy_size = (int *)malloc((n+1)*sizeof(int));
	//store the number of canopies in the first element
	canopy_size[0] = 0;

	//canopies = (int **)malloc(n*sizeof(int *));
	c_idx = 0;

	//printf("push: \n");
	//copy the values to cluster into the data stack
	for (i = idx_0; i <= idx_f; i++){
		push(data_stack, i);
	//	printf("[%d]", data[i][0]);
	}
	//printf("\n");

	while(data_stack->size > 0){
		center = pop(data_stack);
	//	printf("center: %d\n", center);
		canopies[c_idx] = (int *)malloc(n*sizeof(int));
		size = 0;
		canopies[c_idx][size++] = center;

		/*compare against all other points still left*/
		while(data_stack->size > 0){
			cur = pop(data_stack);

			//rough distance estimate (in this case 1-d)*/
			dist_est = abs(data[center][0] - data[cur][0]);
			if (dist_est <= x_1){/*within canopy*/
				canopies[c_idx][size++] = cur;
				if (dist_est > x_2){/*might be in another canopy as well*/
					push(temp_stack, cur);
				}
			}else{
				push(temp_stack, cur);
			}	
		}
		
		/*save the size of this canopy*/
		canopy_size[0]++;
		canopy_size[canopy_size[0]] = size;		

		/*swap stacks to continue building canopies*/
		t = data_stack;
		data_stack = temp_stack;
		temp_stack = t;
		c_idx++;
	}

}

void par_canopy_cluster(int n, int x_1, int x_2, int thread_count){
	int ***canopies;
	int **canopy_size;
	int i, j, l, my_num, chunk, k = -1, cur_size;
	int idx_0, idx_f, num_canopies, cur_canopy_size, thread_owner, canopy_idx;
	int **centers;
	int *center_owners;
	centers = (int **)malloc(n*sizeof(int *));
	center_owners = (int *)malloc(n*sizeof(int));
	canopy_size = (int **)malloc(thread_count*sizeof(int *));
	canopies = (int ***)malloc(thread_count*sizeof(int **));
	chunk = n/thread_count;
	int **c_canopies;
	int **final_canopies;
	int *c_canopy_size;
	int *final_canopy_size;
	c_canopies = (int **)malloc(n*sizeof(int *));
	final_canopies = (int **)malloc(n*sizeof(int *));
	c_canopy_size = (int *)malloc((n+1)*sizeof(int));
	final_canopy_size = (int *)malloc((n+1)*sizeof(int));
	
#	pragma omp parallel num_threads(thread_count)\
		private(my_num, idx_0, idx_f)
	{
		my_num = omp_get_thread_num();
		idx_0 = my_num*chunk;
		idx_f = idx_0 + chunk -1;
		if (my_num == (thread_count-1))
			idx_f = n-1;
		canopies[my_num] = (int **)malloc(n*sizeof(int *));
		canopy_size[my_num] = (int *)malloc((n+1)*sizeof(int));
		canopy_cluster(n, x_1, x_2, canopies[my_num], canopy_size[my_num], idx_0, idx_f, data);
	}

	//reduce the clusters from each thread by performing a second canopy
	for (i = 0; i < thread_count; i++){
		//for each canopy found by this thread
		num_canopies = canopy_size[i][0];
		for (j = 0; j < num_canopies; j++){
			k++;
			centers[k] = (int *)malloc(2*sizeof(int));
			centers[k][0] = data[canopies[i][j][0]][0];
			centers[k][1] = j;
			center_owners[k] = i;
			//printf("canopy center: %d, thread: %d\n", canopies[i][j][0], i);
		}	
	}
	k++;

	//determine which canopies to merge
	canopy_cluster(k, x_1, x_2, c_canopies, c_canopy_size, 0, k-1, centers); 
	num_canopies = c_canopy_size[0];

	for (i = 0; i < num_canopies; i++){
		cur_size = c_canopy_size[i+1];
		//printf("New canopy #%d\n", i);
		final_canopies[i] = (int *)malloc(n*sizeof(int));
		final_canopy_size[i] = 0;
		for (j = 0; j < cur_size; j++){
			thread_owner = center_owners[c_canopies[i][j]];
			canopy_idx = centers[c_canopies[i][j]][1];
			cur_canopy_size = canopy_size[thread_owner][canopy_idx+1]; 
			for (l = 0; l < cur_canopy_size; l++){
				//add all points in this sub-canopy into the final canopy
				final_canopies[i][final_canopy_size[i]++] = canopies[thread_owner][canopy_idx][l]; 
			}
		}
	}	

	printf("\nFinal canopies\n");
	for (i = 0; i < num_canopies; i++){
		for (j = 0; j < final_canopy_size[i]; j++){
			printf("[%d] ", final_canopies[i][j]);
		}
		printf("\n");
	}


}
 


int main(int argc, char* argv[]){
        int i, n, max, thread_count;
	
	if (argc < 3){
		printf("usage: [data size] [max range of test data] [num threads]\n");
		return -1;
	}

        n = strtol(argv[1], NULL, 10);
        max = strtol(argv[2], NULL, 10);
        thread_count = strtol(argv[3], NULL, 10);
        setup(n, max);

        for (i = 0; i < n; i++){
                printf("id:%d x:%d y:%d\n", i, data[i][0], data[i][1]);
        }
	/*
        printf("Distance between points 0 and 1: %f\n", distance(data[0][0], data[0][1], data[1][0], data[1][1]));
        */

        par_canopy_cluster(n, max/4, max/10, thread_count);
        return 0;
}

