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



typedef struct {
    int recordcount; // Πόσες εγγραφές έχει το block
} HP_Block_Metadata;


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
    int file_handle;          // Το αναγνωριστικό του αρχείου
    int current_block;        // Το block που εξετάζουμε τώρα
    int current_record_index; // Ο δείκτης της εγγραφής μέσα στο block
    int search_id;            // Το ID που ψάχνουμε
} HeapFileIterator;

#endif /* HP_FILE_STRUCTS_H */
