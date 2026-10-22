// #include<stdio.h>
// #include<sys/time.h>
// #include <stdlib.h>
// #include "../../LibShalom/NN_LIB/LibShalom.h"

// static double gtod_ref_time_sec = 0.0;

// double dclock()
// {
//         double the_time, norm_sec;
//         struct timeval tv;

//         gettimeofday( &tv, NULL );

//         if ( gtod_ref_time_sec == 0.0 )
//                 gtod_ref_time_sec = ( double ) tv.tv_sec;

//         norm_sec = ( double ) tv.tv_sec - gtod_ref_time_sec;

//         the_time = norm_sec + tv.tv_usec * 1.0e-6;

//         return the_time;
// }

// int main()
// {

// 	int i,j,k;
// 	int loop= 100;
// 	long M, N, K;
//         M= N = K = 384;
//         /* row-major */
// 	double *A = ( double * ) malloc( K* M * sizeof( double));
// 	double *B = ( double * ) malloc( K* N * sizeof( double));
// 	double *C = ( double * ) malloc( M* N * sizeof( double));

// 	double drand48();
//     double start, cost;
// 	/* initialize input matrices A and B*/
// 	for ( i = 0; i < M; i++ )
// 	{
// 		for ( j = 0; j < K; j++ )
// 			A [i* K + j]= 2.0 * (double)drand48( ) - 1.0 ;
// 	}

// 	for ( i = 0; i < K; i++ )
// 	{
// 		for ( j = 0; j < N; j++ )
// 			B [i * K + j]= 2.0 * (double)drand48( ) - 1.0 ;
// 	}

// 	// warm up
// 	//perform C = A * B (B is transposed)
// 	for( i =0 ;i< 5; i++){
// 		LibShalom_dgemm(NoTrans, Trans, C, A, B, M, N, K);
//     }

//     start = dclock();
// 	for( i= 0; i< loop ;i++){
// 		LibShalom_dgemm(NoTrans, Trans, C, A, B, M, N, K);
//     }
//     printf("--------------------------\n");
//     cost =(dclock()-start)/loop;


//     double gflops = (2.0 * (double)M * (double)N * (double)K * 1.0e-09) / cost;
//     printf("m=%d n=%d k=%d time=%.9f s  GFLOP/s=%.2f\n",
//             M, N, K, cost, gflops);


// 	free(A);
// 	free(B);
// 	free(C);
// 	return 0;
// }

/****************************************************************************/
#include <stdlib.h>
#include <stdlib.h>
#include "LibShalom.h"
#include<sys/time.h>


static double gtod_ref_time_sec = 0.0;

double dclock()
{
        double the_time, norm_sec;
        struct timeval tv;

        gettimeofday( &tv, NULL );

        if ( gtod_ref_time_sec == 0.0 )
                gtod_ref_time_sec = ( double ) tv.tv_sec;

        norm_sec = ( double ) tv.tv_sec - gtod_ref_time_sec;

        the_time = norm_sec + tv.tv_usec * 1.0e-6;

        return the_time;
}
int main()
{

	int i,j,k;
	int loop= 100;
	long M, N, K;
        M= N = K = 384;
        /* row-major */
	double *A = ( double * ) malloc( K* M * sizeof( double ) );
	double *B = ( double * ) malloc( K* N * sizeof( double ) );
	double *C = ( double * ) malloc( M* N * sizeof( double ) );

	double drand48();
    double start, cost;
	/* initialize input matrices A and B*/
	for ( i = 0; i < M; i++ )
	{
		for ( j = 0; j < K; j++ )
			A [i* K + j]= 2.0 * (double)drand48( ) - 1.0 ;
	}

	for ( i = 0; i < K; i++ )
	{
		for ( j = 0; j < N; j++ )
			B [i * K + j]= 2.0 * (double)drand48( ) - 1.0 ;
	}

	// warm up
	//perform C = A * B (B is transposed)
	for( i =0 ;i< 5; i++)
		LibShalom_dgemm(NoTrans, NoTrans, C, A, B, M, N, K);

	start = dclock();
	for( i= 0; i< loop ;i++)
		LibShalom_dgemm(NoTrans, NoTrans, C, A, B, M, N, K);
    cost =(dclock()-start)/loop;

    double gflops = (2.0 * (double)M * (double)N * (double)K * 1.0e-09) / cost;
    printf("m=%d n=%d k=%d time=%.9f s  GFLOP/s=%.2f\n",
            M, N, K, cost, gflops);
	free(A);
	free(B);
	free(C);
	return 0;
}