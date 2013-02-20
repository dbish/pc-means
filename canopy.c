#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
#include <math.h>


/*
 * generate 2-d data points for testing
 * int a: upper range
 * int b: lower range
 * int n: number of data points
 */
int* practice_data(int a, int b, int n){
	int x, y, i;
	int **points = (int **)malloc(n*sizeof(int *));
	
	//randomly generate 2-d data points within the a, b range 
	//on the x and y axis
	for (i = 0; i < n; i++){
		points[i] = (int *)malloc(2*sizeof(int));
		
	}
}

/*
 * Distance estimation algorithm used by Canopy cluster
 * 
 */
int distance_estimate(){
	int dist = 0;

	//calculate the estimate

	return dist;
}

/* 
 *Determine canopy centers
 * 	int x: smaller threshold
 *      int y: larger threshold
 */
void generate_canopies(int x, int y){
	
}

/*
 * Cluster points using final canopy centers
 *
 */
void cluster(){

}

//main method used for testing
int main(int argc, char* argv[]){
	

	return 0;
}
