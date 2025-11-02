#ifndef HP_FILE_STRUCTS_H
#define HP_FILE_STRUCTS_H

#include <record.h>
#include <stdbool.h>

/**
 * @file hp_file_structs.h
 * @brief Data structures for heap file management
 */

/* -------------------------------------------------------------------------- */
/*                              Data Structures                               */
/* -------------------------------------------------------------------------- */

/**
 * @brief Heap file header containing metadata about the file organization
 */
typedef struct HeapFileHeader {
    int maxrecords;
    bool freespace;
    int lastblock;
} HeapFileHeader;

/**
 * @brief Iterator for scanning through records in a heap file
 */
typedef struct HeapFileIterator{
    int recordcount;
} HeapFileIterator;

#endif /* HP_FILE_STRUCTS_H */
