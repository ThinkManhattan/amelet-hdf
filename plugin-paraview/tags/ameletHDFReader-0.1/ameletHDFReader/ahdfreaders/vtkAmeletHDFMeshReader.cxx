#include <vtkAmeletHDFMeshReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkVoxel.h>
#include <vtkPixel.h>
#include <vtkLine.h>
#include <vtkTriangle.h>
#include <vtkTetra.h>
#include <vtkQuad.h>
#include <vtkVertex.h>
#include <vtkHexahedron.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>
#include <vtkCellData.h>
#include <vtkCell.h>
#include "vtkIdList.h"

#include "amelethdf.h"
#include "mesh.h"
#include "unstructuredmesh.h"
#include "structuredmesh.h"

using namespace vtkstd;

#include <map>



#define TRUE            1
#define FALSE           0

int vtkAmeletHDFMeshReader::readUmesh(hid_t meshId, char *name, vtkUnstructuredGrid *ugrid)
{
    std::cout<<"Read umesh"<<std::endl;
    std::cout<<"=========="<<std::endl;
    //Unstructuredmesh umesh;
    hid_t nodes_id, eltype_id, eltnode_id, loc_id;
    nodes_t umeshnodes;
    loc_id = H5Gopen1(meshId,name);
    // read nodes
    if (H5Lexists(loc_id,"nodes",H5P_DEFAULT)!=FALSE)
    {
    nodes_id = H5Dopen1(loc_id,"nodes");
    vtkPoints *points = vtkPoints::New();
    umeshnodes = readNodes(nodes_id);
    for (int i=0 ;i<umeshnodes.nbnodes;i++)
         points->InsertNextPoint(umeshnodes.nodes[i][0],umeshnodes.nodes[i][1],
       		                                        umeshnodes.nodes[i][2]);
    ugrid->SetPoints(points);
    points->Delete();
    //this->UpdateProgress(this->GetProgress()+0.125);
    elttypes_t umeshelttypes;
    eltnodes_t umesheltnodes;
    umeshelttypes.nbelttypes=0;
    umesheltnodes.nbeltnodes=0;
    if (H5Lexists(loc_id,"elementNodes",H5P_DEFAULT)!=FALSE)
    {
    	eltnode_id = H5Dopen1(loc_id,"elementNodes");
        eltype_id = H5Dopen1(loc_id,"elementTypes");
        umeshelttypes = readElementTypes(eltype_id);
        umesheltnodes = readElementNodes(eltnode_id);
    }

    int idnode=0;
    for(int i=0; i<umeshelttypes.nbelttypes;i++)
    {
      	if(umeshelttypes.elttypes[i]==1)
       	{
       		vtkLine * linecell = vtkLine::New();
       		linecell->GetPointIds()->SetId(0,umesheltnodes.eltnodes[idnode]);
       		linecell->GetPointIds()->SetId(1,umesheltnodes.eltnodes[idnode+1]);
       		ugrid->InsertNextCell(linecell->GetCellType(),linecell->GetPointIds());
                linecell->Delete();
       		idnode=idnode+2;
       	}
       	else if(umeshelttypes.elttypes[i]==11)
       	{
       		vtkTriangle * tricell = vtkTriangle::New();
       		tricell->GetPointIds()->SetId(0,umesheltnodes.eltnodes[idnode]);
       		tricell->GetPointIds()->SetId(1,umesheltnodes.eltnodes[idnode+1]);
       		tricell->GetPointIds()->SetId(2,umesheltnodes.eltnodes[idnode+2]);
       		ugrid->InsertNextCell(tricell->GetCellType(),tricell->GetPointIds());
                tricell->Delete();
       		idnode=idnode+3;
       	}
       	else if(umeshelttypes.elttypes[i]==13)
       	{
       		vtkQuad * quadcell = vtkQuad::New();
       		quadcell->GetPointIds()->SetId(0,umesheltnodes.eltnodes[idnode]);
       		quadcell->GetPointIds()->SetId(1,umesheltnodes.eltnodes[idnode+1]);
       		quadcell->GetPointIds()->SetId(2,umesheltnodes.eltnodes[idnode+2]);
       		quadcell->GetPointIds()->SetId(3,umesheltnodes.eltnodes[idnode+3]);
       		ugrid->InsertNextCell(quadcell->GetCellType(),quadcell->GetPointIds());
                quadcell->Delete();
       		idnode=idnode+4;
       	}
       	else if(umeshelttypes.elttypes[i]==101)
       	{
       		vtkTetra * tetracell = vtkTetra::New();
       		tetracell->GetPointIds()->SetId(0,umesheltnodes.eltnodes[idnode]);
       		tetracell->GetPointIds()->SetId(1,umesheltnodes.eltnodes[idnode+1]);
       		tetracell->GetPointIds()->SetId(2,umesheltnodes.eltnodes[idnode+2]);
       		tetracell->GetPointIds()->SetId(3,umesheltnodes.eltnodes[idnode+3]);
       		ugrid->InsertNextCell(tetracell->GetCellType(),tetracell->GetPointIds());
                tetracell->Delete();
       		idnode=idnode+4;
       	}
    }
    //this->UpdateProgress(this->GetProgress()+0.25);
    // Read GroupGroup
    if(umesheltnodes.nbeltnodes==0)
    {
    	//so there is only nodes in mesh
    	for(int i=0;i<ugrid->GetNumberOfPoints();i++)
    	{
    		vtkVertex * vertexcell = vtkVertex::New();
    		vertexcell->GetPointIds()->SetId(0,i);
    		ugrid->InsertNextCell(vertexcell->GetCellType(),vertexcell->GetPointIds());
                vertexcell->Delete();
    	}
    }
    vtkIntArray *grpgroupId = vtkIntArray::New();
    vtkIntArray *groupId = vtkIntArray::New();
    grpgroupId->SetName("groupGroup Id");
    groupId->SetName("group Id");
    // Initialize groupId to 0
    for (int i=0;i<ugrid->GetNumberOfCells();i++)
    {
        grpgroupId->InsertTuple1(i,-1);
        groupId->InsertTuple1(i,-1);
    }

    children_t child;
    hid_t groupGroup_id, group_id;
    if (H5Lexists(loc_id,"groupGroup",H5P_DEFAULT)!=FALSE)
    {
    	groupGroup_id = H5Gopen1(loc_id,"groupGroup");
    	child = read_children_name(loc_id,"groupGroup");
    	group_id = H5Gopen1(loc_id,"group");
    	for(int i=0;i<child.nbchild;i++)
    	{
    		hid_t grpGrp_id = H5Dopen1(groupGroup_id , child.childnames[i]);
    		groupgroup_t grpGrp = readGroupGroup(grpGrp_id,child.childnames[i]);
    		for(int j=0;j<grpGrp.nbeltgroupGroup;j++)
    		{
                    if (H5Lexists(group_id,grpGrp.groupGroupnames[j],H5P_DEFAULT)!=FALSE)
                    {
                        hid_t grp_id = H5Dopen1(group_id,grpGrp.groupGroupnames[j]);
                        ugroup_t grp;
                        if(strcmp(read_string_attribute(group_id, grpGrp.groupGroupnames[j], "type"),"node")!=0)
                        {
    			    grp = readUGroup(grp_id,grpGrp.groupGroupnames[j]);
                            for(int k=0;k<grp.nbeltgroup;k++)
                                grpgroupId->SetTuple1(grp.eltgroup[k],i);
                            H5Dclose(grp_id);
                        }
                    }
    		}
    		H5Dclose(grpGrp_id);
    	}
        H5Gclose(group_id);
    	H5Gclose(groupGroup_id);
        
    	//ugrid->GetCellData()->SetScalars(grpgroupId);
        ugrid->GetCellData()->AddArray(grpgroupId);
        grpgroupId->Delete();
    }
    // set data for each group
    group_id = H5Gopen1(loc_id,"group");
    child = read_children_name(loc_id,"group");
    std::cout<<"read groups"<<std::endl;
    for (int i=0;i<child.nbchild;i++)
    {
        hid_t grp_id = H5Dopen1(group_id,child.childnames[i]);
        ugroup_t grp;
        if(strcmp(read_string_attribute(group_id, child.childnames[i], "type"),"node")!=0)
        {
            grp = readUGroup(grp_id,child.childnames[i]);
            for (int k=0;k<grp.nbeltgroup;k++)
                groupId->SetTuple1(grp.eltgroup[k],i);
        }
        else
        {
            grp = readUGroup(grp_id,child.childnames[i]);
            for(int k=0;k<grp.nbeltgroup;k++)
            {
                vtkVertex * vertexcell = vtkVertex::New();
                vertexcell->GetPointIds()->SetId(0,grp.eltgroup[k]);
                int nbcells = ugrid->GetNumberOfCells();
                ugrid->InsertNextCell(vertexcell->GetCellType(),vertexcell->GetPointIds());
                vertexcell->Delete();
                grpgroupId->InsertTuple1(nbcells,-1);
                groupId->InsertTuple1(nbcells,i);
            }
        }
        H5Dclose(grp_id);
    }
    H5Dclose(group_id);
    //this->UpdateProgress(this->GetProgress()+0.5);
    ugrid->GetCellData()->AddArray(groupId);
    groupId->Delete();
    
    return 1;
    }
    else
    {
	    return 0;
    }
}

int vtkAmeletHDFMeshReader::readSmesh(hid_t meshId, char *name, vtkUnstructuredGrid *sgrid)
{
    std::cout<<"Read smesh"<<std::endl;
    std::cout<<"=========="<<std::endl;
    hid_t cgrid, loc_id;

    loc_id = H5Gopen1(meshId, name);
    //read cartesian grid
    cgrid = H5Gopen1(loc_id,"cartesianGrid");
    axis_t x,y,z;
    x = readAxis(cgrid,"x");
    y = readAxis(cgrid,"y");
    z = readAxis(cgrid,"z");
    H5Gclose(cgrid);

    vector<bool> ptexist;
    int idptexist;
    cout<<"nbx="<<x.nbnodes<<endl;
    cout<<"nby="<<y.nbnodes<<endl;
    cout<<"nbz="<<z.nbnodes<<endl;

    for(int k=0;k<z.nbnodes;k++)
    { 
        for(int j=0;j<y.nbnodes;j++)
        {
            for(int i=0;i<x.nbnodes;i++)
            {
		idptexist = (k*(x.nbnodes)*(y.nbnodes))+(j*x.nbnodes)+i;
                ptexist.push_back(false);
            }
        }
    }
    //this->UpdateProgress(this->GetProgress()+0.125);
    vtkIntArray *grpgroupId = vtkIntArray::New();
    vtkIntArray *groupId = vtkIntArray::New();
    grpgroupId->SetName("groupGroup Id");
    groupId->SetName("group Id");
    

    //read group
    sgroup_t sgroup;
    hid_t group_id;
    hid_t norm_id = -1;
    group_id = H5Gopen1(loc_id,"group");
    children_t child, grpchild;
    if (H5Lexists(loc_id,"normal",H5P_DEFAULT)!=FALSE)
    {
        norm_id=H5Gopen1(loc_id,"normal");
        child=read_children_name(loc_id,"normal");
    }
    grpchild = read_children_name(loc_id,"group");
    std::cout<<"child = "<<grpchild.nbchild<<std::endl;
    snorm_t normals;
    // read groupGroup
    children_t groupGroup;
    hid_t groupGroup_id;
    map<const char*,int> grpgroupId_value;

    int cellId=0;
    int groupValue=-1;
    if (H5Lexists(loc_id,"groupGroup",H5P_DEFAULT)!=FALSE)
    {
        groupGroup_id = H5Gopen1(loc_id,"groupGroup");
    	groupGroup = read_children_name(loc_id,"groupGroup");
    	for(int k=0;k<grpchild.nbchild;k++)
    	{
    	    int grpValue = -1;
            for(int i=0;i<groupGroup.nbchild;i++)
    	    {
                hid_t grpGrp_id = H5Dopen1(groupGroup_id , groupGroup.childnames[i]);
                groupgroup_t grpGrp = readGroupGroup(grpGrp_id,groupGroup.childnames[i]);
                for(int j=0; j<grpGrp.nbeltgroupGroup;j++)
                {
                    if(strcmp(grpGrp.groupGroupnames[j],grpchild.childnames[k])==0)
                        grpValue = i;
                }
                H5Dclose(grpGrp_id);
            }
            grpgroupId_value[grpchild.childnames[k]]=grpValue;
    	}
    	H5Gclose(groupGroup_id);
    }
    //this->UpdateProgress(this->GetProgress()+0.25);
    std::cout<<"nb child="<<child.nbchild<<std::endl;
    for(int i=0;i<grpchild.nbchild;i++)
    {
        hid_t grp_id = H5Dopen1(group_id,grpchild.childnames[i]);
        if(norm_id!=-1)
        {
            if (H5Lexists(norm_id,grpchild.childnames[i],H5P_DEFAULT)!=FALSE)
            {
                hid_t normChildId = H5Dopen1(norm_id,grpchild.childnames[i]);
                std::cout<<"read normals"<<std::endl;
                normals = readNormals(normChildId);
                H5Dclose(normChildId);
            }
        }
        cout<<"read sgroup :"<< grpchild.childnames[i]<<endl;
        sgroup = readSGroup(grp_id,grpchild.childnames[i]);
        H5Dclose(grp_id);
        for(unsigned int j=0;j<sgroup.nbelt;j++)
        {
            if(strcmp(sgroup.entityType,"face")==0)
            {
              
                int ijk[4][3];
                if(norm_id!=-1)
                {
                    if ((strcmp(normals.normals[j],"x+")==0) ||
                        (strcmp(normals.normals[j],"x-")==0))
                    {
                        // point 1
                        ijk[0][0]=sgroup.imin[j];
                        ijk[0][1]=sgroup.jmin[j];
                        ijk[0][2]=sgroup.kmin[j];
                        // point 2
                        ijk[1][0]=sgroup.imin[j];
                        ijk[1][1]=sgroup.jmax[j];
                        ijk[1][2]=sgroup.kmin[j];
                        // point 3
                        ijk[2][0]=sgroup.imin[j];
                        ijk[2][1]=sgroup.jmin[j];
                        ijk[2][2]=sgroup.kmax[j];
                        // point 4
                        ijk[3][0]=sgroup.imin[j];
                        ijk[3][1]=sgroup.jmax[j];
                        ijk[3][2]=sgroup.kmax[j];
                        double point[4][3];
                        for (int ii=0; ii<4; ii++)
                        {
                            unsigned int id = (ijk[ii][2]*(x.nbnodes)*(y.nbnodes))+
                                     (ijk[ii][1]*(x.nbnodes))+ijk[ii][0];
                            
                            ptexist[id]=true;
                        }
                    }
                    if ((strcmp(normals.normals[j],"y+")==0) ||
                        (strcmp(normals.normals[j],"y-")==0))
                    {
                        // point 1
                        ijk[0][0]=sgroup.imin[j];
                        ijk[0][1]=sgroup.jmin[j];
                        ijk[0][2]=sgroup.kmin[j];
                        // point 2
                        ijk[1][0]=sgroup.imax[j];
                        ijk[1][1]=sgroup.jmin[j];
                        ijk[1][2]=sgroup.kmin[j];
                        // point 3
                        ijk[2][0]=sgroup.imin[j];
                        ijk[2][1]=sgroup.jmin[j];
                        ijk[2][2]=sgroup.kmax[j];
                        // point 4
                        ijk[3][0]=sgroup.imax[j];
                        ijk[3][1]=sgroup.jmin[j];
                        ijk[3][2]=sgroup.kmax[j];
                        double point[4][3];
                        for (int ii=0; ii<4; ii++)
                        {
                            unsigned int id = (ijk[ii][2]*(x.nbnodes)*(y.nbnodes))+
                                     (ijk[ii][1]*(x.nbnodes))+ijk[ii][0];
                            ptexist[id]=true;
                        }
                    }
                    if ((strcmp(normals.normals[j],"z+")==0) ||
                        (strcmp(normals.normals[j],"z-")==0))
                    {
                        // point 1
                        ijk[0][0]=sgroup.imin[j];
                        ijk[0][1]=sgroup.jmin[j];
                        ijk[0][2]=sgroup.kmin[j];
                        // point 2
                        ijk[1][0]=sgroup.imax[j];
                        ijk[1][1]=sgroup.jmin[j];
                        ijk[1][2]=sgroup.kmin[j];
                        // point 3
                        ijk[2][0]=sgroup.imin[j];
                        ijk[2][1]=sgroup.jmax[j];
                        ijk[2][2]=sgroup.kmin[j];
                        // point 4
                        ijk[3][0]=sgroup.imax[j];
                        ijk[3][1]=sgroup.jmax[j];
                        ijk[3][2]=sgroup.kmin[j];
                        double point[4][3];
                        for (int ii=0; ii<4; ii++)
                        {
                            unsigned int id = (ijk[ii][2]*(x.nbnodes)*(y.nbnodes))+
                                     (ijk[ii][1]*(x.nbnodes))+ijk[ii][0];
                            ptexist[id]=true;
                        }
                    }
                }
            }
            else if(strcmp(sgroup.entityType,"volume")==0)
            {
                int ijk[8][3];
                // point 1
                ijk[0][0]=sgroup.imin[j];
                ijk[0][1]=sgroup.jmin[j];
                ijk[0][2]=sgroup.kmin[j];
                // point 2
                ijk[1][0]=sgroup.imax[j];
                ijk[1][1]=sgroup.jmin[j];
                ijk[1][2]=sgroup.kmin[j];
                // point 3
                ijk[2][0]=sgroup.imin[j];
                ijk[2][1]=sgroup.jmax[j];
                ijk[2][2]=sgroup.kmin[j];
                // point 4
                ijk[3][0]=sgroup.imax[j];
                ijk[3][1]=sgroup.jmax[j];
                ijk[3][2]=sgroup.kmin[j];
                // point 5
                ijk[4][0]=sgroup.imin[j];
                ijk[4][1]=sgroup.jmin[j];
                ijk[4][2]=sgroup.kmax[j];
                // point 6
                ijk[5][0]=sgroup.imax[j];
                ijk[5][1]=sgroup.jmin[j];
                ijk[5][2]=sgroup.kmax[j];
                // point 7
                ijk[6][0]=sgroup.imin[j];
                ijk[6][1]=sgroup.jmax[j];
                ijk[6][2]=sgroup.kmax[j];
                // point 8
                ijk[7][0]=sgroup.imax[j];
                ijk[7][1]=sgroup.jmax[j];
                ijk[7][2]=sgroup.kmax[j];
                double point[8][3];
                for (int ii=0; ii<8; ii++)
                {
                    unsigned int id = (ijk[ii][2]*(x.nbnodes)*(y.nbnodes))+
                             (ijk[ii][1]*(x.nbnodes))+ijk[ii][0];
                    ptexist[id]=true;
                }
            }
            else if((strcmp(sgroup.entityType,"edge")==0) ||
                    (strcmp(sgroup.entityType,"slot")==0))
            {
                int ijk[2][3];
                // point 1
                ijk[0][0]=sgroup.imin[j];
                ijk[0][1]=sgroup.jmin[j];
                ijk[0][2]=sgroup.kmin[j];
                // point 2
                ijk[1][0]=sgroup.imax[j];
                ijk[1][1]=sgroup.jmax[j];
                ijk[1][2]=sgroup.kmax[j];
                double point[2][3];
                for (int ii=0; ii<2; ii++)
                {
                    unsigned int id = (ijk[ii][2]*(x.nbnodes)*(y.nbnodes))+
                             (ijk[ii][1]*(x.nbnodes))+ijk[ii][0];
                    ptexist[id]=true;
                }
            }
            else if(strcmp(sgroup.type,"node")==0)
            {
                int ijk[3];
                ijk[0]=sgroup.imin[j];
                ijk[1]=sgroup.jmin[j];
                ijk[2]=sgroup.kmin[j];
                unsigned int id = (ijk[2]*(x.nbnodes)*(y.nbnodes))+
                         (ijk[1]*(x.nbnodes))+ijk[0];
                ptexist[id]=true;
            }
        }
    }
    unsigned int nbptexistreal=0;
    unsigned int nbexist=0;
    vtkPoints *xyzpointsreal = vtkPoints::New();
    map <unsigned int,int> ptugridreal;
    for (int k=0; k<z.nbnodes;k++)
    {
	for(int j=0;j<y.nbnodes;j++)
	{
		for(int i=0;i<x.nbnodes;i++)
		{
			idptexist = (k*(x.nbnodes)*(y.nbnodes))+(j*(x.nbnodes))+i;
			if(ptexist[idptexist])
			{
				xyzpointsreal->InsertNextPoint(x.nodes[i],y.nodes[j],z.nodes[k]);
				ptugridreal[idptexist]=nbptexistreal;
				nbptexistreal=nbptexistreal+1;
			}
			nbexist=nbexist+1;
		}
	}
    }
    sgrid->SetPoints(xyzpointsreal);
    xyzpointsreal->Delete();
    ptexist.clear();
    for(int i=0;i<grpchild.nbchild;i++)
    {
        hid_t grp_id = H5Dopen1(group_id,grpchild.childnames[i]);
        groupValue=groupValue+1;
        if(norm_id!=-1)
        {
            if (H5Lexists(norm_id,grpchild.childnames[i],H5P_DEFAULT)!=FALSE)
            {
                hid_t normChildId = H5Dopen1(norm_id,grpchild.childnames[i]);
                std::cout<<"read normals"<<std::endl;
                normals = readNormals(normChildId);
                H5Dclose(normChildId);
            }
        }
        sgroup = readSGroup(grp_id,grpchild.childnames[i]);
        
        H5Dclose(grp_id);
	for(unsigned int j=0;j<sgroup.nbelt;j++)
        {
            if(strcmp(sgroup.entityType,"face")==0)
            {
                int ijk[4][3];
                if(norm_id!=-1)
                {
                    if ((strcmp(normals.normals[j],"x+")==0) ||
                        (strcmp(normals.normals[j],"x-")==0))
                    {
                        // point 1
                        ijk[0][0]=sgroup.imin[j];
                        ijk[0][1]=sgroup.jmin[j];
                        ijk[0][2]=sgroup.kmin[j];
                        // point 2
                        ijk[1][0]=sgroup.imin[j];
                        ijk[1][1]=sgroup.jmax[j];
                        ijk[1][2]=sgroup.kmin[j];
                        // point 3
                        ijk[2][0]=sgroup.imin[j];
                        ijk[2][1]=sgroup.jmin[j];
                        ijk[2][2]=sgroup.kmax[j];
                        // point 4
                        ijk[3][0]=sgroup.imin[j];
                        ijk[3][1]=sgroup.jmax[j];
                        ijk[3][2]=sgroup.kmax[j];
                        double point[4][3];
                        vtkPixel *pixelcell = vtkPixel::New();
                        for (int ii=0; ii<4; ii++)
                        {
                            unsigned int idtemp = (ijk[ii][2]*(x.nbnodes)*(y.nbnodes))+
                                     (ijk[ii][1]*(x.nbnodes))+ijk[ii][0];
			    unsigned int id = ptugridreal[idtemp];
                            sgrid->GetPoint(id,point[ii]);
                            pixelcell->GetPointIds()->SetId(ii,id);
                        }
                        sgrid->InsertNextCell(pixelcell->GetCellType(),
                                              pixelcell->GetPointIds());
                        pixelcell->Delete();
                        grpgroupId->InsertTuple1(cellId,grpgroupId_value[grpchild.childnames[i]]);
                        
                        cellId++;
                    }
                    if ((strcmp(normals.normals[j],"y+")==0) ||
                        (strcmp(normals.normals[j],"y-")==0))
                    {
                        // point 1
                        ijk[0][0]=sgroup.imin[j];
                        ijk[0][1]=sgroup.jmin[j];
                        ijk[0][2]=sgroup.kmin[j];
                        // point 2
                        ijk[1][0]=sgroup.imax[j];
                        ijk[1][1]=sgroup.jmin[j];
                        ijk[1][2]=sgroup.kmin[j];
                        // point 3
                        ijk[2][0]=sgroup.imin[j];
                        ijk[2][1]=sgroup.jmin[j];
                        ijk[2][2]=sgroup.kmax[j];
                        // point 4
                        ijk[3][0]=sgroup.imax[j];
                        ijk[3][1]=sgroup.jmin[j];
                        ijk[3][2]=sgroup.kmax[j];
                        double point[4][3];
                        vtkPixel *pixelcell = vtkPixel::New();
                        for (int ii=0; ii<4; ii++)
                        {
                            unsigned int idtemp = (ijk[ii][2]*(x.nbnodes)*(y.nbnodes))+
                                     (ijk[ii][1]*(x.nbnodes))+ijk[ii][0];
			    unsigned int id = ptugridreal[idtemp];
                            sgrid->GetPoint(id,point[ii]);
                            pixelcell->GetPointIds()->SetId(ii,id);
                        }
                        sgrid->InsertNextCell(pixelcell->GetCellType(),
                                              pixelcell->GetPointIds());
                        pixelcell->Delete();
                        grpgroupId->InsertTuple1(cellId,grpgroupId_value[grpchild.childnames[i]]);
                        cellId++;
                    }
                    if ((strcmp(normals.normals[j],"z+")==0) ||
                        (strcmp(normals.normals[j],"z-")==0))
                    {
                        // point 1
                        ijk[0][0]=sgroup.imin[j];
                        ijk[0][1]=sgroup.jmin[j];
                        ijk[0][2]=sgroup.kmin[j];
                        // point 2
                        ijk[1][0]=sgroup.imax[j];
                        ijk[1][1]=sgroup.jmin[j];
                        ijk[1][2]=sgroup.kmin[j];
                        // point 3
                        ijk[2][0]=sgroup.imin[j];
                        ijk[2][1]=sgroup.jmax[j];
                        ijk[2][2]=sgroup.kmin[j];
                        // point 4
                        ijk[3][0]=sgroup.imax[j];
                        ijk[3][1]=sgroup.jmax[j];
                        ijk[3][2]=sgroup.kmin[j];
                        double point[4][3];
                        vtkPixel *pixelcell = vtkPixel::New();
                        for (int ii=0; ii<4; ii++)
                        {
                            unsigned int idtemp = (ijk[ii][2]*(x.nbnodes)*(y.nbnodes))+
                                     (ijk[ii][1]*(x.nbnodes))+ijk[ii][0];
			    unsigned int id = ptugridreal[idtemp];
                            sgrid->GetPoint(id,point[ii]);
                            pixelcell->GetPointIds()->SetId(ii,id);
                        }
                        sgrid->InsertNextCell(pixelcell->GetCellType(),
                                              pixelcell->GetPointIds());
                        pixelcell->Delete();
                        grpgroupId->InsertTuple1(cellId,grpgroupId_value[grpchild.childnames[i]]);
                        cellId++;
                    }
                }
                groupId->InsertTuple1(cellId-1,groupValue);
            }
            else if(strcmp(sgroup.entityType,"volume")==0)
            {
                int ijk[8][3];
                // point 1
                ijk[0][0]=sgroup.imin[j];
                ijk[0][1]=sgroup.jmin[j];
                ijk[0][2]=sgroup.kmin[j];
                // point 2
                ijk[1][0]=sgroup.imax[j];
                ijk[1][1]=sgroup.jmin[j];
                ijk[1][2]=sgroup.kmin[j];
                // point 3
                ijk[2][0]=sgroup.imin[j];
                ijk[2][1]=sgroup.jmax[j];
                ijk[2][2]=sgroup.kmin[j];
                // point 4
                ijk[3][0]=sgroup.imax[j];
                ijk[3][1]=sgroup.jmax[j];
                ijk[3][2]=sgroup.kmin[j];
                // point 5
                ijk[4][0]=sgroup.imin[j];
                ijk[4][1]=sgroup.jmin[j];
                ijk[4][2]=sgroup.kmax[j];
                // point 6
                ijk[5][0]=sgroup.imax[j];
                ijk[5][1]=sgroup.jmin[j];
                ijk[5][2]=sgroup.kmax[j];
                // point 7
                ijk[6][0]=sgroup.imin[j];
                ijk[6][1]=sgroup.jmax[j];
                ijk[6][2]=sgroup.kmax[j];
                // point 8
                ijk[7][0]=sgroup.imax[j];
                ijk[7][1]=sgroup.jmax[j];
                ijk[7][2]=sgroup.kmax[j];
                double point[8][3];
                vtkVoxel *voxelcell = vtkVoxel::New();
                for (int ii=0; ii<8; ii++)
                {
                    unsigned int idtemp = ijk[ii][2]*(x.nbnodes)*(y.nbnodes)+
                             ijk[ii][1]*(x.nbnodes)+ijk[ii][0];
		    unsigned int id = ptugridreal[idtemp];
                    sgrid->GetPoint(id,point[ii]);
                    voxelcell->GetPointIds()->SetId(ii,id);
                }
                sgrid->InsertNextCell(voxelcell->GetCellType(),voxelcell->GetPointIds());
                voxelcell->Delete();
                grpgroupId->InsertTuple1(cellId,grpgroupId_value[grpchild.childnames[i]]);
                groupId->InsertTuple1(cellId,groupValue);
                
                cellId++;
            }
            else if((strcmp(sgroup.entityType,"edge")==0) ||
                    (strcmp(sgroup.entityType,"slot")==0))
            {
                int ijk[2][3];
                // point 1
                ijk[0][0]=sgroup.imin[j];
                ijk[0][1]=sgroup.jmin[j];
                ijk[0][2]=sgroup.kmin[j];
                // point 2
                ijk[1][0]=sgroup.imax[j];
                ijk[1][1]=sgroup.jmax[j];
                ijk[1][2]=sgroup.kmax[j];
                vtkLine *linecell = vtkLine::New();
                double point[2][3];
                for (int ii=0; ii<2; ii++)
                {
                    unsigned int idtemp = ijk[ii][2]*(x.nbnodes)*(y.nbnodes)+
                             ijk[ii][1]*(x.nbnodes)+ijk[ii][0];
		    unsigned int id = ptugridreal[idtemp];
                    sgrid->GetPoint(id,point[ii]);
                    linecell->GetPointIds()->SetId(ii,id);
                }
                sgrid->InsertNextCell(linecell->GetCellType(),linecell->GetPointIds());
                linecell->Delete();
                grpgroupId->InsertTuple1(cellId,grpgroupId_value[grpchild.childnames[i]]);
                groupId->InsertTuple1(cellId,groupValue);
                
                cellId++;
            }
            else if(strcmp(sgroup.type,"node")==0)
            {
                int ijk[3];
                ijk[0]=sgroup.imin[j];
                ijk[1]=sgroup.jmin[j];
                ijk[2]=sgroup.kmin[j];
                vtkVertex *vertexcell = vtkVertex::New();
                double point[3];
                unsigned int idtemp = ijk[2]*(x.nbnodes)*(y.nbnodes)+
                         ijk[1]*(x.nbnodes)+ijk[0];
		unsigned int id = ptugridreal[idtemp];
                sgrid->GetPoint(id,point);
                vertexcell->GetPointIds()->SetId(0,id);
                sgrid->InsertNextCell(vertexcell->GetCellType(),vertexcell->GetPointIds());
                vertexcell->Delete();
                grpgroupId->InsertTuple1(cellId,grpgroupId_value[grpchild.childnames[i]]);
                groupId->InsertTuple1(cellId,groupValue);
                cellId++;
            }
        }
    }
    ptugridreal.clear();
    if(norm_id!=-1) H5Gclose(norm_id);
    H5Gclose(group_id);
    H5Gclose(loc_id);
    //this->UpdateProgress(this->GetProgress()+0.5);
    sgrid->GetCellData()->AddArray(grpgroupId);
    sgrid->GetCellData()->AddArray(groupId);
    grpgroupId->Delete();
    groupId->Delete();


    return 1;
}
