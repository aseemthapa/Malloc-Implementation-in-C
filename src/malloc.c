#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}


struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};


struct _block *freeList = NULL; /* Free list to track the _blocks available */
struct _block *next_pointer = NULL; /* Free list to track the _blocks available */
struct _block newBlock;
/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = freeList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;      
   }
   //curr->prev = *last;
#endif

#if defined BEST && BEST == 0
   //printf("INSIDE Starting\n");
   curr = freeList;
   size_t min_size = 0;
   struct _block *return_pointer = NULL;
   //printf("%p\n",curr);
   //printf("INSIDE init\n");
   //printf("TODO: Implement best fit here\n");
   while(curr)
   {
   		//printf("INSIDE Main LOOP\n");
   		if(curr->free)
   		{ //For when the pointer has space available
   			//printf("INSIDE CURR = FREE\n");
		   	if(curr->size == size)
		   	{ //if the size is same current is the best fit
		   		return_pointer = curr;
		   		break;
		   	}
		   	else if(curr->size > size)
		   	{ //if the pointer has more space than needed
		   		if(min_size == 0) //WHen the first curr that can fit appearss
		   		{
		   				min_size = curr->size; //change the minimum size to the pointer's size
		   				return_pointer = curr;
		   				*last = curr;
	     				curr  = curr->next;   
	     				continue;
		   		}
		   		else 
		   		{
		   			if(curr->size < min_size) 
		   			{
		   				min_size = curr->size;
		   				return_pointer = curr;
		   				*last = curr;
	     				curr  = curr->next;   
	     				continue;
		   			}
		   			else
		   			{
		   				*last = curr;
	     				curr  = curr->next;
		   			}
		   		}
		   	} 
		   	else //When size of pointer cannot fit the required space
		   	{
		   		*last = curr;
	     		curr  = curr->next; 
		   	}
		}
		else
		{
			*last = curr;
	     	curr  = curr->next; 
		}
   }
   curr = return_pointer;

#endif

#if defined WORST && WORST == 0
   //printf("TODO: Implement worst fit here\n");
   curr = freeList;
   size_t max_size = 0;
   struct _block *return_pointer = NULL;
   //printf("%p\n",curr);
   //printf("INSIDE init\n");
   //printf("TODO: Implement best fit here\n");
   while(curr)
   {
   		//printf("INSIDE Main LOOP\n");
   		if(curr->free)
   		{ 
		   	if(curr->size >= size)
		   	{ //if the pointer has more space than needed	   		
		   			if(curr->size > max_size) 
		   			{
		   				max_size = curr->size;
		   				return_pointer = curr;
		   				*last = curr;
	     				curr  = curr->next;   
	     				continue;
		   			}
		   			else
		   			{
		   				*last = curr;
	     				curr  = curr->next;
		   			}
		   	} 
		   	else //When size of pointer cannot fit the required space
		   	{
		   		*last = curr;
	     		curr  = curr->next; 
		   	}
		}
		else
		{
			*last = curr;
	     	curr  = curr->next; 
		}
   }
   curr = return_pointer;
#endif

#if defined NEXT && NEXT == 0
   //printf("TODO: Implement next fit here\n");
   //printf("Next = %p\n",next_pointer);
   if(next_pointer)
   {
   		curr = next_pointer;
   }
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;      
   }
   next_pointer = curr;
#endif
   //printf("%p\n",curr);   
   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update freeList if not set */
   if (freeList == NULL) 
   {
      freeList = curr;
   }

   /* Attach new _block to prev _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   num_grows++;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */

void split_block(struct _block *curr,size_t size) //Insert a new block between current pointer and the pointer after current pointer
{	
	struct _block* new = &newBlock;
	//new->size = curr->size - size - ALIGN4(sizeof(struct _block)); //The size of new block to be added
	new->free = true; //The created block is free
	new->size = curr->size - size - sizeof(struct _block);
	new->prev = curr;
	new->next = curr->next;
	curr->next = new; 
	curr->size = size; //The block now holds only the essential size
	curr->free = false; //The block that the new block is created from is full.
	num_splits++; //Increase the number of split amounts
	num_blocks++; //Splitting increases no. of blocks
} 

void coalesce_blocks(struct _block *curr){
	//Current pointer is aready free so don't have to change that
	curr->size = curr->size + (curr->next)->size + sizeof(struct _block);
	curr->next = (curr->next)->next; //Point to the next block
	num_coalesces++; //Increase number of coaleces
	num_blocks--; //Coalescing decreases no. of blocks
	// return;
}

void coalesce_blocks_back(struct _block *curr){
	//Current pointer is aready free so don't have to change that
	(curr->prev)->size = (curr->prev)->size + curr->size + sizeof(struct _block);
	(curr->prev)->next = curr->next; //Point to the next block
	num_coalesces++; //Increase number of coaleces
	num_blocks--; //Coalescing decreases no. of blocks
	//return;
}

void *malloc(size_t size) 
{


   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block */
   struct _block *last = freeList;
   struct _block *next = findFreeBlock(&last, size);

   //printf("Curr = %p\n",next);
   //Split if the block size is more than needed
   if((next != NULL) && (next->size > size))
   {
   		//printf("splitting now\n");
   		split_block(next,size);
   		//printf("OUT\n");
   }

   num_reuses++; //Reuses = no.of total requested memory - no. of growheap memory
   //So, increase reuses when malloc is called(new block is requested)
   //And, decrease reuses if, the malloc results in growheap.

   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
      next->prev = last;
      num_reuses--; //By logic for reuses
	  num_blocks++; //When grow heap happens no. of block increases
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   
   /* Mark _block as in use */
   next->free = false;

   num_mallocs++; //No. of times malloc is called

   num_requested = num_requested + size;
   if(size>max_heap) max_heap = size;
   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}



void* calloc(size_t nmemb, size_t size)
{
	
	struct _block* out = malloc(nmemb*size); //Call malloc
	memset(out,0,sizeof(out)+ (nmemb*size)); //memory set to 0
	return out; //Return the initialized pointer
	
}



void *realloc (void* ptr, size_t resize){
	//Implemet realloc here
	//Create a new pointer
	void* newptr;
	int org_size;
	//Calculate the original size:
	org_size = BLOCK_HEADER(ptr)->size;
	//If original size is greater than new size realloc fails
	if(org_size >= resize){
		return ptr;
	}
	//Make a new pointer
	newptr = malloc(resize);
	//Copy everything from original pointer to new pointer
	memcpy(newptr,ptr,org_size);
	//Delete the old pointer:
	free(ptr);
	//return the new pointer
	return newptr;
}


 /* \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
   num_frees++;
   /* TODO: Coalesce free _blocks if needed */

   //DeEBUG STEPS:
   /*
   printf("curr = %p\n",curr);
   printf("prev = %p\n",curr->prev);
   printf("next = %p\n",curr->next);
	*/
   
   //Check for previous block
   if(curr->prev && (curr->prev)->free)
   {
   		//printf("IN prev %p\n",curr->prev);
   		coalesce_blocks_back(curr);   		
   }

   
   //Check for next block
   if(curr->next && (curr->next)->free)
   {
   		//printf("IN next%p\n",curr->next);
   		coalesce_blocks(curr);  	
   }
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
