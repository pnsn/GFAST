#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <hdf5.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "gfast_hdf5.h"
#include "iscl/log/log.h"
#include "iscl/os/os.h"

/*!
 * @brief Opens an HDF5 file and returns handle for reading only
 *
 * @param[in] flname   name of HDF5 file to open (NULL terminated)
 *
 * @result file handle
 *
 * @author Ben Baker, ISTI
 *
 */
hid_t h5_open_rdonly(const char *flname)
{
    const char *fcnm = "h5_open_rdonly\0";
    hid_t file_id;
    if (!ISCL_os_path_isfile(flname))
    {
        log_errorF("%s: HDF5 file %s does not exist!\n", fcnm, flname);
    }
    file_id = H5Fopen(flname, H5F_ACC_RDONLY, H5P_DEFAULT);
    return file_id; 
}
//============================================================================//
/*!
 * @brief Opens an HDF5 file and returns handle for reading and writing
 *
 * @param[in] flname   name of HDF5 file to open (NULL terminated)
 *
 * @result file handle
 *
 * @author Ben Baker, ISTI
 *
 */
hid_t h5_open_rdwt(const char *flname)
{
    hid_t file_id;
    file_id = H5Fopen(flname, H5F_ACC_RDWR, H5P_DEFAULT);
    return file_id;
}
//============================================================================//
/*!
 * @brief Closes an HDF5 file
 *
 * @param[in] file_id    HDF5 file handle
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int h5_close(const hid_t file_id)
{
    int ierr;
    ierr = (int) (H5Fclose(file_id));
    return ierr; 
}
//============================================================================//
/*!
 * @brief Writes a float array to HDF5
 *
 * @param[in] dset_name   name of dataset to write
 * @param[in] file_id     HDF5 file handle
 * @param[in] n           size of dataset to write
 * @param[in] x           dataset to write
 * 
 * @result 0 indicates success
 * 
 * @author Ben Baker, ISTI
 *
 */
int h5_write_array__float(const char *dset_name, const hid_t file_id,
                          const int n, const float *x)
{
    const char *fcnm = "h5_write_array__float\0";
    char *citem = (char *)calloc(strlen(dset_name)+1, sizeof(char));
    hid_t flt_dataspace_id, flt_dataset_id;
    hsize_t dims[1];
    herr_t status;
    //------------------------------------------------------------------------//
    //  
    // Copy file handle and name
    strcpy(citem,dset_name);
    // Create dataspace
    dims[0] = (hsize_t) n;
    flt_dataspace_id = H5Screate_simple(1, dims, NULL);
    // Create dataset
    flt_dataset_id = H5Dcreate2(file_id, citem, H5T_NATIVE_FLOAT,
                                flt_dataspace_id,
                                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    // Write data
    status = H5Dwrite(flt_dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, x);
    if (status != 0)
    {
        log_errorF(" %s: Write error\n", fcnm);
        return -1;
    }
    // Close the dataspace
    status  = H5Sclose(flt_dataspace_id);
    status += H5Dclose(flt_dataset_id);
    if (status != 0)
    {
        log_errorF(" %s: Close error\n", fcnm);
    }
    free(citem);
    return status;
}
//============================================================================//
/*!
 * @brief Writes a double array to HDF5
 *
 * @param[in] dset_name   name of dataset to write
 * @param[in] file_id     HDF5 file handle
 * @param[in] n           size of dataset to write
 * @param[in] x           dataset to write
 * 
 * @result 0 indicates success
 * 
 * @author Ben Baker, ISTI
 *
 */
int h5_write_array__double(const char *dset_name, const hid_t file_id,
                           const int n, const double *x)
{
    const char *fcnm = "h5_write_array__double\0";
    char *citem = (char *)calloc(strlen(dset_name)+1, sizeof(char));
    hid_t dbl_dataspace_id, dbl_dataset_id;
    hsize_t dims[1];
    herr_t status;
    //------------------------------------------------------------------------//
    //
    // Copy file handle and name
    strcpy(citem, dset_name);
    // Create dataspace
    dims[0] = (hsize_t) n;
    dbl_dataspace_id = H5Screate_simple(1, dims, NULL);
    // Create dataset
    dbl_dataset_id = H5Dcreate2(file_id, citem, H5T_NATIVE_DOUBLE,
                                dbl_dataspace_id,
                                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    // Write data
    status = H5Dwrite(dbl_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, x);
    if (status != 0)
    {
        log_errorF(" %s: Write error\n", fcnm);
        return -1;
    }
    // Close the dataspace
    status  = H5Sclose(dbl_dataspace_id);
    status += H5Dclose(dbl_dataset_id);
    if (status != 0)
    {
        log_errorF(" %s: Close error\n", fcnm);
    }
    free(citem);
    return status;
}
//============================================================================//
/*!
 * @brief Writes an int array to HDF5
 *
 * @param[in] dset_name   name of dataset to write
 * @param[in] file_id     HDF5 file handle
 * @param[in] n           size of dataset to write
 * @param[in] x           dataset to write
 * 
 * @result 0 indicates success
 * 
 * @author Ben Baker, ISTI
 *
 */
int h5_write_array__int(const char *dset_name, const hid_t file_id,
                        const int n, const int *x)
{
    const char *fcnm = "h5_write_array__int\0";
    char *citem = (char *)calloc(strlen(dset_name)+1, sizeof(char));
    hid_t int_dataspace_id, int_dataset_id;
    hsize_t dims[1];
    herr_t status;
    //------------------------------------------------------------------------//
    // Copy file handle and name
    strcpy(citem, dset_name);
    // Create dataspace
    dims[0] = (hsize_t) n;
    int_dataspace_id = H5Screate_simple(1, dims, NULL);
    // Create dataset
    int_dataset_id = H5Dcreate2(file_id, citem, H5T_NATIVE_INT,
                                int_dataspace_id,
                                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    // Write data
    status = H5Dwrite(int_dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, x);
    if (status != 0)
    {
        log_errorF(" %s: Write error\n", fcnm);
        return -1;
    }
    // Close the dataspace
    status  = H5Sclose(int_dataspace_id);
    status += H5Dclose(int_dataset_id);
    if (status != 0)
    {
        log_errorF(" %s: Close error\n", fcnm);
    }
    free(citem);
    return status;
}
//============================================================================//
/*!
 * @brief Writes a char ** array to HDF5
 *
 * @param[in] citem_chr  name of char** dataset
 * @param[in] file_id    HDF5 file handle
 * @param[in] n          number of items in c
 * @param[in] c          char** dataset to write [n]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int h5_write_array__chars(const char *citem_chr, const hid_t file_id,
                          const int n, char **c)
{
    const char *fcnm = "h5_write_array__chars\0";
    char **cout, *citem_hdf5;
    int len_item = (int) strlen(citem_chr);
    hid_t chr_dataset_id, chr_dataspace_id, cftype, cmtype;
    hsize_t dims[1];
    herr_t status;
    int i, lens;
    //------------------------------------------------------------------------//
    //  
    // Set the name of the attribute while remembering a null terminator 
    citem_hdf5 = (char *)calloc((size_t) (len_item+1), sizeof(char));
    strncpy(citem_hdf5, citem_chr, len_item);
    // Create dataspace
    dims[0] = (hsize_t) n;
    chr_dataspace_id = H5Screate_simple(1, dims, NULL);
    // Create file and memory types
    cftype = H5Tcopy(H5T_C_S1);
    status = H5Tset_size(cftype, H5T_VARIABLE);
    if (status < 0)
    {
        log_errorF("%s: Error setting space \n", fcnm);
        return -1;
    }
    cmtype = H5Tcopy(H5T_C_S1);
    status = H5Tset_size(cmtype, H5T_VARIABLE);
    if (status < 0)
    {
        log_errorF("%s: Error setting memory space\n", fcnm);
        return -1;
    }
    // Create the dataset
    chr_dataset_id = H5Dcreate2(file_id, citem_hdf5, cftype,
                                chr_dataspace_id,
                                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    // Create output
    cout = (char **)calloc((size_t) n, sizeof(char *));
    for (i=0; i<n; i++)
    {
        lens = (int) (strlen(c[i]));
        cout[i] = (char *)calloc((size_t) (lens+1), sizeof(char));
        strcpy(cout[i], c[i]);
    }
    // Write the data
    status = H5Dwrite(chr_dataset_id, cmtype, H5S_ALL,H5S_ALL,
                      H5P_DEFAULT, cout);
    if (status != 0)
    {
        log_errorF("%s: Write error\n", fcnm);
        return -1;
    }
    status += H5Tclose(cftype);
    status += H5Tclose(cmtype);
    status += H5Sclose(chr_dataspace_id);
    status += H5Dclose(chr_dataset_id);
    if (status != 0)
    {
        log_errorF("%s: Close errors\n", fcnm);
    } 
    // Free space
    for (i=0; i<n; i++)
    {
        free(cout[i]);
    }
    free(cout);
    free(citem_hdf5);
    return status;
}
//===========================================================================//
/*! 
 * @brief Reads a string array into a char** array 
 * 
 * @param[in] citem        name of HDF5 dataset 
 * @param[in] file_id      HDF5 file handle
 *
 * @param[out] nitems      number of rows in output char ** array
 * @param[out] ierr        0 indicates success
 *
 * @result char** array to be read from the HDF5 file file_id
 *
 */
char **h5_read_array__string(const char *citem, const hid_t file_id,
                             int *nitems, int *ierr)
{
    const char *fcnm = "h5_read_array__string\0";
    char **cout, *citem_hdf5;
    size_t lenos = strlen(citem);
    hid_t dataSet, fileType, memType, space;
    size_t sdim;
    hsize_t dims[1];
    herr_t status;
    int i, ndims;
    //------------------------------------------------------------------------//
    cout = NULL;
    *ierr = 0;
    *nitems = 0;
    citem_hdf5 = (char *)calloc(lenos+1, sizeof(char));
    strcpy(citem_hdf5, citem);
    dataSet = H5Dopen(file_id, citem_hdf5, H5P_DEFAULT);
    fileType = H5Dget_type(dataSet);
    sdim = H5Tget_size(fileType) + 1; // make space for null terminator
    space = H5Dget_space(dataSet);
    ndims = H5Sget_simple_extent_dims(space, dims, NULL);
    *nitems = (int) dims[0];
    if (*nitems < 1)
    {
        log_errorF("%s: Error no data to read %d\n", fcnm, *nitems);
        *ierr = 1;
        return cout;
    }
    // Allocate array of pointers to rows
    cout = (char **)calloc((size_t) *nitems, sizeof(char *));
    // Allocate space for integer data
    cout[0] = (char *)calloc(dims[0], sdim*sizeof(char));
    for (i=1; i<*nitems; i++)
    {
        cout[i] = cout[0] + (size_t) i*sdim;
    }
    memType = H5Tcopy(H5T_C_S1);
    status = H5Tset_size(memType, sdim);
    if (status < 0)
    {
        log_errorF("%s: Error setting size\n", fcnm);
        *ierr = 1;
        return cout;
    }
    status = H5Dread(dataSet, memType, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                     cout[0]);
    if (status < 0)
    {
        log_errorF("%s: Error reading data\n", fcnm);
        *ierr = 1;
        return cout;
    }
    status = H5Dclose(dataSet);
    status = H5Sclose(space);
    status = H5Tclose(fileType);
    status = H5Tclose(memType);
    free(citem_hdf5);
    return cout;
} 
//============================================================================//
/*! 
 * @brief Reads a char** array 
 * 
 * @param[in] citem        name of HDF5 dataset 
 * @param[in] file_id      HDF5 file handle
 *
 * @param[out] nitems      number of rows in output char ** array
 * @param[out] ierr        0 indicates success
 *
 * @result char** array to be read from the HDF5 file file_id
 *
 */
char **h5_read_array__char(const char *citem, const hid_t file_id,
                           int *nitems, int *ierr)
{
    const char *fcnm = "h5_read_array__char\0";
    char **cdata, **cwork, *citem_hdf5;
    size_t len_item = strlen(citem);
    size_t lens;
    int i;
    hid_t chr_dataset, cspace, cmtype;
    hsize_t dims[2];
    herr_t status;
    //------------------------------------------------------------------------//
    //  
    // Set the name of the attribute while remembering a null terminator 
    cdata = NULL;
    *ierr = 0;
    citem_hdf5 = (char *) calloc(len_item+1, sizeof(char));
    strcpy(citem_hdf5, citem);
    // Open dataset
    chr_dataset = H5Dopen(file_id, citem_hdf5, H5P_DEFAULT);
    // Get the datatype
    // Get dataspace and allocate memory for read buffer.
    cspace = H5Dget_space(chr_dataset);
    *nitems = H5Sget_simple_extent_dims(cspace, dims, NULL);
    *nitems = (int) dims[0]; 
    cwork = (char **)calloc((size_t) dims[0], sizeof(char *));
    // Create the memory datatype
    cmtype = H5Tcopy(H5T_C_S1);
    status = H5Tset_size(cmtype,H5T_VARIABLE);
    if (status < 0)
    {
        log_errorF("%s: Error creating memory datatype\n",fcnm);
        *ierr = 1;
        return NULL;
    }
    // Read the data
    status = H5Dread(chr_dataset, cmtype, H5S_ALL, H5S_ALL,
                     H5P_DEFAULT, cwork);
    if (status < 0)
    {
        log_errorF("%s: Error reading char data\n",fcnm);
        *ierr = 1;
        return NULL;
    }
    // Close and release resources
    status = H5Dclose(chr_dataset);
    if (status < 0)
    {
        log_errorF("%s: Error closing dataset\n",fcnm);
        *ierr = 1;
        return NULL;
    }
    status = H5Sclose(cspace);
    if (status < 0){
        log_errorF("%s: Error closing dataspace\n",fcnm);
        *ierr = 1;
        return NULL;
    }
    status = H5Tclose(cmtype);
    if (status < 0)
    {
        log_errorF("%s: Error closing memory type\n",fcnm);
        *ierr = 1;
        return NULL;
    }
    // Copy back and free workspace
    cdata = (char **)calloc((size_t) *nitems, sizeof(char *));
    for (i=0; i<*nitems; i++)
    {
        lens = strlen(cwork[i]);
        cdata[i] = (char *)calloc(lens+1, sizeof(char));
        strcpy(cdata[i], cwork[i]);
        free(cwork[i]);
    }
    free(cwork);
    free(citem_hdf5);
    return cdata;
}
//============================================================================//
/*!
 * @brief Reads a double array from HDF5
 *
 * @param[in] dset_name   name of dataset to read 
 * @param[in] file_id     HDF5 file handle
 * @param[in] nref        size of dataset to read 
 *
 * @param[out] x          dataset read from disk 
 * 
 * @result 0 indicates success
 * 
 * @author Ben Baker, ISTI
 *
 */
int h5_read_array__double(const char *dset_name, const hid_t file_id,
                          const int nref, double *x)
{
    const char *fcnm = "h5_read_array__double\0";
    char *citem = (char *)calloc(strlen(dset_name)+1, sizeof(char));
    hid_t memspace, dbl_dataspace, dbl_dataset, cparms;
    hsize_t *dims;
    herr_t status;
    int nwork, rank, i;
    //------------------------------------------------------------------------//
    //  
    // Copy file hand and name
    strcpy(citem,dset_name);
    // Open dataset
    dbl_dataset = H5Dopen(file_id, citem, H5P_DEFAULT);
    // Create dataspace 
    dbl_dataspace = H5Dget_space(dbl_dataset);
    // Get size of dimensions
    rank = H5Sget_simple_extent_ndims(dbl_dataspace);
    dims = (hsize_t *)calloc((size_t) (rank), sizeof(hsize_t));
    status = H5Sget_simple_extent_dims(dbl_dataspace, dims, NULL);
    nwork = 1;
    for (i=0; i<rank; i++)
    {
        nwork = nwork*(int) dims[i];
    }
    if (nwork > nref)
    {
        log_errorF("%s: Insufficient space!\n", fcnm);
        return -1;
    }
    // Get properties handle
    cparms = H5Dget_create_plist(dbl_dataset);
    // Define memory space
    memspace = H5Screate_simple(1, dims, NULL);
    // Load data
    status = H5Dread(dbl_dataset, H5T_NATIVE_DOUBLE, memspace, dbl_dataspace,
                     H5P_DEFAULT, x);
    if (status != 0)
    {
        log_errorF("%s: Error loading data\n", fcnm);
        return -1;
    }
    // Close it up
    status  = H5Pclose(cparms);
    status += H5Sclose(dbl_dataspace);
    status += H5Sclose(memspace);
    status += H5Dclose(dbl_dataset);
    if (status != 0){
        log_errorF("%s: Error closing space\n", fcnm);
    }
    free(citem);
    free(dims);
    return status;
}
//============================================================================//
/*!
 * @brief Reads a float array from HDF5
 *
 * @param[in] dset_name   name of dataset to read 
 * @param[in] file_id     HDF5 file handle
 * @param[in] nref        size of dataset to read 
 *
 * @param[out] x          dataset read from disk 
 * 
 * @result 0 indicates success
 * 
 * @author Ben Baker, ISTI
 *
 */
int h5_read_array__float(const char *dset_name, const hid_t file_id,
                         const int nref, float *x)
{
    const char *fcnm = "h5_read_array__float\0";
    char *citem = (char *)calloc(strlen(dset_name)+1, sizeof(char));
    hid_t memspace, flt_dataspace, flt_dataset, cparms;
    hsize_t *dims;
    herr_t status;
    int nwork, rank, i;
    //------------------------------------------------------------------------//
    //  
    // Copy file hand and name
    strcpy(citem,dset_name);
    // Open dataset
    flt_dataset = H5Dopen(file_id,citem, H5P_DEFAULT);
    // Create dataspace 
    flt_dataspace = H5Dget_space(flt_dataset);
    // Get size of dimensions
    rank = H5Sget_simple_extent_ndims(flt_dataspace);
    dims = (hsize_t *)calloc((size_t) rank, sizeof(hsize_t));
    status = H5Sget_simple_extent_dims(flt_dataspace, dims, NULL);
    nwork = 1;
    for (i=0; i<rank; i++)
    {
        nwork = nwork*(int) (dims[i]);
    }
    if (nwork > nref)
    {
        log_errorF("%s: Insufficient space!\n", fcnm);
        return -1;
    }
    // Get properties handle
    cparms = H5Dget_create_plist(flt_dataset);
    // Define memory space
    memspace = H5Screate_simple(1, dims, NULL);
    // Load data
    status = H5Dread(flt_dataset, H5T_NATIVE_FLOAT, memspace, flt_dataspace,
                     H5P_DEFAULT, x);
    if (status != 0)
    {
        log_errorF("%s: Error loading data\n", fcnm);
        return -1;
    }
    // Close it up
    status  = H5Pclose(cparms);
    status += H5Sclose(flt_dataspace);
    status += H5Sclose(memspace);
    status += H5Dclose(flt_dataset);
    if (status != 0)
    {
        log_errorF("%s: Error closing space\n", fcnm);
    }
    free(citem);
    free(dims);
    return status;
}
//============================================================================//
/*!
 * @brief Reads an integer array from HDF5
 *
 * @param[in] dset_name   name of dataset to read 
 * @param[in] file_id     HDF5 file handle
 * @param[in] nref        size of dataset to read 
 *
 * @param[out] x          dataset read from disk 
 * 
 * @result 0 indicates success
 * 
 * @author Ben Baker, ISTI
 *
 */
int h5_read_array__int(const char *dset_name, const hid_t file_id,
                       const int nref, int *x)
{
    const char *fcnm = "h5_read_array__int\0";
    char *citem = (char *)calloc(strlen(dset_name)+1,sizeof(char));
    hid_t memspace, int_dataspace, int_dataset, cparms;
    hsize_t *dims;
    herr_t status;
    int nwork, rank, i;
    //------------------------------------------------------------------------//
    //
    // Copy file hand and name
    strcpy(citem,dset_name);
    // Open dataset
    int_dataset = H5Dopen(file_id,citem,H5P_DEFAULT);
    // Create dataspace 
    int_dataspace = H5Dget_space(int_dataset);
    // Get size of dimensions
    rank = H5Sget_simple_extent_ndims(int_dataspace);
    dims = (hsize_t *)calloc((size_t) rank, sizeof(hsize_t));
    status = H5Sget_simple_extent_dims(int_dataspace, dims, NULL);
    nwork = 1;
    for (i=0; i<rank; i++){
        nwork = nwork*(int) (dims[i]);
    }
    if (nwork > nref){
        log_errorF("%s: Insufficient space!\n", fcnm);
        return -1;
    }
    // Get properties handle
    cparms = H5Dget_create_plist(int_dataset);
    // Define memory space
    memspace = H5Screate_simple(1, dims, NULL);
    // Load data
    status = H5Dread(int_dataset, H5T_NATIVE_INT, memspace, int_dataspace,
                     H5P_DEFAULT, x);
    if (status != 0)
    {
        log_errorF("%s: Error reading data\n", fcnm);
        return -1;
    }
    // Close it up
    status  = H5Pclose(cparms);
    status += H5Sclose(int_dataspace);
    status += H5Sclose(memspace);
    status += H5Dclose(int_dataset);
    if (status != 0)
    {
        log_errorF("%s: Error closing h5\n", fcnm);
    }
    free(citem);
    free(dims);
    return status;
}
//============================================================================//
/*!
 * @brief Writes double attributes to an HDF5 dataset
 *
 * @param[in] citem        attribute name 
 * @param[in] hdf5_id      HDF5 dataset handle
 * @param[in] n            number of attributes
 * @param[in] attr_data    double attributes [n]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int h5_write_attribute__double(const char *citem, const hid_t hdf5_id,
                               const int n, const double *attr_data)
{
    const char *fcnm = "h5_write_attribute__double\0";
    char *citem_hdf5;
    int len_item = (int) (strlen(citem));
    hid_t attr_dataspace_id, attribute_id;
    hsize_t dims[1];
    herr_t status;
    //------------------------------------------------------------------------//
    //
    // Set item name
    citem_hdf5 = (char *)calloc((size_t) (len_item+1), sizeof(char));
    strncpy(citem_hdf5, citem, len_item);
    // Create a dataspace for the attribute
    dims[0] = (hsize_t) n;
    attr_dataspace_id = H5Screate_simple(1, dims, NULL);
    // Create dataset id for the attribute
    attribute_id = H5Acreate2(hdf5_id, citem_hdf5, H5T_NATIVE_DOUBLE,
                              attr_dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
    // Write the attribute data
    status  = H5Awrite(attribute_id, H5T_NATIVE_DOUBLE, attr_data);
    status += H5Aclose(attribute_id);
    status += H5Sclose(attr_dataspace_id);
    if (status != 0)
    {
        log_errorF("%s: Error writing attributes\n", fcnm);
    }
    // Free space 
    free(citem_hdf5);
    citem_hdf5 = NULL;
    return status;
}
//============================================================================//
/*!
 * @brief Writes integer attributes to an HDF5 dataset
 *
 * @param[in] citem        attribute name 
 * @param[in] hdf5_id      HDF5 dataset handle
 * @param[in] n            number of attributes
 * @param[in] attr_data    integer attributes [n]
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
int h5_write_attribute__int(const char *citem, const hid_t hdf5_id,
                            const int n, const int *attr_data)
{
    const char *fcnm = "h5_write_attribute__int\0";
    char *citem_hdf5;
    int len_item = (int) strlen(citem);
    hid_t attr_dataspace_id, attribute_id;
    hsize_t dims[1];
    herr_t status;
    //------------------------------------------------------------------------//
    //  
    // Set item name
    citem_hdf5 = (char *)calloc((size_t) (len_item+1), sizeof(char));
    strncpy(citem_hdf5, citem, len_item);
    // Create a dataspace for the attribute
    dims[0] = (hsize_t) n;
    attr_dataspace_id = H5Screate_simple(1, dims, NULL);
    // Create dataset id for the attribute
    attribute_id = H5Acreate2(hdf5_id, citem_hdf5, H5T_NATIVE_INT,
                              attr_dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
    // Write the attribute data
    status  = H5Awrite(attribute_id, H5T_NATIVE_INT, attr_data);
    status += H5Aclose(attribute_id);
    status += H5Sclose(attr_dataspace_id);
    if (status != 0)
    {
        log_errorF("%s: Error writing attributes\n", fcnm);
    }   
    // Free space 
    free(citem_hdf5);
    citem_hdf5 = NULL;
    return status;
}
//============================================================================//
/*!
 * @brief Writes character attributes to an HDF5 dataset
 *
 * @param[in] citem       attribute name 
 * @param[in] hdf5_id     HDF5 dataset handle
 * @param[in] n           number of attributes
 * @param[in] cattr       charater attributes [n]
 *
 * @result 0 indicates success
 * 
 * @author Ben Baker, ISTI
 *
 */
int h5_write_attribute__char(const char *citem, const hid_t hdf5_id,
                             const int n, const char **cattr)
{
    const char *fcnm = "h5_write_attribute__char\0";
    char **cout, *citem_hdf5;
    int len_item = (int) (strlen(citem));
    int i, lens;
    hid_t attr_dataspace_id, attribute_id, cftype, cmtype;
    hsize_t dims[1];
    herr_t status;
    //------------------------------------------------------------------------//
    //
    // Set item name
    citem_hdf5 = (char *)calloc((size_t) (len_item+1),sizeof(char));
    strncpy(citem_hdf5, citem, len_item);
    // Create a dataspace 
    dims[0] = (hsize_t) n;
    attr_dataspace_id = H5Screate_simple(1, dims, NULL);
    // Create the datatypes 
    cftype = H5Tcopy(H5T_C_S1);
    status = H5Tset_size(cftype, H5T_VARIABLE);
    if (status < 0)
    {
        log_errorF("%s: Error setting space!\n",fcnm);
        return -1;
    }
    cmtype = H5Tcopy(H5T_C_S1);
    status = H5Tset_size(cmtype, H5T_VARIABLE);
    if (status < 0)
    {
        log_errorF("%s: Error setting memory space\n", fcnm);
        return -1; 
    }
    // Create dataset for attribute
    attribute_id = H5Acreate2(hdf5_id, citem_hdf5, cftype,
                              attr_dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
    // Create output
    cout = (char **)calloc((size_t) n, sizeof(char *));
    for (i=0; i<n; i++)
    {
        lens = (int) (strlen(cattr[i]));
        cout[i] = (char *)calloc((size_t) (lens+1), sizeof(char));
        strncpy(cout[i], cattr[i], lens);
    } 
    // Write the attributes
    status  = H5Awrite(attribute_id, cmtype, cout);
    // Close it up
    status += H5Aclose(attribute_id);
    status += H5Tclose(cftype);
    status += H5Tclose(cmtype);
    status += H5Sclose(attr_dataspace_id);
    if (status != 0)
    {
        log_errorF("%s: Error writing attribute!\n", fcnm);
    }
    // Free space
    for (i=0; i<n; i++)
    {
        free(cout[i]);
        cout[i] = NULL;
    }
    free(cout);
    free(citem_hdf5);
    citem_hdf5 = NULL;
    cout = NULL;
    return status;
} 
//============================================================================//
/*!
 * @brief Gets the number of members in a group
 *
 * @param[in] group_name    name of HDF5 group
 * @param[in] file_id       HDF5 file handle
 *
 * @result number of members in HDF5 group (0 is empty group)
 *
 * @author Ben Baker, ISTI
 *
 */
int h5_n_group_members(const char *group_name, const hid_t file_id)
{
    int nmember;
    char *citem = (char *)calloc(strlen(group_name)+1, sizeof(char));
    hid_t group_id;
    hsize_t nobj;
    herr_t status;
    //------------------------------------------------------------------------//
    //
    // copy h5 handle and name
    strcpy(citem,group_name);
    // Check groups exists
    status = H5Gget_objinfo(file_id, citem, 0, NULL);
    if (status == 0)
    {
        // Open the group and get the number of objects
        group_id = H5Gopen2(file_id, citem, H5P_DEFAULT);
        status = H5Gget_num_objs(group_id, &nobj);
        // close it up
        status = H5Gclose(group_id);
        nmember = (int) nobj;
    }
    else
    {
        nmember = 0;
    }
    // Free space
    free(citem);
    return nmember;
}
//============================================================================//
/*!
 * @brief Gets the size of an array with name citem
 *
 * @param[in] file_id       HDF5 file handle 
 * @param[in] citem         null terminated name of item of which to 
 *                          identify size
 *
 * @result length of citem
 *
 * @author Ben Baker, ISTI
 *
 */
int h5_get_array_size(const hid_t file_id, const char *citem)
{
    const char *fcnm = "h5_get_array_size\0";
    char *citem_hdf5;
    int nwork_out;
    hid_t dataspace_id, dataset_id;
    hsize_t nwork;
    herr_t status;
    //------------------------------------------------------------------------//
    //  
    // Copy h5 handle and item name
    citem_hdf5 = (char *)calloc(strlen(citem)+1, sizeof(char));
    strcpy(citem_hdf5,citem);
    // Open dataset
    dataset_id = H5Dopen(file_id, citem_hdf5, H5P_DEFAULT);
    // Open dataspace 
    dataspace_id = H5Dget_space(dataset_id);
    // Get the size of the dataspace 
    nwork = (hsize_t) (H5Sget_simple_extent_npoints(dataspace_id));
    // Close it up 
    status  = H5Sclose(dataspace_id);
    status += H5Dclose(dataset_id);
    if (status != 0)
    {
        log_errorF("%s: Error closing h5\n", fcnm);
        return (int) nwork;
    }   
    free(citem_hdf5);
    nwork_out = (int) nwork;
    return nwork_out;
}
//============================================================================//
/*!
 * @brief Determines if an item exists
 *
 * @param[in] file_id       HDF5 file handle
 * @param[in] citem_in      name of item to find
 * 
 * @result 1 indicates item exists; 0 if the item does not exist
 *
 * @author Ben Baker, ISTI
 *
 */
bool h5_item_exists(const hid_t file_id, const char *citem_in)
{
    char *citem = (char *)calloc(strlen(citem_in)+1, sizeof(char));
    int lexist;
    bool exist;
    strcpy(citem, citem_in);
    lexist = H5Lexists(file_id, citem, H5P_DEFAULT);
    if (lexist == 1)
    {
        exist = true;
    }
    else
    {
        exist = false;
    }
    free(citem);
    return exist;
}
//============================================================================//
/*! 
 * @brief Creates a group in HDF5 with name cgroup
 * 
 * @param[in] cgroup     name of group to create
 * @param[in] file_id    HDF5 file handle 
 *
 * @result 0 indicates success
 *
 * @author Ben Baker, ISTI
 *
 */
herr_t h5_create_group(const hid_t file_id, const char *cgroup)
{
    const char *fcnm = "h5_create_group\0";
    char *cgroup_hdf5;
    int len_item = (int) (strlen(cgroup));
    hid_t group_id;
    herr_t status;
    int lexist;
    //------------------------------------------------------------------------//
    //  
    // Set group name 
    cgroup_hdf5 = (char *)calloc((size_t) (len_item+1), sizeof(char));
    strncpy(cgroup_hdf5, cgroup, len_item);
    // Make sure group does not exist 
    lexist = H5Lexists(file_id, cgroup, H5P_DEFAULT);
    if (lexist == 1)
    {
        log_warnF("%s: Warning group exists; skipping\n", fcnm);
        free(cgroup_hdf5);
        return 0;
    }
    // Create group
    group_id = H5Gcreate2(file_id, cgroup_hdf5, H5P_DEFAULT, H5P_DEFAULT,
                          H5P_DEFAULT);
    // Close group
    status = H5Gclose(group_id);
    if (status < 0)
    {
        log_errorF("%s: Error closing group\n", fcnm);
    }
    // Free space
    free(cgroup_hdf5);
    return status;
}
