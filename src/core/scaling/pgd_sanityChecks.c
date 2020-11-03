#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gfast_core.h"

void swap_floats(float *, float *);
void swap_ints(int *, int *);
void selectionSort(float unsorted[], int n, int mth_indx[]);

/*!
 * @brief Computes the right hand side in the peak ground displacement 
 *        estimation s.t.
 *        \f$ \textbf{b} = \left \{ \log_{10}(d) - A \right \} \f$
 *        where A is a scalar shift and d is the distance at each station.
 *
 * @param[in] n         number of points
 * @param[in] dist_tol  distance tolerance - if d is less than this then
 *                      it will be set to a default value (cm)
 * @param[in] dist_def  distance default value (cm)
 * @param[in] A         shift so that b = log10(d) - A
 * @param[in] d         max distance (cm) at each site [n]
 *
 * @param[out] b        right hand side in Gm = b [n]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker (ISTI)
 *
 */
int core_scaling_pgd_sanityChecks(const int n,
                            const double dist_tol,
                            const double dist_def,
                            const double *__restrict__ repi,
                            const double *__restrict__ d
                            )
{
    double dist;
    int i;
    if (n < 1)
    {
        LOG_ERRMSG("Invalid number of points: %d", n);
        return -1;
    }
    for (i=0; i<n; i++)
    {
        dist = d[i];
        if (dist - dist_tol < 0.0){
          LOG_MSG("Warning: d[%d]=%f < dist_tol=%f !!!", i, d[i], dist_tol);
        }
        //if (dist - dist_tol < 0.0){dist = dist_def;}
    }

    int index_sort[n];
    for (i=0; i < n; i++){
      index_sort[i]=i;
    }

    LOG_MSG("%s", "Unsorted PGD distance (km) and amps (cm):");
    for (i=0; i < n; i++){
      LOG_MSG("%02d %6.2f ", i, repi[i]);
    }
    //LOG_MSG("\n ");
    for (i=0; i < n; i++){
      LOG_MSG("%02d %6.2f ", i, d[i]);
    }
    //LOG_MSG("\n ");

    selectionSort(repi, n, mth_indx);

    LOG_MSG("%s", "Sorted PGD amps by increasing distance:");
    for (i=0; i < n; i++){
      LOG_MSG("%02d %6.2f ", i, repi[index_sort[i]]);
    }
    //LOG_MSG("\n ");
    for (i=0; i < n; i++){
      LOG_MSG("%02d %6.2f ", i, d[index_sort[i]]);
    }
    //LOG_MSG("\n ");

  return 0;
}



void swap_floats(float* xp, float* yp)
{
  float temp = *xp;
  *xp = *yp;
  *yp = temp;
}

void swap_ints(int* xp, int* yp)
{
  int temp = *xp;
  *xp = *yp;
  *yp = temp;
}

void selectionSort(float unsorted[], int n, int mth_indx[])
{
  int i, j, min_idx;
  float sorted[n];
  for (i=0; i < n; i++){
    sorted[i]=unsorted[i];
  }

// One by one move boundary of unsorted subarray
  for (i = 0; i < n - 1; i++) {
  // Find the minimum element in unsorted array
    min_idx = i;
    for (j = i + 1; j < n; j++)
      if (sorted[j] < sorted[min_idx])
        min_idx = j;
    // Swap the found minimum element with the first element
    //LOG_MSG("swap a[%d]=%f with a[%d]=%f\n", min_idx, sorted[min_idx], i, sorted[i]);
    swap_floats(&sorted[min_idx], &sorted[i]);
    swap_ints(&mth_indx[min_idx], &mth_indx[i]);
  }
}

