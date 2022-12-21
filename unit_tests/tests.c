#include <stdio.h>
#include <stdlib.h>

int coord_test_ll2utm(void);
int pgd_inversion_test(void);
int pgd_inversion_test2(void);
int cmopad_test(int verb);
int readCoreInfo_test(void);
int cmt_greens_test(void);
int cmt_inversion_test(void);
int ff_greens_test(void);
int ff_meshPlane_test(void);
int ff_regularizer_test(void);
int ff_inversion_test(void);

int main()
{
    int ierr;

    //------------------------------------------------------------------------//
    // test the vital core utilities.  if any of these fail then the program  //
    // will fail                                                              //
    //------------------------------------------------------------------------//
    printf("%s: Beginning core tests...\n", __func__);
    ierr = readCoreInfo_test();
    if (ierr != 0)
    {
        printf("%s: Failed the coreInfo test\n", __func__);
        return EXIT_FAILURE;
    }

/*
    if (ierr != 0)
    {
        printf("%s: Failed the least-squares test!\n", __func__);
        return EXIT_FAILURE;
    }
*/

    ierr = coord_test_ll2utm();
    if (ierr != 0)
    {
        printf("%s: Failed the latlon 2 utm test!\n", __func__);
        return EXIT_FAILURE;
    }

/*
    ierr = cmopad_test(0);
    if (ierr != 0)
    {
        printf("%s: Failed MT decomposition!\n", __func__);
        return EXIT_FAILURE;
    }
*/

    //------------------------------------------------------------------------//
    // test the forward modeling and regularization matrices.  if any of      //
    // these fail the inversions will fail.                                   //
    //------------------------------------------------------------------------//
    printf("\n%s: Beginning matrix generation tests...\n", __func__);
    ierr = cmt_greens_test();
    if (ierr != 0)
    {
        printf("%s: Failed to compute Greens functions!\n", __func__);
        return EXIT_FAILURE;
    }

    ierr = ff_greens_test();
    if (ierr != 0)
    {   
        printf("%s: Failed the ff greens test\n", __func__);
        return EXIT_FAILURE;
    }

    ierr = ff_meshPlane_test();
    if (ierr != 0)
    {
        printf("%s: FAiled to meshPlane test\n", __func__);
        return EXIT_FAILURE;
    }

    ierr = ff_regularizer_test();
    if (ierr != 0)
    {
        printf("%s: Failed the ff regularizer test\n", __func__);
        return EXIT_FAILURE;
    }

    //------------------------------------------------------------------------//
    // verify the inversions.                                                 //
    //------------------------------------------------------------------------//
    printf("\n%s: Beginning inversions tests...\n", __func__);
    ierr = pgd_inversion_test();
    if (ierr != 0)
    {
        printf("%s: Failed PGD inversion test!\n", __func__);
        return EXIT_FAILURE;
    }
    ierr = pgd_inversion_test2();
    if (ierr != 0)
    {
        printf("%s: Failed PGD inversion test2\n", __func__);
        return EXIT_FAILURE;
    }

    ierr = cmt_inversion_test();
    if (ierr != 0)
    {
        printf("%s: Failed CMT inversion test\n", __func__);
        return EXIT_FAILURE;
    }

    ierr = ff_inversion_test();
    if (ierr != 0)
    {
        printf("%s: Failed FF Inversion test\n", __func__);
        return EXIT_FAILURE;
    }
    printf("%s: All tests passed\n", __func__);
    return EXIT_SUCCESS;
}
