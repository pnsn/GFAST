#ifndef __SACIO_SACIO_H__
#define __SACIO_SACIO_H__
#ifdef __cplusplus
extern "C"
{
#endif
void rsac1(char *, float [], int *, float *, float *,
           int *, int *, int );
void rsach(char *, int *, int);
void getfhv(char *, float *, int *, int);
void getkhv(char *, char *, int *, int, int);
void getnhv(char *, int *, int *, int);
void newhdr();
void setkhv(char *, char *, int *, int, int);
void setnhv(char *, int *, int *, int);
void setfhv(char *, float *, int *, int);
void wsac0(char *, float *, float *, int *, int);

int sacio_readHeader(char *flname, int argc, char *argv[], int *itype,
                     int nc, char hdr_char[][16],
                     int ni, int *hdr_ints,
                     int nd, double *hdr_dble);
double *sacio_readData(char *flname, int *npts, int *ierr);
int sacio_writeTrace(char *flname, int argc, char *argv[], int *itype,
                     int nc, char hdr_char[][16],
                     int ni, int *hdr_ints,
                     int nd, double *hdr_dble,
                     int npts, double *data);
#ifdef __cplusplus
}
#endif
#endif /* __SACIO_SACIO_H__ */
