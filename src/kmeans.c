/* http://cs.smu.ca/~r_zhang/code/kmeans.c */
/*****
** kmeans.c
** - a simple k-means clustering routine
** - returns the cluster labels of the data points in an array
** - here's an example
**   extern int *k_means(double**, int, int, int, double, double**);
**   ...
**   int *c = k_means(data_points, num_points, dim, 20, 1e-4, 0);
**   for (i = 0; i < num_points; i++) {
**      printf("data point %d is in cluster %d\n", i, c[i]);
**   }
**   ...
**   free(c);
** Parameters
** - array of data points (double **data)
** - number of data points (int n)
** - dimension (int m)
** - desired number of clusters (int k)
** - error tolerance (double t)
**   - used as the stopping criterion, i.e. when the sum of
**     squared euclidean distance (standard error for k-means)
**     of an iteration is within the tolerable range from that
**     of the previous iteration, the clusters are considered
**     "stable", and the function returns
**   - a suggested value would be 0.0001
** - output address for the final centroids (double **centroids)
**   - user must make sure the memory is properly allocated, or
**     pass the null pointer if not interested in the centroids
** References
** - J. MacQueen, "Some methods for classification and analysis
**   of multivariate observations", Fifth Berkeley Symposium on
**   Math Statistics and Probability, 281-297, 1967.
** - I.S. Dhillon and D.S. Modha, "A data-clustering algorithm
**   on distributed memory multiprocessors",
**   Large-Scale Parallel Data Mining, 245-260, 1999.
** Notes
** - this function is provided as is with no warranty.
** - the author is not responsible for any damage caused
**   either directly or indirectly by using this function.
** - anybody is free to do whatever he/she wants with this
**   function as long as this header section is preserved.
** Created on 2005-04-12 by
** - Roger Zhang (rogerz@cs.dal.ca)
** Modifications
** -
** Last compiled under Linux with gcc-3
*/
#include "ribs_defs.h"
#include "kmeans.h"
#include "vmbuf.h"
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <math.h>

int *k_means(double **data, int n, int m, int k, double t, double **centroids) {
   /* output cluster label for each data point */
   static struct vmbuf labels_buf = VMBUF_INITIALIZER;
   vmbuf_init(&labels_buf, 4096);
   int *labels = memset(vmbuf_allocptr(&labels_buf, n * sizeof(int)), 0, n * sizeof(int));
   int h, i, j; /* loop counters, of course :) */
   static struct vmbuf counts_buf = VMBUF_INITIALIZER;
   vmbuf_init(&counts_buf, 4096);
   /* size of each cluster */
   int *counts = memset(vmbuf_allocptr(&counts_buf, k * sizeof(int)), 0, k * sizeof(int));

   double old_error, error = DBL_MAX; /* sum of squared euclidean distance */
   static struct vmbuf centroids_buf = VMBUF_INITIALIZER;
   static struct vmbuf temp_centroids_buf = VMBUF_INITIALIZER;
   vmbuf_init(&centroids_buf, 4096);
   vmbuf_init(&temp_centroids_buf, 4096);
   size_t c_ptr_table_size = k * sizeof(double *);
   size_t c_vect_size = m * sizeof(double);
   void *centroids_buf_ptr = vmbuf_allocptr(&centroids_buf, c_ptr_table_size + (k * c_vect_size));
   void *temp_centroids_buf_ptr = vmbuf_allocptr(&temp_centroids_buf, c_ptr_table_size + (k * c_vect_size));
   double **c = centroids ? centroids : centroids_buf_ptr;
   double **c1 = temp_centroids_buf_ptr;
   void *centroids_buf_ptr_data = centroids_buf_ptr + c_ptr_table_size;
   void *temp_centroids_buf_ptr_data = temp_centroids_buf_ptr + c_ptr_table_size;

   assert(data && k > 0 && k <= n && m > 0 && t >= 0); /* for debugging */

   /****
   ** initialization */

   for (h = i = 0; i < k; h += n / k, ++i) {
        c1[i] = temp_centroids_buf_ptr_data + i * c_vect_size;
        if (!centroids)
            c[i] = centroids_buf_ptr_data + i * c_vect_size;
        /* pick k points as initial centroids */
        for (j = m; j-- > 0; c[i][j] = data[h][j]);
   }

   /****
   ** main loop */

   do {
      /* save error from last step */
      old_error = error, error = 0;

      /* clear old counts and temp centroids */
      for (i = 0; i < k; counts[i++] = 0) {
         for (j = 0; j < m; c1[i][j++] = 0);
      }

      for (h = 0; h < n; h++) {
         /* identify the closest cluster */
         double min_distance = DBL_MAX;
         for (i = 0; i < k; i++) {
            double distance = 0;
            for (j = m; j-- > 0; distance += pow(data[h][j] - c[i][j], 2));
            if (distance < min_distance) {
               labels[h] = i;
               min_distance = distance;
            }
         }
         /* update size and temp centroid of the destination cluster */
         for (j = m; j-- > 0; c1[labels[h]][j] += data[h][j]);
         counts[labels[h]]++;
         /* update standard error */
         error += min_distance;
      }

      for (i = 0; i < k; i++) { /* update all centroids */
         for (j = 0; j < m; j++) {
            c[i][j] = counts[i] ? c1[i][j] / counts[i] : c1[i][j];
         }
      }

   } while (fabs(error - old_error) > t);

   return labels;
}

