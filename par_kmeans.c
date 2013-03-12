#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <omp.h>

/*
 * generate test data from the range
 *   0 to max and set up global vars
 * int n: number of data points
 * int max: all data is within the (0, max) range
 * int k: number of clusters
 */
void setup(int n, int max, int k, double **clusters, int **data){
        int i;
        //data = (int **)malloc(n*sizeof(int *));
	//clusters = (double **)malloc(k*sizeof(double *));	

	//generate random data for testing
        for (i = 0; i < n; i++){
                data[i] = (int *)malloc(2*sizeof(int));
		data[i][0] = (rand()%max);
		data[i][1] = (rand()%max);
        }

	//initialize clusters variable
	for (i = 0; i < k; i++){
		clusters[i] = (double *)malloc(2*sizeof(double));
	}
	
	
}


/* returns 2d euclidean distance 
 * double x0, y0: coordinates of first point: (x0, y0)
 * double x1, y1: coordinates of second point: (x1, y1)
*/

double distance(double x0, double y0, double x1, double y1){
	return sqrt((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1));
}

/* find the cluter these coordinates belong to
 * double x, y: coordinates of the point: (x, y)
 * int k: number of cluters
*/
int assign_cluster(double x, double y, int k, double **clusters){
	int i, cluster;
	double min, cur;

	min = distance(x, y, clusters[0][0], clusters[0][1]);
	cluster = 0;
	for (i = 1; i < k; i++){
		cur = distance(x, y, clusters[i][0], clusters[i][1]);
		if (cur < min){
			min = cur;
			cluster = i;
		}
	}
	return cluster;
}

/*
 * int n: number of data points
 * int k: number of clusters
 */

void kmeans(int n, int k, double **clusters, int **data){
	int i, j, count, cluster, my_num;
	double percent_change;
	int* cluster_membership;
	double **new_clusters;
	new_clusters = (double **)malloc(k*sizeof(double *));	
	cluster_membership = (int *)malloc(n*sizeof(int));
	count = 0;
	int threshold = 10;
	percent_change = 100;

	//thread variables
	int num_threads = omp_get_max_threads();
	double ***local_clusters;

	//choose first set of cluster centers
	for (i = 0; i < k; i++){
		clusters[i][0] = data[i][0]; 
		clusters[i][1] = data[i][1];
		
	}
	
	//initialize new clusters variable 
	//it has three coords: [x][y][number of members]
	for (i = 0; i < k; i++){
		new_clusters[i] = (double *)malloc(3*sizeof(double));
		new_clusters[i][0] = 0;
		new_clusters[i][1] = 0;
		new_clusters[i][2] = 0;
	}

	for (i = 0; i < n; i++){
		cluster_membership[i] = -1;
	}


	//setup local clusters storage
	local_clusters = (double***)malloc(num_threads*sizeof(double **));
	for (i = 0; i < num_threads; i++){
		local_clusters[i] = (double **)malloc(k*sizeof(double *));
		for (j = 0; j < k; j++){
			local_clusters[i][j] = (double *)malloc(3*sizeof(double));
			local_clusters[i][j][0] = 0;	
			local_clusters[i][j][1] = 0;	
			local_clusters[i][j][2] = 0;	
		}
	}

	while((percent_change > threshold) && (count < 100)){
		percent_change = 0;

		#pragma omp parallel \
			shared(n, clusters, cluster_membership, local_clusters)
		{
			my_num = omp_get_thread_num();
		
			#pragma omp for \
				private(i, j, cluster) \
				reduction(+:percent_change)
			for(i = 0; i < n; i++){
				//find the cluster it belongs to
				cluster = assign_cluster(data[i][0], data[i][1], k, clusters);
			
				//check to see if this data point changed clusters
				if (cluster != cluster_membership[i]) {
					percent_change++;
					cluster_membership[i] = cluster;
				}
		
				//track the new cluster sum
				local_clusters[my_num][cluster][0] += data[i][0];
				local_clusters[my_num][cluster][1] += data[i][1];
				local_clusters[my_num][cluster][2]++;
			} 

			//reduction on the local arrays
			for (i = 0; i < k; i++){
				for (j = 0; j < num_threads; j++){
					new_clusters[i][0] += local_clusters[j][i][0];
					new_clusters[i][1] += local_clusters[j][i][1];
					new_clusters[i][2] += local_clusters[j][i][2];
					local_clusters[j][i][0] = local_clusters[j][i][1] = local_clusters[j][i][2] = 0;
				}
			}	

			//centroid calculation
			for (i = 0; i < k; i++){
				if (new_clusters[i][2] > 0){
					new_clusters[i][0] /= new_clusters[i][2];
					new_clusters[i][1] /= new_clusters[i][2];
				}
				clusters[i][0] = new_clusters[i][0];
				clusters[i][1] = new_clusters[i][1];
				new_clusters[i][0] = new_clusters[i][1] = new_clusters[i][2] = 0;
			}
			percent_change /= n;
			count++;
		}
	}

	for (i = 0; i < n; i++){
		printf("%d %d %d\n", data[i][0], data[i][1], cluster_membership[i]);
	}
	
	
	
}


int main(int argc, char* argv[]){
	int i, n, max, k;
	double **clusters;
	int **data;
	n = strtol(argv[1], NULL, 10);
	max = strtol(argv[2], NULL, 10);
	k = strtol(argv[3], NULL, 10);
	clusters = (double **)malloc(k*sizeof(double *));	
        data = (int **)malloc(n*sizeof(int *));
	setup(n, max, k, clusters, data);

	/*for (i = 0; i < 2; i++){
		printf("id:%d x:%d y:%d\n", i, data[i][0], data[i][1]);  
	}
	printf("Distance between points 0 and 1: %f\n", distance(data[0][0], data[0][1], data[1][0], data[1][1]));
	*/
	kmeans(n, k, clusters, data);
	
	return 0;
}

