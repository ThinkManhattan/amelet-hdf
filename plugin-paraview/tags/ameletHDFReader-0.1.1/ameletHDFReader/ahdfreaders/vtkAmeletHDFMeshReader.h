#ifndef __VTKvtkAmeletHDFMeshReader_H_
#define __VTKvtkAmeletHDFMeshReader_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <hdf5.h>
#ifdef __cplusplus
}
#endif
#include "vtkUnstructuredGrid.h"



class vtkAmeletHDFMeshReader
{
public:
    virtual int readUmesh(hid_t loc_id, char *name, vtkUnstructuredGrid *ugrid);
    virtual int readSmesh(hid_t loc_id, char *name, vtkUnstructuredGrid *sgrid);

};
#endif /* __VTKvtkAmeletHDFMeshReader_H_  */
