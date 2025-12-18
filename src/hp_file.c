#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#include "bf.h"
#include "hp_file_structs.h"
#include "hp_file_funcs.h"
#include "record.h"

#define CALL_BF(call)         \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      return 0;        \
    }                         \
  }

int HeapFile_Create(const char* fileName)
{

  if ((BF_CreateFile(fileName))!=1 )
    return 0;

  int file_handle;
  //άνοιγμα αρχείου
  BF_OpenFile(fileName , &file_handle );

  //αρχικοποίηση block
  BF_Block *block;
  BF_Block_Init(&block);
  BF_AllocateBlock( file_handle , block);
  void* data = BF_Block_GetData(block);
  HeapFileHeader meta; 

  //αρχικοίηση των μέγιστων εγγραφών ανάλογα το μέγεθος του block
  meta.maxrecords = (BF_BLOCK_SIZE - sizeof(HeapFileIterator)) / sizeof(Record);         ///maxrecords
  meta.freespace = false;
  meta.lastblock = 0;

  memcpy(data , &meta , sizeof(meta));

  BF_Block_SetDirty(block);
  BF_UnpinBlock(block);
  BF_CloseFile(file_handle);
  BF_Block_Destroy(&block);
  
  return 1;
}

int HeapFile_Open(const char *fileName, int *file_handle, HeapFileHeader** header_info)
{
  BF_OpenFile(fileName , file_handle);
  BF_Block *block;
  BF_Block_Init(&block);

  //παίρνουμε το πρώτο block
  BF_GetBlock(*file_handle , 0 , block);

  //δημιουργούμε δείκτη στα δεδομένα του block
  void* data = BF_Block_GetData(block);
  HeapFileHeader** hpInfo = data;    
  BF_Block_Destroy(&block);
  //return hpInfo;
  return 1;
}

int HeapFile_Close(int file_handle, HeapFileHeader *hp_info)
{
  BF_CloseFile(file_handle);
  return 1;
}

int HeapFile_InsertRecord(int file_handle, HeapFileHeader *hp_info, const Record record)
{
  BF_Block *block;
  BF_Block_Init(&block);

  if(hp_info->freespace == false){
    BF_AllocateBlock(file_handle , block);
    hp_info->freespace = true;
    hp_info->lastblock++;

    void* data = BF_Block_GetData(block);

    HeapFileIterator  block_info;
    block_info.recordcount = 0;

    data+= sizeof(block_info);

    Record* rec = data;
    rec[block_info.recordcount] = record;
    block_info.recordcount++;

    data-= sizeof(block_info);

    memcpy(data, &block_info , sizeof(block_info));
  }
  //an exw free space
    else{
    BF_GetBlock(file_handle , hp_info->lastblock , block);
    void* data = BF_Block_GetData(block);
    HeapFileIterator block_info;

    memcpy(&block_info , data, sizeof(block_info));

    data+= sizeof(block_info);

    Record* rec = data;
    rec[block_info.recordcount] = record;
    block_info.recordcount++;

    data-= sizeof(block_info);

    memcpy(data, &block_info , sizeof(block_info));

    if(block_info.recordcount >= hp_info->maxrecords){
      hp_info->freespace = false;
    }
  }

  BF_Block_SetDirty(block);
  BF_UnpinBlock(block);
  BF_Block_Destroy(&block);
  return 1;
  }



HeapFileIterator HeapFile_CreateIterator(    int file_handle, HeapFileHeader* header_info, int id)
{
  BF_Block *block;
  BF_Block_Init(&block);

  for(int i = 1; i <= header_info->lastblock; i++){
    //pairnw ta data
    BF_GetBlock(file_handle , i , block);
    void* data = BF_Block_GetData(block);
    HeapFileIterator block_info;
    
    memcpy(&block_info , data, sizeof(block_info));
    int reccount = block_info.recordcount;
    
    data+= sizeof(block_info);
    
    Record* rec = data;
    
    for(int j = 0; j < reccount; j++){
      if(rec[j].id == id){
        //out.recordcount = &rec[j];
        printRecord(rec[j]);
        
      }
    }
    BF_UnpinBlock(block);
  }
  BF_Block_Destroy(&block);
  HeapFileIterator out;
  return out;
}


int HeapFile_GetNextRecord(    HeapFileIterator* heap_iterator, Record** record)
{
    BF_Block block;
    BF_Block_Init(&block);

    int total_blocks;
    BF_GetBlockCounter(heap_iterator->file_handle, &total_blocks);

    // Συνεχίζουμε από το block που μείναμε
    for (int i = heap_iterator->current_block; i < total_blocks; i++) {
        CALL_BF(BF_GetBlock(heap_iterator->file_handle, i, block));
        void data = BF_Block_GetData(block);

        HP_Block_Metadata block_meta;
        memcpy(&block_meta, data, sizeof(HP_Block_Metadata));

        // Ο δείκτης πηγαίνει μετά τα metadata, εκεί που αρχίζουν οι εγγραφές
        Record* records = (Record*)(data + sizeof(HP_Block_Metadata));

        // Ψάχνουμε τις εγγραφές του block
        for (int j = heap_iterator->current_record_index; j < block_meta.recordcount; j++) {

         if (records[j].id == heap_iterator->search_id) {
                *record = &records[j]; 

                // Αποθηκεύουμε τη θέση μας για την επόμενη κλήση
                heap_iterator->current_block = i;
                heap_iterator->current_record_index = j + 1; // Συνεχίζουμε από την επόμενη

                BF_UnpinBlock(block);
                BF_Block_Destroy(&block);
                return 1; 
            }
        }

        // Τελείωσε το τρέχον block, πάμε στο επόμενο
        heap_iterator->current_block = i + 1;
        heap_iterator->current_record_index = 0; // Reset για το νέο block
        BF_UnpinBlock(block);
    }

    BF_Block_Destroy(&block);
    return 0; 
}

