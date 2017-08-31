#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdbool.h>
#include "gfast_config.h"
#ifdef GFAST_USE_INTEL
#include <mkl_lapacke.h>
#include <mkl_cblas.h>
#else
#include <cblas.h>
#include <lapacke.h>
#include <lapacke_utils.h>
#endif
#include <png.h>
#include "beachball.h"
#include "compearth.h"
//#include "cmopad.h"
//#include "pixmap.h"
//#include "pixmap_png.h"
//#include "pixmap_jpg.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif
#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif
#define PIXMAP_COLORS 3

static void vecstrd(const double str, const double dip, double r[3]);
static void strdv(const double r[3], double *str, double *dip);
static void crossp(const double *__restrict__ u,
                   const double *__restrict__ v,
                   double *__restrict__ n);
static void strdvec(double r[3], double *str, double *dip);
static void rotmat(const int naxis, const double theta,
                   double *__restrict__ r,
                   double *__restrict__ rt);
static void isItNear(const double *__restrict__ pt, 
                     double *howClose,
                     double *__restrict__ ptold);
static double dot3(const double *__restrict__ x,
                   const double *__restrict__ y);
static void gemv3(const double *__restrict__ A, const double *__restrict__ x,
                  double *__restrict__ y);
static double transform(const double a, const double b,
                        const double c, const double d,
                        const double x);

int beachball_setMomentTensor(const double m11, const double m22,
                              const double m33, const double m12,
                              const double m13, const double m23,
                              const enum compearthCoordSystem_enum basis,
                              struct beachballPlot_struct *beachball)
{
    //struct cmopad_struct cmt;
    //double mt[3][3];
    double m9[9], eigs[3];
    double m6[6], fp1[3], fp2[3], clvdPct, dcPct, devPct, isoPct, M0, Mw;
    int ierr, info;
    bool lexplosion, limplosion;
    //memset(&cmt, 0, sizeof(struct cmopad_struct));
    memset(beachball->t_axis, 0, 3*sizeof(double));
    memset(beachball->null_axis, 0, 3*sizeof(double));
    memset(beachball->p_axis, 0, 3*sizeof(double));
    // the edge case is for purely isotropic sources
    lexplosion = false;
    limplosion = false;
    m9[0] = m11;
    m9[1] = m12;
    m9[2] = m13;
    m9[3] = m12;
    m9[4] = m22;
    m9[5] = m23;
    m9[6] = m13;
    m9[7] = m23;
    m9[8] = m33;
    info = LAPACKE_dsyev(LAPACK_COL_MAJOR, 'N', 'L', 3, m9, 3, eigs);
    if (info != 0)
    {
        printf("Failed classifying eigenvalues\n");
    }
    if ((eigs[0] > 0.0 && eigs[1] > 0.0 && eigs[2] > 0.0))
    {   
        //printf("Source is pure explosion\n"); 
        lexplosion = true;
    }
    if ((eigs[0] < 0.0 && eigs[1] < 0.0 && eigs[2] < 0.0))
    {   
        printf("Source is pure implosion\n");
        limplosion = true;
    }
    // perform the decomposition 
    m6[0] = m11;
    m6[1] = m22;
    m6[2] = m33;
    m6[3] = m12;
    m6[4] = m13;
    m6[5] = m23;
    ierr = compearth_standardDecomposition(1, m6, basis,
                                           &M0, &Mw, fp1, fp2,
                                           beachball->p_axis,
                                           beachball->null_axis,
                                           beachball->t_axis,
                                           &isoPct, &devPct, &dcPct, &clvdPct);
/*
    // copy the input moment tensor
    mt[0][0] = m11;
    mt[1][1] = m22;
    mt[2][2] = m33;
    mt[0][1] = mt[1][0] = m12;
    mt[0][2] = mt[2][0] = m13;
    mt[1][2] = mt[2][1] = m23;
    ierr = cmopad_basis_transformMatrixM33(mt, basis, NED);
    if (ierr != 0)
    {
        printf("Error transforming matrix\n");
        return -1;
    }
    // decompose the moment tensor
    ierr = cmopad_standardDecomposition(mt, &cmt);
    if (ierr != 0)
    {
        printf("Failed initializing moment tensor from strike, dip, rake\n");
        return -1;
    }
    // get the principal tension, null, and plunge axes 
    ierr = cmopad_MT2PrincipalAxisSystem(0, &cmt);
    if (ierr != 0)
    {
        printf("Error computing principal axes\n");
        return -1;
    }
    if (lexplosion || limplosion)
    {
        cmt.p_axis[2] = cmt.eigenvalues[0];
        cmt.null_axis[2] = cmt.eigenvalues[1];
        cmt_axis[2] = cmt.eigenvalues[2];
    }
    else
    {
        // Convert pressure, null, and tension principal axes to
        // azimuth, plunge, and length
        ierr += cmopad_Eigenvector2PrincipalAxis(NED, cmt.eig_pnt[0],
                                                 cmt.p_axis,    cmt.p_axis);
        ierr += cmopad_Eigenvector2PrincipalAxis(NED, cmt.eig_pnt[1],
                                                 cmt.null_axis, cmt.null_axis);
        ierr += cmopad_Eigenvector2PrincipalAxis(NED, cmt.eig_pnt[2],
                                                 cmt.t_axis,    cmt.t_axis);
        if (ierr != 0)
        {
            printf("Error converting eigenvector to principal axis!\n");
            return -1;
        }
    }
    memcpy(&beachball->cmt, &cmt, sizeof(struct cmopad_struct));
*/
    beachball->lhaveMT = true;
    return 0;
}

int beachball_writePNG(const char *fileName,
                       size_t height, size_t width, unsigned char *bytes)
{
    FILE *file = NULL;
    png_structp png = NULL;
    png_infop info = NULL;
    png_bytepp rows = NULL;
    int h;

    file = fopen(fileName, "wb");
    if (file == NULL){printf("failed to open file\n");}
    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png == NULL){printf("error making png struct\n");}
    info = png_create_info_struct(png); 
    if (info == NULL){printf("error making png info struct\n");}
    png_init_io(png, file);
    png_set_IHDR(png, info, width, height, 8,
                 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png, info);
    rows = calloc(height, sizeof(png_bytep));
    for (h=0; h<(int) height; h++)
    {
        rows[h] = bytes + (PIXMAP_COLORS*h*(int) width);
    }
    png_write_rows(png, rows, height);
    png_write_end(png, NULL); //info);
    png_destroy_write_struct(&png, &info);
    if (rows != NULL){free(rows);}
    fclose(file);
    return 0;
}

int beachball_draw(const char *fileName,
                   struct beachballPlot_struct beachball)
{
    const char *fcnm = "beachball_draw\0";
    struct beachBall_struct beachImage;
    unsigned char *image;
    double baz, bdp, bev, paz, pdp, pev, taz, tdp, tev, x, xnorm, y;
    int i, i1, i2, indx, ix, iy, jy, k;
    size_t npixel;
    taz = beachball.t_axis[0];
    tdp = beachball.t_axis[1];
    tev = beachball.t_axis[2];
    baz = beachball.null_axis[0];
    bdp = beachball.null_axis[1];
    bev = beachball.null_axis[2];
    paz = beachball.p_axis[0];
    pdp = beachball.p_axis[1];
    pev = beachball.p_axis[2];
    // normalize the moment tensor for my sanity
    xnorm = MAX(fabs(tev), MAX(fabs(bev), fabs(pev)));
    if (xnorm == 0.0)
    {
        printf("%s: Error normalization is zero %f %f %f\n", fcnm, tev, bev, pev);
        return -1; 
    }
    // compute everything describing the beachball 
    memset(&beachImage, 0, sizeof(struct beachBall_struct));
    cliffsNodes_beachBallP(beachball.xc, beachball.yc, beachball.rad,
                           taz, tdp, tev,
                           baz, bdp, bev,
                           paz, pdp, pev,
                           beachball.nNodalLine,
                           beachball.nFocalSphere,
                           beachball.nxPixel, beachball.nyPixel,
                           true, true, true,
                           &beachImage);
    // draw the image
    npixel = (size_t) (beachball.nxPixel*beachball.nyPixel);
    image = (unsigned char *)
            calloc(PIXMAP_COLORS*npixel, sizeof(unsigned char));
    // Set a white background
    for (indx=0; indx<3*beachball.nxPixel*beachball.nyPixel; indx++)
    {
        image[indx] = 255;
    } 
    // Draw the polarity
    if (beachball.lwantPolarity)
    {    
        for (i=0; i<beachImage.npolarity; i++) 
        {
            x = transform(beachball.xc - beachball.rad,
                          beachball.xc + beachball.rad,
                          0, (double) (beachball.nxPixel-1),
                          beachImage.xPolarity[i]);
            y = transform(beachball.yc - beachball.rad,
                          beachball.yc + beachball.rad,
                          0, (double) (beachball.nyPixel-1),
                          beachImage.yPolarity[i]);
            ix = (int) (x + 0.5);
            iy = (int) (y + 0.5);
            ix = MAX(0, MIN(ix, beachball.nxPixel - 1)); 
            iy = MAX(0, MIN(iy, beachball.nyPixel - 1)); 
            jy = beachball.nyPixel - 1 - iy;
            if (beachImage.polarity[i] ==-1)
            {
                // white
                image[3*(jy*beachball.nxPixel+ix)+0] = 255; 
                image[3*(jy*beachball.nxPixel+ix)+1] = 255; 
                image[3*(jy*beachball.nxPixel+ix)+2] = 255; 
            }
            else
            {
                // red
                image[3*(jy*beachball.nxPixel+ix)+0] = 255; 
                image[3*(jy*beachball.nxPixel+ix)+1] = 0; 
                image[3*(jy*beachball.nxPixel+ix)+2] = 0; 
            }
        }
    }
    // Add the nodal lines
    if (beachball.lwantNodalLines)
    {
        // draw first nodal line
        for (k=0; k<beachImage.nNodalLineSegs1; k++)
        {
            i1 = beachImage.nodalLine1Ptr[k];
            i2 = beachImage.nodalLine1Ptr[k+1];
            for (i=i1; i<i2; i++)
            {
                // start point in vector (line)
                x = transform(beachball.xc - beachball.rad,
                              beachball.xc + beachball.rad,
                              0, (double) (beachball.nxPixel-1),
                              beachImage.xNodalLine1[2*i]);
                y = transform(beachball.yc - beachball.rad,
                              beachball.yc + beachball.rad,
                              0, (double) (beachball.nyPixel-1),
                              beachImage.yNodalLine1[2*i]);
                ix = (int) (x + 0.5);
                iy = (int) (y + 0.5);
                ix = MAX(0, MIN(ix, beachball.nxPixel - 1));
                iy = MAX(0, MIN(iy, beachball.nyPixel - 1));
                jy = beachball.nyPixel - 1 - iy;
                // black
                image[3*(jy*beachball.nxPixel+ix)+0] = 0;
                image[3*(jy*beachball.nxPixel+ix)+1] = 0;
                image[3*(jy*beachball.nxPixel+ix)+2] = 0;
                // end point in vector (line)
                x = transform(beachball.xc - beachball.rad,
                              beachball.xc + beachball.rad,
                              0, (double) (beachball.nxPixel-1),
                              beachImage.xNodalLine1[2*i+1]);
                y = transform(beachball.yc - beachball.rad,
                              beachball.yc + beachball.rad,
                              0, (double) (beachball.nyPixel-1),
                              beachImage.yNodalLine1[2*i+1]);
                ix = (int) (x + 0.5);
                iy = (int) (y + 0.5);
                ix = MAX(0, MIN(ix, beachball.nxPixel - 1));
                iy = MAX(0, MIN(iy, beachball.nyPixel - 1));
                jy = beachball.nyPixel - 1 - iy;
                // black
                image[3*(jy*beachball.nxPixel+ix)+0] = 0;
                image[3*(jy*beachball.nxPixel+ix)+1] = 0;
                image[3*(jy*beachball.nxPixel+ix)+2] = 0;
            }
        }
        // draw second nodal line
        for (k=0; k<beachImage.nNodalLineSegs2; k++) 
        {
            i1 = beachImage.nodalLine2Ptr[k];
            i2 = beachImage.nodalLine2Ptr[k+1];
            for (i=i1; i<i2; i++) 
            {
                // start point in vector (line)
                x = transform(beachball.xc - beachball.rad,
                              beachball.xc + beachball.rad,
                              0, (double) (beachball.nxPixel-1),
                              beachImage.xNodalLine2[2*i]);
                y = transform(beachball.yc - beachball.rad,
                              beachball.yc + beachball.rad,
                              0, (double) (beachball.nyPixel-1),
                              beachImage.yNodalLine2[2*i]);
                ix = (int) (x + 0.5);
                iy = (int) (y + 0.5);
                ix = MAX(0, MIN(ix, beachball.nxPixel - 1)); 
                iy = MAX(0, MIN(iy, beachball.nyPixel - 1)); 
                jy = beachball.nyPixel - 1 - iy;
                // black
                image[3*(jy*beachball.nxPixel+ix)+0] = 0; 
                image[3*(jy*beachball.nxPixel+ix)+1] = 0; 
                image[3*(jy*beachball.nxPixel+ix)+2] = 0; 
                // end point in vector (line)
                x = transform(beachball.xc - beachball.rad,
                              beachball.xc + beachball.rad,
                              0, (double) (beachball.nxPixel-1),
                              beachImage.xNodalLine2[2*i+1]);
                y = transform(beachball.yc - beachball.rad,
                              beachball.yc + beachball.rad,
                              0, (double) (beachball.nyPixel-1),
                              beachImage.yNodalLine2[2*i+1]);
                ix = (int) (x + 0.5);
                iy = (int) (y + 0.5);
                ix = MAX(0, MIN(ix, beachball.nxPixel - 1)); 
                iy = MAX(0, MIN(iy, beachball.nyPixel - 1)); 
                jy = beachball.nyPixel - 1 - iy;
                // black
                image[3*(jy*beachball.nxPixel+ix)+0] = 0; 
                image[3*(jy*beachball.nxPixel+ix)+1] = 0; 
                image[3*(jy*beachball.nxPixel+ix)+2] = 0; 
            }
        }
    }
    // Draw an outline
    for (i=0; i<beachImage.nFocalSphere; i++)
    {
        x = transform(beachball.xc - beachball.rad,
                      beachball.xc + beachball.rad,
                      0, (double) (beachball.nxPixel-1),
                      beachImage.xFocalSphere[i]);
        y = transform(beachball.yc - beachball.rad,
                      beachball.yc + beachball.rad,
                      0, (double) (beachball.nyPixel-1),
                      beachImage.yFocalSphere[i]);
        ix = (int) (x + 0.5);
        iy = (int) (y + 0.5);
        ix = MAX(0, MIN(ix, beachball.nxPixel - 1));
        iy = MAX(0, MIN(iy, beachball.nyPixel - 1));
        jy = beachball.nyPixel - 1 - iy;
        // black
        image[3*(jy*beachball.nxPixel+ix)+0] = 0;
        image[3*(jy*beachball.nxPixel+ix)+1] = 0;
        image[3*(jy*beachball.nxPixel+ix)+2] = 0;
    }
    // Write it
    beachball_writePNG(fileName,
                       (size_t) beachball.nxPixel, (size_t) beachball.nyPixel,
                       image);
    free(image);
    cliffsNodes_free(&beachImage);
    return 0;
}

int beachball_plotDefaults(struct beachballPlot_struct *beachball)
{
    memset(beachball, 0, sizeof(struct beachballPlot_struct));
    beachball->rad = 1.0;
    beachball->xc = 1.5;
    beachball->yc = 1.5;
    beachball->nxPixel = 251;
    beachball->nyPixel = 251;
    beachball->nNodalLine = 3*beachball->nxPixel;
    beachball->nFocalSphere = 4*beachball->nxPixel;   
    beachball->lwantPolarity = true;
    beachball->lwantNodalLines = true;
    beachball->lhaveMT = false;
    return 0;
}

static double transform(const double a, const double b,
                        const double c, const double d,
                        const double x)
{
    double det, c1, c2, xi;
    det = 1.0/(b - a);
    c1 = det*(b*c - a*d);
    c2 = det*(-c + d);
    xi = c1 + x*c2;
    return xi;
}
//============================================================================//
int cliffsNodes_plotStations(const double xc, const double yc, const double rad,
                             const int nstat, 
                             const double *__restrict__ az, 
                             const double *__restrict__ toa,
                             double *__restrict__ xp, 
                             double *__restrict__ yp);

/*!
 * @brief Computes the station locations for the given azimuth
 *        and take-off angle on a focal sphere with radius rad
 *        centered at (xc, yc).  This is based on Cliff Frohlich's
 *        stnPlt but now processes multiple stations and does no plotting.
 *
 * @param[in] xc     x center of focal sphere
 * @param[in] yc     y center of focal sphere
 * @param[in] rad    radius of sphere
 * @param[in] nstat  number of stations
 * @param[in] az     source station azimuths (degrees) [nstat]
 * @param[in] toa    take off angle (degrees) from source to stations [nstat]
 *
 * @param[out] xp    station's x location on focal sphere [nstat]
 * @param[out] yp    station's y location on focal sphere [nstat]
 *
 * @author Ben Baker
 *
 * @copyright BSD 
 *
 */
int cliffsNodes_plotStations(const double xc, const double yc, const double rad,
                             const int nstat, 
                             const double *__restrict__ az,
                             const double *__restrict__ toa,
                             double *__restrict__ xp,
                             double *__restrict__ yp)
{
    const char *fcnm = "cliffsNodes_plotStations\0";
    double azrad, rp, sign, toaN;
    int i;
    const double sqrt2 = 1.4142135623730951;
    const double degrad = M_PI/180.0;
    //------------------------------------------------------------------------//
    //
    // error handling
    if (nstat < 1 || az == NULL || toa == NULL || xp == NULL || yp == NULL)
    {
        if (nstat < 1){printf("%s: No stations\n", fcnm);}
        if (az == NULL){printf("%s: az is NULL\n", fcnm);}
        if (toa == NULL){printf("%s: toa is NULL\n", fcnm);}
        if (xp == NULL){printf("%s: xp is NULL\n", fcnm);}
        if (yp == NULL){printf("%s: yp is NULL\n", fcnm);}
        return -1;
    }
    // compute point on focal shere for each station
    for (i=0; i<nstat; i++)
    {
        // ray normally goes down
        sign = 1.0;
        toaN = toa[i];
        // ray is going up
        if (toaN > 90.)
        {
            toaN = 180. - toa[i];
            sign =-1.;
        }
        rp = sign*rad*sqrt2*sin(0.5*degrad*toaN);
        azrad = az[i]*degrad;
        xp[i] = xc + rp*sin(azrad);
        yp[i] = yc + rp*cos(azrad);
    }
    return 0;
}
//============================================================================//
/*!
 * @brief Given the azimuth and dip of the tension and pressure eigenvectors
 *        in geographic coordinates along with normalized eigenvalues
 *        this computes the moment tensort in geographic coordinates as
 *        well as the rotation and transpose rotation matrices which rotate
 *        from a system where t=1-axis and p=2-axis.  This is based on 
 *        Cliff Frohlich's mtensor.
 *
 * @param[in] taz    tension axis azimuth (degrees)
 * @param[in] tdp    tension axis dip (degrees)
 * @param[in] paz    pressure axis azimuth (degrees)
 * @param[in] pdp    pressure axis dip
 * @param[in] tev1   normalized eigenvalue for tension eigenvector
 * @param[in] bev1   normalized eigenvalue for null eigenvector 
 * @param[in] pev1   normalized eigenvalue for pressure eigenvector 
 *
 * @param[out] mt    [3 x 3] moment tensor (column major)
 * @param[out] rm    [3 x 3] rotation matrix as described above (column major)
 * @param[out] rmt   [3 x 3] transpose rotation matrix described above 
 *                   (column major)
 * @author Ben Baker
 *
 * @copyright BSD
 *
 */
void cliffsNodes_mtensor(const double taz, const double tdp,
                         const double paz, const double pdp,
                         const double tev1, const double bev1,
                         const double pev1,
                         double *__restrict__ mt,
                         double *__restrict__ rm,
                         double *__restrict__ rmt,
                         double *ampMax)
{                             
    double mpt[9], sm1[9], smt1[9], sm2[9], smt2[9],  sm3[9],
           smt3[9], work[9], p[3], pnew[3], pxt[3], t[3], pang;
    const double degrad = M_PI/180.0;
    // set up moment tensor in t-p system
    memset(mpt, 0, 9*sizeof(double));
    mpt[0] = tev1;
    mpt[4] = pev1;
    mpt[8] = bev1;
    // condition p-axis so that it really is perpendicular to t-axis
    vecstrd(taz, tdp, t);
    vecstrd(paz, pdp, p);
    crossp(p, t, pxt);
    crossp(t, pxt, p);
    // Step 1: rotate 1-axis about 3-axis so that 1-axis lies along t azimuth
    rotmat(3, taz-90., sm1, smt1);
    // Step 2: rotate 1-axis about 2-axis so that 1-axis liesalong t
    rotmat(2, tdp, sm2, smt2);
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, 3, 3, 3,
                1.0, sm2, 3, sm1, 3, 0.0, sm3, 3); 
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, 3, 3, 3,
                1.0, smt1, 3, smt2, 3, 0.0, smt3, 3);
    // in this system, p should lie in 2-3 plane
    cblas_dgemv(CblasColMajor, CblasNoTrans, 3, 3,
                1.0, sm3, 3, p, 1, 0.0, pnew, 1); 
    // Step 3: rotate about 1 axis so that 2 axis lies along p
    pang = atan2(pnew[2], pnew[1])/degrad; //(3.14159/180.)
    rotmat(1, -pang, sm1, smt1);
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, 3, 3, 3,
                1.0, sm1, 3, sm3, 3, 0.0, rm, 3); 
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, 3, 3, 3,
                1.0, smt3, 3, smt1, 3, 0.0, rmt, 3);
    // Finally: find moment tensor in space coordinates.
    // This is because Mpt=RM*Mspace*RMT, thus Mspace=RMT*Mpt*RM
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, 3, 3, 3,
                1.0, rmt, 3, mpt, 3, 0.0, work, 3);
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, 3, 3, 3,
                1.0, work, 3, rm, 3, 0.0, mt, 3);
    // ampMax is the absolute maximum amplitude
    *ampMax = MAX(fabs(pev1), MAX(fabs(tev1),  fabs(bev1)));
    return;
}
//============================================================================//
/*!
 * @brief Computes the lower-hemisphere projection `beachball' diagram for a
 *        focal mechanism.  This is based on Cliff Frohlich's bballP and 
 *        has been translated to C and does no plotting.
 *
 * @param[in] xc               plot coordinates (e.g. pixels) of center of
 *                             beachball
 * @param[in] yc               plot coordinates (e.g. pixels) of center of
 *                             beachball
 * @param[in] rad              beachball radius (e.g. pixels)
 * @param[in] taz1             azimuth (degrees) of tension eigenvector
 * @param[in] tdp1             dip (degrees) of tension eigenvector
 * @param[in] tevI             eigenvalue corresponding to tension eigenvector
 *                             (e.g. Newton meters)
 * @param[in] baz1             azimuth (degrees) of null eigenvector
 * @param[in] bdp1             dip (degrees) of null eigenvector
 * @param[in] bevI             eigenvalue corresponding to null eigenvector
 *                             (e.g. Newton meters)
 * @param[in] paz1             azimuth (degrees) of pressure eigenvector
 * @param[in] pdp1             dip (degrees) of pressure eigenvector
 * @param[in] pevI             eigenvalue corresponding to pressure eigenvector
 *                             (e.g. Newton meters)
 * @param[in] nNodalLine       number of points to draw in nodal line.
 * @param[in] nFocalSphere     number of points to draw in focal sphere.
 * @param[in] nxPixel          number of pixels in x when drawing polarity
 * @param[in] nyPixel          number of pixels in y when drawing polairty
 * @param[in] lwantNodalLines  if true then compute the nodal lines.
 * @param[in] lwantPTBAxes     if true then compute the PTB axis points.
 * @param[in] lwantPolarity    if true then compute the polarity.
 *
 * @param[out] beachBall       container for the pressure, tension and null axes,
 *                             the nodal lines, and the polarity
 *
 * @author Ben Baker
 *
 * @copyright BSD
 *
 * @bug this routine fails if rad is not = 1.
 *
 */
void cliffsNodes_beachBallP(const double xc, const double yc, const double rad,
                            const double taz1, const double tdp1,
                            const double tevI,
                            const double baz1, const double bdp1,
                            const double bevI,
                            const double paz1, const double pdp1,
                            const double pevI,
                            const int nNodalLine,
                            const int nFocalSphere,
                            const int nxPixel, const int nyPixel,
                            const bool lwantNodalLines,
                            const bool lwantPTBAxes,
                            const bool lwantPolarity,
                            struct beachBall_struct *beachBall)
{
    double *xw1, *xw2, *yw1, *yw2;
    double rm[9] __attribute__ ((aligned(64)));
    double rmt[9] __attribute__ ((aligned(64)));
    double tm[9] __attribute__ ((aligned(64)));
    double pt[3] __attribute__ ((aligned(64)));
    double ptnew[3] __attribute__ ((aligned(64)));
    double ax1[3] __attribute__ ((aligned(64)));
    double ax2[3] __attribute__ ((aligned(64)));
    double ax3[3] __attribute__ ((aligned(64)));
    double ptLine1[3] __attribute__ ((aligned(64)));
    double ptLine2[3] __attribute__ ((aligned(64)));
    double ptLine1Old[3] __attribute__ ((aligned(64)));
    double ptLine2Old[3] __attribute__ ((aligned(64)));
    double ampMax, az, baz, bdp, bev, cosPsi, cosTheta, 
           dip, dp, dpsi, dx, dy, evI, fclvd, fIso, howClose, rad2,
           rb, rp, rpt, rpt2, rt, paz, pdp, pev, plusMin, psi,
           sinsq, sinPsi, sinTheta, str,
           taz, tdp, tev, theta, x, x0, xpt, y, y0, ypt;
    int *pn1, *pn2, ix, ixy, iy, k, nlines, nnl1, nnl2, npts, nseg,
        nx, ny, penDown, psave1, psave2;
    size_t nwork;
    const double sqrt2 = 1.4142135623730951;
    const double sqrt2i = 1.0/sqrt2;
    const double degrad = M_PI/180.0;
    const double degradi = 180.0/M_PI; 
    const double third = 1.0/3.0;
    const double tol = 0.95;
    // null out result
    memset(beachBall, 0, sizeof(struct beachBall_struct));
    // Avoid overwriting memory
    taz = taz1;
    tdp = tdp1;
    baz = baz1;
    bdp = bdp1;
    paz = paz1;
    pdp = pdp1;
    // Step 1: set up axes:
    //  - D axis ('dominant' 1-axis - with largest absolute eigenvalue)
    //  - M axis ('minor'    3-axis)
    //  - B axis (           2-axis)
    evI = (tevI + bevI + pevI)*third;
    tev = tevI - evI;
    bev = bevI - evI;
    pev = pevI - evI;
    vecstrd(paz, pdp, ax1);
    vecstrd(taz, tdp, ax3);
    fclvd = fabs(bev/pev);
    fIso = evI/pev;
    if (fabs(tev) > fabs(pev))
    {
        vecstrd(taz, tdp, ax1);
        vecstrd(paz, pdp, ax3);
        fclvd = fabs(bev/tev);
        fIso = evI/tev;
    }
    //if (fabs(evI) > .03) 
    //{
    //    write(6,901) evI
    //}
    crossp(ax1, ax3, ax2);
    strdvec(ax2, &baz, &bdp);
    cliffsNodes_mtensor(taz, tdp, paz, pdp, tevI, bevI, pevI,
                        tm, rm, rmt, &ampMax);
    // Step 2: Now: plot upper half nodal line:
    // point on nodal line is represented by vector pt;
    //  pt is defined by two angles 
    //  - psi is azimuth with respect to 3-axis (M-axis) 
    //    in 2-3 plane (in B-M plane)
    //  - theta is angle with respect to 1 axis.
    // It can be shown (i. e., cliff did it once) that:
    //   sin(theta)**2=(2+2*fIso)/(3+(1-2*f)*cos(2*psi))
    // Additionally, a constraint corresponding to Cliff's Rule 1
    // for existance of P nodal planes is applied
    //psi = 0.;
    //dpsi = 2.;
    //npts  = nNodalLine1; //1 + (int)(360.0/dpsi + 0.5); //nint((360./dpsi))
    //dpsi = dpsi*degrad;
    psave1 =-1;
    psave2 =-1;
    memset(ptLine1Old, 0, 3*sizeof(double));
    memset(ptLine2Old, 0, 3*sizeof(double));
    if (lwantNodalLines && nNodalLine > 1 &&
        !(tevI > 0.0 && bevI > 0.0 && pevI > 0.0) &&
        !(tevI < 0.0 && bevI < 0.0 && pevI < 0.0))
    {
        npts = nNodalLine;
        psi = 0.0;
        dpsi = 360.0/(double) (nNodalLine - 1);
        dpsi = dpsi*degrad;
        xw1 = (double *) calloc((size_t) npts, sizeof(double));
        yw1 = (double *) calloc((size_t) npts, sizeof(double));
        pn1 = (int *) calloc((size_t) npts+1, sizeof(int));
        xw2 = (double *) calloc((size_t) npts, sizeof(double));
        yw2 = (double *) calloc((size_t) npts, sizeof(double));
        pn2 = (int *) calloc((size_t) npts+1, sizeof(int));
        nnl1 = 0;
        nnl2 = 0;
        for (k=0; k<npts; k++)
        {
            psi = k*dpsi;
            theta = 0.0;
            sinsq = (2. + 2.*fIso)/(3. + (1. - 2.*fclvd)*cos(2.*psi));
            if (sinsq > 0 && sinsq <= 1.0)
            {
                theta = asin(sqrt(sinsq));
                cosTheta = cos(theta);
                sinTheta = sin(theta);
                cosPsi = cos(psi);
                sinPsi = sin(psi);
                // upper half nodal line
                ptLine1[0] = cosTheta*ax1[0] 
                           + sinTheta*(sinPsi*ax2[0] + cosPsi*ax3[0]);
                ptLine1[1] = cosTheta*ax1[1]
                           + sinTheta*(sinPsi*ax2[1] + cosPsi*ax3[1]);
                ptLine1[2] = cosTheta*ax1[2]
                           + sinTheta*(sinPsi*ax2[2] + cosPsi*ax3[2]);
                // lower half nodal line
                ptLine2[0] =-cosTheta*ax1[0]
                           - sinTheta*(sinPsi*ax2[0] + cosPsi*ax3[0]);
                ptLine2[1] =-cosTheta*ax1[1]
                           - sinTheta*(sinPsi*ax2[1] + cosPsi*ax3[1]);
                ptLine2[2] =-cosTheta*ax1[2]
                           - sinTheta*(sinPsi*ax2[2] + cosPsi*ax3[2]);
                isItNear(ptLine1, &howClose, ptLine1Old);
                // upper half nodal line
                penDown =-1;
                if (ptLine1[2] < 0.0){penDown = 1;}
                if (howClose < tol){penDown =-1;}
                strdv(ptLine1, &str, &dip);
                rpt = rad*sqrt2*sin(0.5*degrad*(90.-dip));
                xpt = xc + rpt*sin(str*degrad);
                ypt = yc + rpt*cos(str*degrad);
                xw1[k] = xpt;
                yw1[k] = ypt;
                if (k == 0){pn1[k] = 3;}
                if (penDown == 1 && psave1 == 1)
                {
                    pn1[k] = 2;
                    //printf("%f %f\n", xpt, ypt);
                }
                if (penDown ==-1){pn1[k] = 3;}
                nnl1 = nnl1 + 1;
                psave1 = penDown;
                // lower half nodal line
                penDown =-1;
                if (ptLine2[2] < 0.0){penDown = 1;} 
                if (howClose < tol){penDown =-1;}
                strdv(ptLine2, &str, &dip);
                rpt = rad*sqrt2*sin(0.5*degrad*(90.-dip));
                xpt = xc + rpt*sin(str*degrad);
                ypt = yc + rpt*cos(str*degrad);
                xw2[k] = xpt;
                yw2[k] = ypt;
                if (k == 0){pn2[k] = 3;} // pen down
                if (penDown == 1 && psave2 == 1)
                {
                    pn2[k] = 2;
                    //xptsLine2[k] = xpt;
                    //yptsLine2[k] = ypt;
                    //penLine2[k] = penDwn;
                }
                if (penDown ==-1){pn2[k] = 3;}
                nnl2 = nnl2 + 1;
                psave2 = penDown;
            }
        } // loop on points
        nwork = (size_t) (MAX(2, 2*nnl1));
        beachBall->xNodalLine1 = (double *) calloc(nwork, sizeof(double));
        beachBall->yNodalLine1 = (double *) calloc(nwork, sizeof(double));
        nwork = (size_t) (nnl1 + 1);
        beachBall->nodalLine1Ptr = (int *) calloc(nwork, sizeof(int));
        nwork = (size_t) (MAX(2, 2*nnl2));
        beachBall->xNodalLine2 = (double *) calloc(nwork, sizeof(double));
        beachBall->yNodalLine2 = (double *) calloc(nwork, sizeof(double));
        nwork = (size_t) (nnl2 + 1);
        beachBall->nodalLine2Ptr = (int *) calloc(nwork, sizeof(int));
        // make sure the lines end
        pn1[nnl1] = 3;
        pn2[nnl2] = 3;
        // compute the nodal line 1 line segments 
        nlines = 0;
        nseg = 0;
        for (k=0; k<nnl1; k++)
        {
            // line begins
            if (pn1[k] == 3 && pn1[k+1] == 2)
            {
                beachBall->xNodalLine1[2*nlines  ] = xw1[k];
                beachBall->xNodalLine1[2*nlines+1] = xw1[k+1]; 
                beachBall->yNodalLine1[2*nlines  ] = yw1[k];
                beachBall->yNodalLine1[2*nlines+1] = yw1[k+1];
                nlines = nlines + 1;
            }
            // line continues
            if (pn1[k] == 2 && pn1[k+1] == 2)
            {
                beachBall->xNodalLine1[2*nlines  ] = xw1[k];
                beachBall->xNodalLine1[2*nlines+1] = xw1[k+1]; 
                beachBall->yNodalLine1[2*nlines  ] = yw1[k];
                beachBall->yNodalLine1[2*nlines+1] = yw1[k+1];
                nlines = nlines + 1;
            }
            // line ends 
            if (pn1[k] == 2 && pn1[k+1] == 3)
            {
/*
                beachBall->xNodalLine1[2*nlines  ] = xw1[k];
                beachBall->xNodalLine1[2*nlines+1] = xw1[k+1]; 
                beachBall->yNodalLine1[2*nlines  ] = yw1[k];
                beachBall->yNodalLine1[2*nlines+1] = yw1[k+1];
                nlines = nlines + 1;
*/
                nseg = nseg + 1;
                beachBall->nodalLine1Ptr[nseg] = nlines;
            }
        }
        beachBall->nNodalLineSegs1 = nseg;
        // compute the nodal line 2 line segments 
        nlines = 0;
        nseg = 0;
        for (k=0; k<nnl2; k++)
        {
            // line begins
            if (pn2[k] == 3 && pn2[k+1] == 2)
            {
                beachBall->xNodalLine2[2*nlines  ] = xw2[k];
                beachBall->xNodalLine2[2*nlines+1] = xw2[k+1]; 
                beachBall->yNodalLine2[2*nlines  ] = yw2[k];
                beachBall->yNodalLine2[2*nlines+1] = yw2[k+1];
                nlines = nlines + 1;
            }
            // line continues
            if (pn2[k] == 2 && pn2[k+1] == 2)
            {
                beachBall->xNodalLine2[2*nlines  ] = xw2[k];
                beachBall->xNodalLine2[2*nlines+1] = xw2[k+1]; 
                beachBall->yNodalLine2[2*nlines  ] = yw2[k];
                beachBall->yNodalLine2[2*nlines+1] = yw2[k+1];
                nlines = nlines + 1;
            }
            // line ends 
            if (pn2[k] == 2 && pn2[k+1] == 3)
            {
                beachBall->xNodalLine2[2*nlines  ] = xw2[k];
                beachBall->xNodalLine2[2*nlines+1] = xw2[k+1]; 
                beachBall->yNodalLine2[2*nlines  ] = yw2[k];
                beachBall->yNodalLine2[2*nlines+1] = yw2[k+1];
                nlines = nlines + 1;
                nseg = nseg + 1;
                beachBall->nodalLine2Ptr[nseg] = nlines;
            }
        }
        beachBall->nNodalLineSegs2 = nseg;
//printf("%d %d\n", nlines, nseg);
        free(pn1);
        free(pn2);
        free(xw1);
        free(xw2);
        free(yw1);
        free(yw2);
    }
    // Step 4: Plot the, P, T, and B axes
    if (lwantPTBAxes)
    {
        rp = rad*sqrt2*sin(0.5*degrad*(90.-pdp));
        rt = rad*sqrt2*sin(0.5*degrad*(90.-tdp));
        rb = rad*sqrt2*sin(0.5*degrad*(90.-bdp));
        beachBall->xp = xc + rp*sin(paz*degrad);
        beachBall->yp = yc + rp*cos(paz*degrad);
        beachBall->xt = xc + rt*sin(taz*degrad);
        beachBall->yt = yc + rt*cos(taz*degrad);
        beachBall->xb = xc + rb*sin(baz*degrad);
        beachBall->yb = yc + rb*cos(baz*degrad);
    }
    // Step 5: Plot the focal sphere
    if (nFocalSphere > 1)
    {
        beachBall->nFocalSphere = nFocalSphere;
        beachBall->xFocalSphere = (double *)
                                  calloc((size_t) nFocalSphere, sizeof(double));
        beachBall->yFocalSphere = (double *)
                                  calloc((size_t) nFocalSphere, sizeof(double));
        dpsi = 2.0*M_PI/(double) (nFocalSphere - 1);
#ifdef _OPENMP
        #pragma omp simd
#endif
        for (k=0; k<nFocalSphere; k++)
        {
            psi = (double) k*dpsi;
            beachBall->xFocalSphere[k] = xc + rad*sin(psi);
            beachBall->yFocalSphere[k] = yc + rad*cos(psi);
        } 
    }
    // Step 6: Plot the + and -
    if (lwantPolarity && nxPixel > 0 && nyPixel > 0)
    {
        // begin in lower left corner
        nx = nxPixel;
        ny = nyPixel;
        dx = 2.0*rad/(double) (nx - 1); 
        dy = 2.0*rad/(double) (ny - 1);
        x0 = xc - rad;
        y0 = yc - rad;
        rad2 = rad*rad; // saves a sqrt computation when comparing distances
        xw1 = (double *)calloc((size_t) (nx*ny), sizeof(double));
        yw1 = (double *)calloc((size_t) (nx*ny), sizeof(double));
        pn1 = (int *)calloc((size_t) (nx*ny), sizeof(int));        
        nwork = 0;
        // loop on pixel grid
        for (iy=0; iy<ny; iy++)
        {
            for (ix=0; ix<nx; ix++)
            {
                x = x0 + (double) ix*dx;
                y = y0 + (double) iy*dy;
                k = iy*nx + ix;
                pn1[k] = 0;
                xw1[k] = x;
                yw1[k] = y;
                // require (x,y) be in the focal sphere
                rpt2 = pow(x - xc, 2) + pow(y - yc, 2);
                if (rpt2 < rad2)
                {
                    nwork = nwork + 1;
                    rpt = sqrt(rpt2);
                    // compute the azimuth
                    theta = atan2(y - yc, x - xc);  // measure from x axis
                    az = (M_PI_2 - theta)*degradi;  // az is measured from north
                    if (az < 0.0){az = az + 360.0;} // convention is [0,360]
                    // compute the dip
                    dp = (M_PI_2 - 2.0*asin(rpt*sqrt2i))*degradi;
                    vecstrd(az, dp, pt);
                    //cblas_dgemv(CblasColMajor, CblasNoTrans, 3, 3,
                    //            1.0, tm, 3, pt, 1, 0.0, ptnew, 1);
                    gemv3(tm, pt, ptnew);
                    plusMin = dot3(pt, ptnew);
                    pn1[k] =-1;
                    if (plusMin > 100.0*DBL_EPSILON*ampMax){pn1[k] = 1;}
                } // end check on location
            } // loop on x
        } // loop on y
        // Copy the points to be plotted
        if (nwork > 0)
        {
            beachBall->npolarity = (int) nwork;
            beachBall->xPolarity = (double *)
                                   calloc((size_t) nwork, sizeof(double));
            beachBall->yPolarity = (double *)
                                   calloc((size_t) nwork, sizeof(double));
            beachBall->polarity = (int *)
                                  calloc((size_t) nwork, sizeof(double));
            k = 0; 
            for (ixy=0; ixy<nx*ny;  ixy++)
            {
                if (pn1[ixy] != 0)
                {
                    beachBall->xPolarity[k] = xw1[ixy];
                    beachBall->yPolarity[k] = yw1[ixy];
                    beachBall->polarity[k] = pn1[ixy];
                    k = k + 1;
                }
            }
        }
        free(pn1);
        free(xw1);
        free(yw1);
    }
    return;
}

void cliffsNodes_free(struct beachBall_struct *beachBall)
{
    if (beachBall->xNodalLine1 != NULL){free(beachBall->xNodalLine1);}
    if (beachBall->yNodalLine1 != NULL){free(beachBall->yNodalLine1);}
    if (beachBall->xNodalLine2 != NULL){free(beachBall->xNodalLine2);}
    if (beachBall->yNodalLine2 != NULL){free(beachBall->yNodalLine2);}
    if (beachBall->xFocalSphere != NULL){free(beachBall->xFocalSphere);}
    if (beachBall->yFocalSphere != NULL){free(beachBall->yFocalSphere);}
    if (beachBall->xPolarity != NULL){free(beachBall->xPolarity);}
    if (beachBall->yPolarity != NULL){free(beachBall->yPolarity);}
    if (beachBall->polarity != NULL){free(beachBall->polarity);}
    if (beachBall->nodalLine1Ptr != NULL){free(beachBall->nodalLine1Ptr);}
    if (beachBall->nodalLine2Ptr != NULL){free(beachBall->nodalLine2Ptr);}
    memset(beachBall, 0, sizeof(struct beachBall_struct));
    return;
}
//============================================================================//
/*! 
 * @brief Finds simple rotation matrix R and transpose R^T for rotation
 *        of a vector by angle theta about x, y, or z.  This is based on 
 *        Cliff Rohlich's rotmat.
 *
 * @param[in] naxis    rotation axis.  if 1 then x.  if 2 then y.  if 3 then z.
 * @param[in] theta    rotation angle (degrees)
 * @param[out] r       rotation matrix [3 x 3] in column major format.
 * @param[out] rt      tranpsose rotation matrix [3 x 3] in column major format.
 *
 * @author Ben Baker
 *
 * @copyright BSD
 *
 */
static void rotmat(const int naxis, const double theta,
                   double *__restrict__ r,
                   double *__restrict__ rt)
{
    double cosThetar, sinThetar, thetar;
    int i, j;
    const double degrad = M_PI/180.0;
    memset(r, 0, 9*sizeof(double));
    thetar = theta*degrad;
    cosThetar = cos(thetar); 
    sinThetar = sin(thetar);
    r[0] = cosThetar;
    r[4] = cosThetar;
    r[8] = cosThetar;
    // naxis = x
    if (naxis == 1)
    {
        r[0] = 1.0;       // (1, 1)
        r[5] = sinThetar; // (3, 2)
        r[7] =-sinThetar; // (2, 3)
    }
    // naxis = y
    else if (naxis == 2)
    {
        r[2] = sinThetar; // (3, 1)
        r[4] = 1.0;       // (2, 2)
        r[6] =-sinThetar; // (1, 3)
    }
    // naxis = z
    else if (naxis == 3)
    {
        r[1] = sinThetar; // (2, 1)
        r[3] =-sinThetar; // (1, 2)
        r[8] = 1.0;       // (3, 3)
    }
    // Compute the transpose   
    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)
        {
            rt[3*i+j] = r[3*j+i];
        }
    }
    return;
}
//============================================================================//
/*!
 * @brief Find components of downward pointing unit vector pole from the 
 *        strike and dip angles.  This is based on Cliff Frolich's vecstrd.
 *
 * @param[in] str    strike angle of pole (degrees)
 * @param[in] dip    dip angle of pole (degrees)
 *
 * @param[out] r     corresponding unit vector pole (3)
 *
 * @author Ben Baker
 *
 * @copyright BSD
 * 
 */
static void vecstrd(const double str, const double dip, double r[3])
{
    double cosdip, cosstr, diprad, sindip, sinstr, strrad;
    const double degrad = M_PI/180.0;
    // convert to radians
    strrad = str*degrad;
    diprad = dip*degrad;
    // compute cosines and sines
    sinstr = sin(strrad);
    cosstr = cos(strrad);
    cosdip = cos(diprad);
    sindip = sin(diprad);
    // compute (x,y,z) from strike and dip for unit radius 
    r[0] = sinstr*cosdip;
    r[1] = cosstr*cosdip;
    r[2] =-sindip;
    return;
}
//============================================================================//
static void isItNear(const double *__restrict__ pt,
                     double *howClose,
                     double *__restrict__ ptold)
{
    *howClose = dot3(ptold, pt); //cblas_ddot(3, ptold, 1, pt, 1); 
    ptold[0] = pt[0];
    ptold[1] = pt[1];
    ptold[2] = pt[2];
    return;
}
/*!
 * @brief Finds the strike and dip of downward pointing unit vector.  This
 *        is based on Cliff Frolich's strdvec.
 *
 * @param[in,out] r      on input contains the downward pointing unit
 *                       vector.
 *                       if r[2] is > 0 then the unit vector is pointing
 *                       updward.  in this case then on output r3 will
 *                       be negated.
 *
 * @param[out] str       strike angle (degrees) \f$ s.t. \phi \in [0,360] \f$.
 * @param[out] dip       dip angle (degrees) 
 *
 * @author Ben Baker
 *
 * @copyright BSD
 *
 */
static void strdvec(double r[3], double *str, double *dip)
{
    double rs;
    const double degradi = 180.0/M_PI;
    if (r[2] > 0.0)
    {
        r[0] =-r[0];
        r[1] =-r[1];
        r[2] =-r[2];
    }
    *str = atan2(r[0], r[1])*degradi;
    if (*str < 0.){*str = *str + 360.;}
    rs = sqrt(r[0]*r[0] + r[1]*r[1]);
    *dip=atan2(-r[2], rs)*degradi;
    return;
}
/*!
 * @brief Cross product of 2 length 3 vectors u and v.  Returns normal vector n.
 * 
 * @param[in] u    first vector in cross product u x v [3]
 * @param[in] v    second vector in cross product u x v [3]
 * 
 * @param[out] n   normal vector n = u x v [3]
 *
 * @author Ben Baker
 *
 * @copyright BSD
 *
 */
static void crossp(const double *__restrict__ u,
                   const double *__restrict__ v,
                   double *__restrict__ n)
{
    n[0] = u[1]*v[2] - u[2]*v[1];
    n[1] = u[2]*v[0] - u[0]*v[2]; 
    n[2] = u[0]*v[1] - u[1]*v[0];
    return;
}
/*!
 * @brief Finds strike and dip given components of unit vector.  This is
 *        based on Cliff Frohlich's strdv.
 *
 * @param[in] r      unit vector 
 * @param[out] str   strike angle (degrees)
 * @param[out] dip   dip angle (degrees) s.t. dip down is positive
 *
 * @author Ben Baker
 *
 * @copyright BSD
 *
 */
static void strdv(const double r[3], double *str, double *dip)
{
    double rs;
    const double degradi = 180.0/M_PI;
    *str = atan2(r[0], r[1])*degradi;
    rs = sqrt(r[0]*r[0] + r[1]*r[1]);
    *dip = atan2(-r[2], rs)*degradi;
    return;
}

/*!
 * @brief Utility function for computing dot product of length 3 vector
 */
static double dot3(const double *__restrict__ x,
                   const double *__restrict__ y)
{
    double dot;
    dot = x[0]*y[0] + x[1]*y[1] + x[2]*y[2];
    return dot;
}

/*!
 * @brief Utility function for computing y = Ax for 3 x 3 matrix.
 */
static void gemv3(const double *__restrict__ A, const double *__restrict__ x,
                  double *__restrict__ y)
{
    y[0] = A[0]*x[0] + A[3]*x[1] + A[6]*x[2];
    y[1] = A[1]*x[0] + A[4]*x[1] + A[7]*x[2];
    y[2] = A[2]*x[0] + A[5]*x[1] + A[8]*x[2];
    return;
}
