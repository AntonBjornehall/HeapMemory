#include <stdio.h>

#define HEAP_CAP 100000
#define CHUNK_CAP 1024

//the "Heap" were we store the bytes
char heap[HEAP_CAP] = {0};
size_t heapSize = 0;

/*Data of a chunk*/
typedef struct {
    void* start;
    size_t sizeOfChunk;
} chunks;

/*A chunkList that contains all the chunks and the propertie data*/
typedef struct {
    int count;
    int sizeOfEmptyChunks;
    int allocatedEmptyChunks[CHUNK_CAP];
    chunks chunkList[CHUNK_CAP];
} chunkList;

/*ChunkList declaration*/
chunkList allocatedBlock;
chunkList freedBlock;


/* Function:    sortingfreedBlock
 * Description: Sorting the freedBlock chunklist in asendingOrder
 * Output:      chunk list is ordered in assending.
 */
void sortingfreedBlock() {
    chunks next;
    int j;
    for(int i = 1; i < freedBlock.count; i++) {
        next = freedBlock.chunkList[i];
        j = i - 1;
        while(j >= 0 && next.sizeOfChunk < freedBlock.chunkList[j].sizeOfChunk) {
            freedBlock.chunkList[j + 1] = freedBlock.chunkList[j];
            j--;
        }
        freedBlock.chunkList[j + 1] = next;

    }
}


/* Function:    removeChunkFromFreed
 * Description: Set the chunk from freed chunks list to zero and set the freedBlock count down by -1.
 * Input:       chunk - the chunk that will be allocated in allocatedBlock and be "Removed" from freedBlock.
 * Output:      the chunk is now set to zero and have no data.
 */
void removeChunkFromFreed(int i, chunks* chunk) {
    chunk->sizeOfChunk = freedBlock.chunkList[freedBlock.count - 1].sizeOfChunk + 1;
    sortingfreedBlock();
    freedBlock.chunkList[freedBlock.count - 1].sizeOfChunk = 0;
    freedBlock.chunkList[freedBlock.count - 1].start = 0;
    --freedBlock.count;
}

/* Function:    avaibleEmptyChunks
 * Description: See if there is any empty chunks in the allocatedChunk list
 * Output:      returning the index of the chunk otherwise return -1 if no one was found.
 */
int avaibleEmptyChunks() {
    if(allocatedBlock.sizeOfEmptyChunks != 0) {
        int i = allocatedBlock.allocatedEmptyChunks[--allocatedBlock.sizeOfEmptyChunks];
        return i;
    }
    return -1;
}

/* Function:    getFreedChunk
 * Description: get the chunk from the freedChunk list.
 * Input:       i - index of the freedChunk
 * Output:      returning the chunk from the freedChunk list
 */
chunks getFreedChunk(int i) {
    chunks tempChunk = freedBlock.chunkList[i];
    return tempChunk;
}

/* Function:    avaibleFreedChunk
 * Description: if there is no size in freedBlock that is equal to size we see if there is one with bigger
                and dived that chunk upp to two chunks so one of them is correct size.
 * Input:       size - the size of the upcoming allocated chunk
 * Output:      returning the index of the chunck that fits from the freedBlock.
 */
int avaibleFreedChunk(size_t size) {
    for(int i = 0; i < freedBlock.count; i++) {
        if(freedBlock.chunkList[i].sizeOfChunk == size) {
            return i;
        } else if(freedBlock.chunkList[i].sizeOfChunk > size) {
            //Creating a new chunk with the leftover of the size that didnt get used.
            int bytesLeft = freedBlock.chunkList[i].sizeOfChunk - size;
            void* ptr = freedBlock.chunkList[i].start + size;
            chunks tempChunk = {ptr, bytesLeft};
            freedBlock.chunkList[freedBlock.count++] = tempChunk;

            //Setting the block that we will return to the size that was demand.
            freedBlock.chunkList[i].sizeOfChunk = size;
            sortingfreedBlock();

            //After we sort the index can have ended up not as we had it before we added the new chunk add sort it.
            for(int i = 0; i < freedBlock.count; i++) {
                if(freedBlock.chunkList[i].sizeOfChunk == size) { 
                    return i;
                }
            }
        }
    }
    return -1;
}

/* Function:    heap_malloc
 * Description: allocate a new chunk that points to a byte in the heap and store its size.
 * Input:       size - the size of the memory in the heap that will be allocated
 * Output:      returning a pointer to its start location in the heap.
 */
void* heap_malloc(size_t size) {    
    //Create a new chunk that will be allocated
    chunks chunk;

    //Check if we have any chunks in the freedList that fits the size.    
    int i = avaibleFreedChunk(size);
    if(i != -1) {
        //We found a chunk and getting that chunk with the correct size and a pointer thats points to it's memmory in the heap.
        chunk = getFreedChunk(i);
        removeChunkFromFreed(i, &freedBlock.chunkList[i]);
        
        //Then need to be allocated again in the allocatedBlock list.
        allocatedBlock.chunkList[allocatedBlock.count++] = chunk;
        return chunk.start;
    }

    
    //Check if there is any allocated chunks in that have been freed and is not in use.
    i = avaibleEmptyChunks();
    if(i != -1) {
        
        void* ptr = heap + heapSize;
        heapSize += size;
        allocatedBlock.chunkList[i].start = ptr;
        allocatedBlock.chunkList[i].sizeOfChunk = size;
        return ptr;
        /*THIS CHUNK IS ALLREADY COUNTED IN THE allocatedBlock.COUNT, SO NO NEED TO ADD ONE MORE*/
    }

    void* ptr = heap + heapSize;
    heapSize += size;
    chunk.start = ptr;
    chunk.sizeOfChunk = size;


    if(i == -1) {
        allocatedBlock.chunkList[allocatedBlock.count++] = chunk;
    }
    return ptr;
}

/* Function:    insertIndexFromChunk
 * Description: insert chunk index to the allocatedEmptyChunk array.
 * Input:       i - index of the chunk that was given in the heap_free function
 * Output:      index is set in the array
 */
void insertIndexFromChunk(int i) {
    allocatedBlock.allocatedEmptyChunks[allocatedBlock.sizeOfEmptyChunks] = i;
    allocatedBlock.sizeOfEmptyChunks++;

}

/* Function:    insertChunckToFreed
 * Description: insert chunk to freed list and set the freed list count to + 1.
 * Input:       chunk - the chunk that was given in the heap_free function
 * Output:      chunk is unusable
 */
void insertChunckToFreed(chunks chunk) {
    chunks tempChunk = {chunk.start, chunk.sizeOfChunk};
    freedBlock.chunkList[freedBlock.count++] = tempChunk;
}

/* Function:    setChunkToNull
 * Description: set the chunk to zero in size and pointer to zero. Thus when we malloc we dont get the old size if we dont give any size.
 * Input:       chunk - the chunk that was given in the heap_free function
 * Output:      chunk is unusable
 */
void setChunkToNull(chunks* chunk) {
    chunk->sizeOfChunk = 0;
    chunk->start = 0;
}


/* Function:    heap_free
 * Description: freed the memory in the heap and rerange the chunks.
 * Input:       *ptr - ptr pointing to a memory in the heap
 * Output:      the chunk is not pointing to the heap anymore
 */
void heap_free(void* ptr) {
    for(int i = 0; i < allocatedBlock.count; i++) {
        if(allocatedBlock.chunkList[i].start == ptr) {
            insertIndexFromChunk(i);
            insertChunckToFreed(allocatedBlock.chunkList[i]);
            sortingfreedBlock();
            setChunkToNull(&allocatedBlock.chunkList[i]);
            break;
        }
    }
}

/*
    TODO:
        FIXA DUMPCHUNKSINFROMATION. (SÅ MAN BÄTTRE KAN SEE ALLA CHUNKS SOM ÄR ALLOKERAD I HEAP)
*/

int main() {
    
    char* ptr = heap_malloc(18);
    for(int i = 0; i < 10; i++) {
        ptr[i] = 'A' + i;
    }
    char* test = heap_malloc(15);
    char* test2 = heap_malloc(3);
    heap_free(ptr);
    heap_free(test);
    heap_free(test2);

    char* ptr2 = heap_malloc(10);
    char* ptr3 = heap_malloc(15);
    char* ptr4 = heap_malloc(17);
    char* ptr5 = heap_malloc(11);
    for(int i = 0; i < allocatedBlock.count; i++) {
        printf("Chunk %d :\n", i + 1);
        printf("Bytes in the chunk: %zu\n", allocatedBlock.chunkList[i].sizeOfChunk);
        printf("This chunk conatins these memmory addresses: \n");
       
        printf("\tStart memmory Address: %d \n", allocatedBlock.chunkList[i].start);
        printf("\tEnd memmory Address: %d \n", allocatedBlock.chunkList[i].start + (allocatedBlock.chunkList[i].sizeOfChunk - 1));
    
    }
    printf("\n");

    return 0;
}