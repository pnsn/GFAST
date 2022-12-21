#ifndef BEACHBALL_H_
#define BEACHBALL_H_ 1
#include <stdbool.h>
#include "compearth.h"

struct beachBall_struct
{
    double *xNodalLine1;   /*!< x points comprising nodal line 1 */
    double *yNodalLine1;   /*!< y points comprising nodal line 1 */
    double *xNodalLine2;   /*!< x points comprising nodal line 2 */
    double *yNodalLine2;   /*!< y points comprising nodal line 2 */
    double *xFocalSphere;  /*!< x locations defining focal sphere */
    double *yFocalSphere;  /*!< y locations defining focal sphere */
    double *xPolarity;     /*!< x locations where polarities are computed */
    double *yPolarity;     /*!< y locations where polarities are computed */
    int *polarity;          /*!< if 1 then the polarity is up.  if -1 then the
                                polarity is down */
    int *nodalLine1Ptr;    /*!< Points to start and end of line segments
                                making up first nodal line [nNodalLine1+1] */
    int *nodalLine2Ptr;    /*!< Points to start and end of line segments
                                making up second nodal line [nNodalLine2+1] */
    double xp;             /*!< x location of pressure dot */
    double yp;             /*!< y location of pressure dot */ 
    double xb;             /*!< x location of null dot */
    double yb;             /*!< y location of null dot */
    double xt;             /*!< x location of tension dot */
    double yt;             /*!< y location of tension dot */
    int nNodalLineSegs1;   /*!< Number of nodal lines segments comprising
                                nodal line */
    int nNodalLineSegs2;   /*!< Number of nodal line segents comprising second
                                nodal line */
    int nFocalSphere;      /*!< Number of points in focal sphere.  if 0 then
                                the focal sphere is not defined  */
    int npolarity;         /*!< Number of polarity points */
    bool lhavePTBAxes;     /*!< If true then the PTB axes are defined */
    char pad[7];
};

struct beachballPlot_struct
{
    double t_axis[3], null_axis[3], p_axis[3];
    double rad;
    double xc; 
    double yc; 
    int nxPixel;
    int nyPixel;
    int nNodalLine;
    int nFocalSphere;
    bool lwantPolarity;
    bool lwantNodalLines;
    bool lhaveMT;
    char pad[7];
};


#ifdef __cplusplus
extern "C"
{
#endif


int beachball_writePNG(const char *fileName,
                       size_t height, size_t width, unsigned char *pixels);
int beachball_draw(const char *fileName,
                   struct beachballPlot_struct beachball);
int beachball_plotDefaults(struct beachballPlot_struct *beachball);
int beachball_setMomentTensor(const double m11, const double m22,
                              const double m33, const double m12,
                              const double m13, const double m23,
                              enum compearthCoordSystem_enum basis,
                              struct beachballPlot_struct *beachball);

void cliffsNodes_mtensor(const double taz, const double tdp,
                         const double paz, const double pdp,
                         const double tev1, const double bev1,
                         const double pev1,
                         double *__restrict__ mt,
                         double *__restrict__ rm,
                         double *__restrict__ rmt,
                         double *ampMax);
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
                            struct beachBall_struct *beachBall);
void cliffsNodes_free(struct beachBall_struct *beachBall);

#ifdef __cplusplus
}
#endif

#endif
