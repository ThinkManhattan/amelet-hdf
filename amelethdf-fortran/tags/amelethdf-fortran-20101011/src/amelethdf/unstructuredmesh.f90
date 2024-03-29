module unstructuredmesh_m
    use h5lt
    use h5tb
    use amelethdf_m, only : check, hdferr, &
                            EL => ELEMENT_NAME_LENGTH, &
                            AL => ABSOLUTE_PATH_NAME_LENGTH, &
                            read_children_name, trim_null_char, &
                            read_attribute
    use mesh_m, only : groupgroup_t, print_groupgroup, read_groupgroup, &
                       groupgroup_clear_content
    use hdfpath_m, only : exists, join

    implicit none

    character(len=*), parameter :: MSIG = "[unstructuredmesh]"

    character(len=*), parameter :: NODES = "/nodes"
    character(len=*), parameter :: ELEMENTS = "/elementTypes"
    character(len=*), parameter :: NODE_TYPE = "node"
    character(len=*), parameter :: ELEMENT_TYPE = "element"
    character(len=*), parameter :: EDGE_ENTITY_TYPE = "edge"
    character(len=*), parameter :: SURFACE_ENTITY_TYPE = "face"
    character(len=*), parameter :: FACE_ENTITY_TYPE = "face"
    character(len=*), parameter :: VOLUME_ENTITY_TYPE = "volume"
    character(len=*), parameter :: ELEMENT_NODES = "/elementNodes"
    character(len=*), parameter :: S_GROUP = "/group"
    character(len=*), parameter :: GROUPGROUP = "/groupGroup"
    character(len=*), parameter :: SELECTOR_ON_MESH = "/selectorOnMesh"
    character(len=*), parameter :: TC_SHORTNAME = "shortName"
    character(len=*), parameter :: TC_INDEX = "index"
    character(len=*), parameter :: TC_V1 = "v1"
    character(len=*), parameter :: TC_V2 = "v2"
    character(len=*), parameter :: TC_V3 = "v3"
    character(len=*), parameter :: L_ELEMENTS = "/elements"

    type selector_on_mesh_node_t
        character(len=EL), dimension(:), allocatable :: short_name
        integer, dimension(:), allocatable :: index
    end type selector_on_mesh_node_t

    type selector_on_mesh_element_t
        character(len=EL), dimension(:), allocatable  :: short_name
        integer, dimension(:), allocatable  :: index
        real, dimension(:), allocatable  :: v1, v2, v3
    end type selector_on_mesh_element_t

    type group_t
        character(len=AL) :: name = ""
        character(len=EL) :: type = ""
        character(len=EL) :: entity_type = ""
        integer, dimension(:), allocatable :: elements
    end type group_t

    type unstructured_mesh_t
        character(len=AL) :: name = ""
        real, dimension(:,:), allocatable :: nodes
        integer, dimension(:), allocatable :: elements
        integer, dimension(:), allocatable :: offsets
        integer, dimension(:), allocatable :: element_nodes
        type(group_t), dimension(:), allocatable :: groups
        type(groupgroup_t), dimension(:), allocatable :: groupgroups
        type(selector_on_mesh_node_t) :: som_node
        type(selector_on_mesh_element_t) :: som_element
    end type unstructured_mesh_t

    contains
        ! Reads an ustructured mesh
        subroutine read(file_id, mesh_path, umesh)
            integer(hid_t), intent(in) :: file_id
            character(len=*), intent(in) :: mesh_path
            type(unstructured_mesh_t), intent(inout) :: umesh

            integer :: type_class
            integer(size_t) :: type_size
            integer(hsize_t), dimension(2) :: two_dims
            integer(hsize_t), dimension(1) :: one_dims

            integer :: i, n
            character(len=AL) :: path
            character(len=AL) :: group_path
            character(len=EL), dimension(:), allocatable :: group_name


            call clear_content(umesh)

            umesh%name = ""
            umesh%name = mesh_path

            ! nodes
            path = ""
            path = trim(mesh_path)//NODES
            if (exists(file_id, path)) then
                ! number of nodes
                call h5ltget_dataset_info_f(file_id, path, two_dims, &
                                            type_class, type_size, hdferr)
                call check(MSIG//"Can't read the number of nodes for :"//path)

                ! nodes
                allocate(umesh%nodes(two_dims(1), two_dims(2)))
                call h5ltread_dataset_float_f(file_id, path, umesh%nodes, &
                                              two_dims, hdferr)
                call check(MSIG//"Can't read the nodes' coordinates for :"//path)
            endif

            ! elements
            path = ""
            path = trim(mesh_path)//ELEMENTS
            if (exists(file_id, path)) then
                ! number of elements
                call h5ltget_dataset_info_f(file_id, path, one_dims, &
                                            type_class, type_size, hdferr)
                call check(MSIG//"Can't read the number of elements for :"//path)

                ! elements
                allocate(umesh%elements(one_dims(1)))
                call h5ltread_dataset_f(file_id, path, H5T_NATIVE_INTEGER_4, &
                                        umesh%elements, one_dims, hdferr)
                call check(MSIG//"Can't read the element's type for :"//path)
            endif

            ! number of elements nodes
            path = ""
            path = trim(mesh_path)//ELEMENT_NODES
            if (exists(file_id, path)) then
                call h5ltget_dataset_info_f(file_id, path, one_dims, &
                                            type_class, type_size, hdferr)
                call check(MSIG//"Can't read the number of element nodes"//path)

                ! elements
                allocate(umesh%element_nodes(one_dims(1)))
                call h5ltread_dataset_int_f(file_id, path, umesh%element_nodes, &
                                            one_dims, hdferr)
                call check(MSIG//"Can't read the element nodes"//path)
            endif

            ! groups
            path = ""
            path = trim(mesh_path)//S_GROUP
            if (exists(file_id, path)) then
                if (allocated(group_name)) deallocate(group_name)
                call read_children_name(file_id, path, group_name)
                n = size(group_name)
                allocate(umesh%groups(n))
                do i=1,n
                    group_path = trim(path)//"/"//trim(group_name(i))
                    call read_group(file_id, trim(group_path), umesh%groups(i))
                enddo
            endif

            ! groupGroups
            path = ""
            path = trim(mesh_path)//GROUPGROUP
            if (exists(file_id, path)) then
                if (allocated(group_name)) deallocate(group_name)
                call read_children_name(file_id, path, group_name)
                n = size(group_name)
                allocate(umesh%groupgroups(n))
                do i=1,n
                    group_path = trim(path)//"/"//trim(group_name(i))
                    call read_groupgroup(file_id, trim(group_path), &
                                         umesh%groupgroups(i))
                enddo
            endif

            path = ""
            path = trim(mesh_path)//SELECTOR_ON_MESH
            if (exists(file_id, path)) then
                ! selectorOnMesh/nodes
                path = ""
                path = trim(mesh_path)//SELECTOR_ON_MESH//NODES
                if (exists(file_id, path)) then
                    call read_selector_on_mesh_node(file_id, trim(path), &
                                                    umesh%som_node)
                endif

                ! selectorOnMesh/elements
                path = ""
                path = trim(mesh_path)//SELECTOR_ON_MESH//L_ELEMENTS
                if (exists(file_id, path)) then
                    call read_selector_on_mesh_element(file_id, trim(path), &
                                                       umesh%som_element)
                endif
            endif
        end subroutine read

        ! Reads an unstructured group
        subroutine read_group(file_id, path, group)
            integer(hid_t), intent(in) :: file_id
            character(len=*), intent(in) :: path
            type(group_t), intent(inout) :: group

            integer(hsize_t), dimension(1) :: dims
            integer :: type_class
            integer(size_t) :: type_size
            logical :: here

            ! Name setting
            group%name = ""
            group%name = trim(path)

            ! type attribute
            group%type = ""
            here = read_attribute(file_id, path, "type", group%type)
            group%entity_type = ""
            if (group%type == ELEMENT_TYPE) then
                here = read_attribute(file_id, path, "entityType", &
                                      group%entity_type)
            endif

            ! dataset info
            call h5ltget_dataset_info_f(file_id, path, dims, type_class, &
                                        type_size, hdferr)
            call check(MSIG//"Can't read info for : "//path)

            ! values
            allocate(group%elements(dims(1)))
            call h5ltread_dataset_int_f(file_id, path, group%elements, &
                                        dims, hdferr)
            call check(MSIG//"Can't read values for : "//path)
        end subroutine read_group

        ! read selector on mesh / node
        ! the selector on mesh element is a table (shortName, index)
        ! shortName is an EL string
        ! index is an integer
        subroutine read_selector_on_mesh_node(file_id, path, somn)
            integer(hid_t), intent(in) :: file_id
            character(len=*), intent(in) :: path
            type(selector_on_mesh_node_t), intent(inout) :: somn

            integer(hsize_t) :: nrecords, nfields, start
            integer(size_t) :: type_size
            character(LEN=EL), dimension(:), allocatable :: field_names
            integer(size_t), dimension(:), allocatable :: field_sizes
            integer(size_t), dimension(:), allocatable :: field_offsets

            integer :: i
            character(len=20), dimension(:), allocatable :: cbuf

            print *, "1 - Path : ", path
            call h5tbget_table_info_f(file_id, path, nfields, nrecords, hdferr)
            call check(MSIG//"Can't read table info for"//path)
            print *, "2 - nfields : ", nfields, ", nrecords : ", nrecords

            allocate(field_names(nfields))
            allocate(field_sizes(nfields))
            allocate(field_offsets(nfields))
!            call h5tbget_field_info_f(file_id, path, nfields, field_names, &
!                                      field_sizes, field_offsets, type_size, &
!                                      hdferr)
            call check(MSIG//"Can't read field info for"//path)
            print *, "3 - "

            if (allocated(somn%short_name)) deallocate(somn%short_name)
            allocate(somn%short_name(nrecords))
            allocate(cbuf(nrecords))
            start = 0
            type_size = 20
            call h5tbread_field_name_f(file_id, path, TC_SHORTNAME, &
                                       start, nrecords, type_size, &
                                       cbuf, hdferr)
            call check(MSIG//"Can't field values for"//path//"#"//TC_SHORTNAME)
            do i=1,size(cbuf)
                call trim_null_char(cbuf(i))
            enddo
            somn%short_name(:) = cbuf(:)
            print *, "4 - "

            if (allocated(somn%index)) deallocate(somn%index)
            allocate(somn%index(nrecords))
            start = 0
            call h5tget_size_f(H5T_NATIVE_INTEGER, type_size, hdferr)
            call h5tbread_field_name_f(file_id, path, TC_INDEX, &
                                       start, nrecords, type_size, &
                                       somn%index, hdferr)
            call check(MSIG//"Can't field values for"//path//"#"//TC_INDEX)
            print *, "5 - "

            deallocate(field_names, field_sizes, field_offsets, cbuf)
        end subroutine read_selector_on_mesh_node


        ! read selector on mesh / element
        ! the selector on mesh element is a table (shortName, index, v1, v2, v3)
        ! shortName is an EL string
        ! index is an integer
        ! v1, v2, v3 are real
        subroutine read_selector_on_mesh_element(file_id, path, some)
            integer(hid_t), intent(in) :: file_id
            character(len=*), intent(in) :: path
            type(selector_on_mesh_element_t), intent(inout) :: some

            integer(hsize_t) :: nrecords, nfields, start
            integer(size_t) :: type_size
            character(len=EL), dimension(:), allocatable :: field_names
            integer(size_t), dimension(:), allocatable :: field_sizes
            integer(size_t), dimension(:), allocatable :: field_offsets

            integer :: i
            character(len=EL), dimension(:), allocatable :: cbuf

            call h5tbget_table_info_f(file_id, path, nfields, nrecords, hdferr)
            call check(MSIG//"Can't read table info for"//path)

            allocate(field_names(nfields))
            allocate(field_sizes(nfields))
            allocate(field_offsets(nfields))
!            call h5tbget_field_info_f(file_id, path, nfields, field_names, &
!                                      field_sizes, field_offsets, type_size, &
!                                      hdferr)
            call check(MSIG//"Can't read field info for"//path)

            if (allocated(some%short_name)) deallocate(some%short_name)
            allocate(some%short_name(nrecords))
            allocate(cbuf(nrecords))
            start = 0
            type_size = EL
            call h5tbread_field_name_f(file_id, path, TC_SHORTNAME, &
                                       start, nrecords, type_size, &
                                       cbuf, hdferr)
            call check(MSIG//"Can't field values for"//path//"#"//TC_SHORTNAME)
            do i=1,size(cbuf)
                call trim_null_char(cbuf(i))
            enddo
            some%short_name(:) = cbuf(:)

            if (allocated(some%index)) deallocate(some%index)
            allocate(some%index(nrecords))
            start = 0
            call h5tget_size_f(H5T_NATIVE_INTEGER, type_size, hdferr)
            call h5tbread_field_name_f(file_id, path, TC_INDEX, &
                                       start, nrecords, type_size, &
                                       some%index, hdferr)
            call check(MSIG//"Can't field values for"//path//"#"//TC_INDEX)

            if (allocated(some%v1)) deallocate(some%v1)
            allocate(some%v1(nrecords))
            start = 0
            call h5tget_size_f(H5T_NATIVE_REAL, type_size, hdferr)
            call h5tbread_field_name_f(file_id, path, TC_V1, &
                                       start, nrecords, type_size, &
                                       some%v1, hdferr)
            call check(MSIG//"Can't field values for"//path//"#"//TC_V1)

            if (allocated(some%v2)) deallocate(some%v2)
            allocate(some%v2(nrecords))
            start = 0
            call h5tget_size_f(H5T_NATIVE_REAL, type_size, hdferr)
            call h5tbread_field_name_f(file_id, path, TC_V2, &
                                       start, nrecords, type_size, &
                                       some%v2, hdferr)
            call check(MSIG//"Can't field values for"//path//"#"//TC_V2)

            if (allocated(some%v3)) deallocate(some%v3)
            allocate(some%v3(nrecords))
            start = 0
            call h5tget_size_f(H5T_NATIVE_REAL, type_size, hdferr)
            call h5tbread_field_name_f(file_id, path, TC_V3, &
                                       start, nrecords, type_size, &
                                       some%v3, hdferr)
            call check(MSIG//"Can't field values for"//path//"#"//TC_V3)

            deallocate(field_names, field_sizes, field_offsets, cbuf)
        end subroutine read_selector_on_mesh_element

        ! Print subroutines

        ! Prints an unstructured mesh to the console
        subroutine printt(umesh)
            type(unstructured_mesh_t), intent(in) :: umesh

            integer :: i, offset = 1
            integer(kind=8) :: element_type
            integer :: nb

            print *
            print *
            print *, "Unstructured Mesh"
            print *, "Name : ", trim(umesh%name)

            nb = 0
            if (allocated(umesh%nodes)) nb = size(umesh%nodes)/3
            print *, "Number of nodes : ", nb/3
            do i=1, nb/3
                print *, "Node n°", i-1, " :", umesh%nodes(:, i)
            enddo

            nb = 0
            if (allocated(umesh%elements)) nb = size(umesh%elements)
            print *, "Number of elements : ", nb
            do i=1, nb
                element_type = umesh%elements(i)
                print *, "Element n°", i-1, ", type :", umesh%elements(i)
                if (element_type == 1) then
                    print *, "  Node n°1 :", umesh%element_nodes(offset)
                    offset = offset + 1
                    print *, "  Node n°2 :", umesh%element_nodes(offset)
                    offset = offset + 1
                else if (element_type == 2) then
                    print *, "  Node n°1 :", umesh%element_nodes(offset)
                    offset = offset + 1
                    print *, "  Node n°2 :", umesh%element_nodes(offset)
                    offset = offset + 1
                    print *, "  Node n°3 :", umesh%element_nodes(offset)
                    offset = offset + 1
                else if (element_type == 11) then
                    continue
                else if (element_type == 12) then
                    continue
                else if (element_type == 13) then
                    continue
                else if (element_type == 14) then
                    continue
                else if (element_type == 101) then
                    continue
                else if (element_type == 102) then
                    continue
                else if (element_type == 103) then
                    stop
                else if (element_type == 104) then
                    stop
                endif
            enddo

            ! Groups
            nb = 0
            if (allocated(umesh%groups)) nb = size(umesh%groups)
            print *
            print *, "Number of groups : ", nb
            do i=1, nb
                print *, "--name : ",trim(umesh%groups(i)%name)
                print *, "  type : ", trim(umesh%groups(i)%type)
                print *, "  entityType : ", trim(umesh%groups(i)%entity_type)
            enddo

            ! GroupGroups
            nb = 0
            if (allocated(umesh%groupgroups)) nb = size(umesh%groupgroups)
            print *
            print *, "Number of groupGroups : ", nb
            do i=1, nb
                print *, "Name : ", trim(umesh%groupgroups(i)%name)
            enddo

            ! SelectorOnMesh/nodes
            nb = 0
            if (allocated(umesh%som_node%short_name)) &
                nb = size(umesh%som_node%short_name)
            print *
            print *, "Selector on mesh / nodes ..."
            do i=1,nb
                print *, "shortName : ", trim(umesh%som_node%short_name(i)), &
                         ", index : ", umesh%som_node%index(i)
            enddo

            ! SelectorOnMesh/elements
            nb = 0
            if (allocated(umesh%som_element%short_name)) &
                nb = size(umesh%som_element%short_name)
            print *
            print *, "Selector on mesh / elements ..."
            do i=1,nb
                print *, "shortName : ", trim(umesh%som_element%short_name(i)), &
                         ", index : ", umesh%som_element%index(i), &
                         ", v1 : ", umesh%som_element%v1(i), &
                         ", v2 : ", umesh%som_element%v2(i), &
                         ", v3 : ", umesh%som_element%v3(i)
            enddo
        end subroutine printt

        ! Look for a group in mesh with a given name
        function get_group_by_name(umesh, path) result(group)
            type(unstructured_mesh_t), target, intent(in) :: umesh
            character(len=*), intent(in) :: path
            type(group_t), pointer :: group

            integer :: i

            nullify(group)
            if (.not. allocated(umesh%groups)) return

            do i=1, size(umesh%groups)
                if (umesh%groups(i)%name == path) then
                    group => umesh%groups(i)
                endif
            enddo
        end function get_group_by_name


        ! Look for a group in mesh with a given short name (node's name)
        function get_group_by_short_name(umesh, name) result(group)
            type(unstructured_mesh_t), target, intent(in) :: umesh
            character(len=*), intent(in) :: name
            type(group_t), pointer :: group

            character(len=AL) :: path

            path = join((/umesh%name, S_GROUP, name/))
            group => get_group_by_name(umesh, path)
        end function get_group_by_short_name
        

        ! Generates the offsets field of an unstructured mesh
        subroutine generate_offsets(umesh)
            type(unstructured_mesh_t), intent(inout) :: umesh

            integer :: i, nb_element

            nb_element = size(umesh%elements)

            if (allocated(umesh%offsets)) deallocate(umesh%offsets)
            allocate(umesh%offsets(nb_element))

            umesh%offsets(1) = 1
            do i=2, nb_element
                umesh%offsets(i) = umesh%offsets(i-1) &
                     + number_of_nodes(umesh%elements(i-1))
            enddo
        end subroutine generate_offsets

        ! Return a number of nodes array for the element in umesh
        ! inferior or equal than element_index
        subroutine generate_node_numbers(umesh, element_index, node_numbers)
            type(unstructured_mesh_t), intent(in) :: umesh
            integer, intent(in) :: element_index
            integer, dimension(size(umesh%elements)), intent(out) :: node_numbers

            integer :: i

            do i=1, element_index
                node_numbers(i) = number_of_nodes(umesh%elements(element_index))
            enddo
        end subroutine generate_node_numbers

        ! Return the number of nodes for a given element type
        elemental function number_of_nodes(element_type)
            integer, intent(in) :: element_type

            integer :: i, number_of_nodes

            select case(element_type)
                case(1)
                    number_of_nodes = 2
                case(2)
                    number_of_nodes = 3
                case(11)
                    number_of_nodes = 3
                case(12)
                    number_of_nodes = 6
                case(13)
                    number_of_nodes = 4
                case(14)
                    number_of_nodes = 8
                case(15)
                    number_of_nodes = 3
                case(16)
                    number_of_nodes = 3
                case(101)
                    number_of_nodes = 4
                case(102)
                    number_of_nodes = 5
                case(103)
                    number_of_nodes = 6
                case(104)
                    number_of_nodes = 8
                case(105)
                    number_of_nodes = 3
                case(106)
                    number_of_nodes = 4
                case(107)
                    number_of_nodes = 2
            end select
        end function number_of_nodes

        ! Given a selector_on_mesh_element_t (some) object, return the index
        ! matching a short_name
        function get_index_by_short_name_in_some(some, short_name) result(ind)
            type(selector_on_mesh_element_t), intent(in) :: some
            character(len=*), intent(in) :: short_name

            integer :: i, ind

            ind = 0
            do i=1, size(some%short_name)
                if (short_name==trim(some%short_name(i))) then
                    ind = some%index(i)
                    exit
                endif
            enddo
        end function get_index_by_short_name_in_some

        ! Clear content subroutines
        subroutine selector_on_mesh_node_clear_content(somn)
            type(selector_on_mesh_node_t), intent(inout) :: somn

            if (allocated(somn%short_name)) deallocate(somn%short_name)
            if (allocated(somn%index)) deallocate(somn%index)
        end subroutine selector_on_mesh_node_clear_content

        subroutine selector_on_mesh_element_clear_content(some)
            type(selector_on_mesh_element_t), intent(inout) :: some

            if (allocated(some%short_name)) deallocate(some%short_name)
            if (allocated(some%index)) deallocate(some%index)
            if (allocated(some%v1)) deallocate(some%v1)
            if (allocated(some%v2)) deallocate(some%v2)
            if (allocated(some%v3)) deallocate(some%v3)
        end subroutine selector_on_mesh_element_clear_content

        subroutine group_clear_content(group)
            type(group_t), intent(inout) :: group

            group%name = ""
            group%type = ""
            group%entity_type = ""
            if (allocated(group%elements)) deallocate(group%elements)
        end subroutine group_clear_content

        subroutine clear_content(umesh)
            type(unstructured_mesh_t), intent(inout) :: umesh

            integer :: i

            umesh%name = ""
            if (allocated(umesh%nodes)) deallocate(umesh%nodes)
            if (allocated(umesh%elements)) deallocate(umesh%elements)
            if (allocated(umesh%offsets)) deallocate(umesh%offsets)
            if (allocated(umesh%element_nodes)) deallocate(umesh%element_nodes)

            if (allocated(umesh%groups)) then
                do i=1,size(umesh%groups)
                    call group_clear_content(umesh%groups(i))
                enddo
                deallocate(umesh%groups)
            endif

            if (allocated(umesh%groupgroups)) then
                do i=1,size(umesh%groupgroups)
                    call groupgroup_clear_content(umesh%groupgroups(i))
                enddo
                deallocate(umesh%groupgroups)
            endif

            call selector_on_mesh_node_clear_content(umesh%som_node)
            call selector_on_mesh_element_clear_content(umesh%som_element)
        end subroutine clear_content
end module unstructuredmesh_m
