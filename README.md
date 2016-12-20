Compiling & Running
===================
Simply run `make` to compile and then run `./dos_scandisk path_to_image_file`. 

Important Notes
===============
When printing out unreferenced clusters, the clusters are not sorted by index. Their ordering is defined by the cluster chain/linked list
defined in the file allocation table. For example, if the cluster chains (files) `1 -> 4` and `2 -> 3 -> 5` are unreferenced in the table,
the program will print out `Unreferenced: 1 4 2 3 5` as opposed to not `Unreferenced: 1 2 3 4 5`. 

Files
=====
- FATImage.h and FATImage.c

	Declares and implements `struct FATImage` and main functions for reading, checking and repairing FAT12 floppy images
	
- ClusterChain.h and ClusterChain.c

    Declares and implements `struct ClusterChain` and supporting functions for storing chains of cluster indices (files)
    parsed from a FAT12 file allocation table.
    
- DirectoryEntry.h and DirectoryEntry.c

    Declares and implements `struct DirectoryEntry` and supporting functions for encapsulating information parsed from a FAT12 directory entry. 

- Helpers.h and Helpers.c

    Declares and implements supporting functions for reading and writing FAT12 file system data
    e.g. reading and writing 12-bit Little-endian numbers 

- dos_scandisk.c

    small entry point that initializes `struct FATImage` and runs file system check and repair operations defined in FATImage.h