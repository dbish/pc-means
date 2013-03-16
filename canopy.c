#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

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
void setup(int n, int max, int k){
        int i;
        data = (int **)malloc(n*sizeof(int *));

        //generate random data for testing
        for (i = 0; i < n; i++){
                data[i] = (int *)malloc(2*sizeof(int));
                data[i][0] = (rand()%max);
                data[i][1] = (rand()%max);
        }

}

void canopy_cluster(int n, int x_1, int x_2){
	int c_idx, size, center, cur, i, j, dist_est;

	Stack *data_stack, *temp_stack, *t;
	data_stack = (Stack*)malloc(sizeof(Stack));
	data_stack->tail = -1;
	data_stack->size = 0;
	data_stack->values = (int *)malloc(n*sizeof(int));
	temp_stack = (Stack*)malloc(sizeof(Stack));
	temp_stack->tail = -1;
	temp_stack->size = 0;
	temp_stack->values = (int *)malloc(n*sizeof(int));
	
	canopy_size = (int *)malloc((n+1)*sizeof(int));
	//store the number of canopies in the first element
	canopy_size[0] = 0;

	canopies = (int **)malloc(n*sizeof(int *));
	c_idx = 0;

	//copy the values to cluster into the data stack
	for (i = 0; i < n; i++){
		push(data_stack, i);
	}

	while(data_stack->size > 0){
		center = pop(data_stack);
		printf("center: %d\n", center);
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
		printf("current number of items in this canopy %d stored at index %d\n", size, canopy_size[0]);
		/*swap stacks to continue building canopies*/
		t = data_stack;
		data_stack = temp_stack;
		temp_stack = t;
		c_idx++;
	}


	printf("number of canopies: %d\n", canopy_size[0]);
	for (i = 1; i <= canopy_size[0]; i++){
		printf("canopy %d has %d items: ", i, canopy_size[i]);
		for (j = 0; j < canopy_size[i]; j++){
			printf("[%d] ",canopies[i-1][j]);
		}
		printf("\n");
	}
}

int main(int argc, char* argv[]){
        int i, n, max, k;
        n = strtol(argv[1], NULL, 10);
        max = strtol(argv[2], NULL, 10);
        setup(n, max, k);

        for (i = 0; i < n; i++){
                printf("id:%d x:%d y:%d\n", i, data[i][0], data[i][1]);
        }
	/*
        printf("Distance between points 0 and 1: %f\n", distance(data[0][0], data[0][1], data[1][0], data[1][1]));
        */

        canopy_cluster(n, max/4, max/10);
        return 0;
}

