#include<stdio.h>
#include<stdlib.h>


typedef struct 
{
	/*
		Block struct

		valid: does the block belongs to the current program
		dirty: does the block need to be written back to memory when it is replaced
		tag: identifier
		used: to determine which block is LRU, 0 means most recently used, all other blocks will be incremented
	*/

   int valid;
   int dirty;
   int tag;
   int used;
}Block; 


typedef struct
{
	/*
		Set struct

		contains different amounts of blocks, depending on the design of the cache
	*/
	Block* bl;
}Set;


typedef struct
{
	/*
		Cache struct

		contains different amounts of sets, depending on the design of the cache
	*/

	Set* s;
}Cache;
Cache cache;

typedef struct
{
	/*
		Memory Reference struct

		inst: type of reference, (R)ead or (W)rite
		tag: tag of the address
		idx: index of the address in cache
	*/


	char inst;
	int tag;
	int idx;
}Ref;
Ref ref[100];


int log2_int(int x)
{
	/*
		Returns base 2 of an integer
	*/

   int ret = 0;
   while (x != 1)
   {
		x /= 2;
		ret++; 
   }
   return ret;
}


int cache_search(int s_size, int idx, int tag)
{
	/*
		Finds an address in cache given the set size, index, and tag
		returns 1 if found, 0 if not
	*/

	int ret = 0;
	int j;
	for (j = 0; j < s_size; j++)						// loop through blocks in a set
	{

		if (cache.s[idx].bl[j].tag == tag && cache.s[idx].bl[j].valid)		// if current block is the address and is valid
		{
			ret = 1;									// address was found in cache
			cache.s[idx].bl[j].used = 0;				// make the address the most recently used
		}
		else											// else the block is not the one we are looking for, increment used
		{
			cache.s[idx].bl[j].used++;					
		}
	}

	return ret;
}


int main()
{

	// zero out reference array 
	int i;
	for (i = 0; i < 100; i++)
	{
		ref[i].inst = 0;
		ref[i].tag = 0;
		ref[i].idx = 0;
	}

	/*
	-------------- parse input --------------
	*/
	// get block size, number of sets, set size
	int bl_size;
	int num_sets;
	int s_size;
	scanf("%d", &bl_size);
	scanf("%d", &num_sets);
	scanf("%d", &s_size);
	
	int offset_bits = log2_int(bl_size);                    // offset bits = log2_int(block size)
	int idx_bits = log2_int(num_sets);                      // index bits = log2_int(number of sets)

	// get references
	int ret = 0;
	i = 0;
	while (ret != -1)
	{
		ret = scanf(" %c", &ref[i].inst);                // get reference type
		if (ret != -1)
		{
			unsigned int addr;
			scanf("%d", &addr);					// get address
			addr = addr >> offset_bits;			// get rid of offset bits
			ref[i].tag = addr >> idx_bits;		// tag = address without offset and index bits
			addr = addr << (32-idx_bits);		// put index bits at the top
			addr = addr >> (32-idx_bits);		// put them back at the bottom, getting rid of the tag
			ref[i].idx = addr;
		}
		i++;
	}

	// print cache configuration
	printf("Block size: %d\n", bl_size);
	printf("Number of sets: %d\n", num_sets);
	printf("Associativity: %d\n", s_size);
	printf("Number of offset bits: %d\n", offset_bits);
	printf("Number of index bits: %d\n", idx_bits);
	printf("Number of tag bits: %d\n", 32 - idx_bits - offset_bits);


	// set cache
	cache.s = (Set*) malloc(num_sets * sizeof(Set));		// allocate number sets array to number of sets
	for (i=0; i < num_sets; i++)
	{
		cache.s[i].bl = (Block*) malloc(s_size * sizeof(Block));	// allocate blocks in set
		int j;
		for (j = 0; j < s_size; j++)						// zero out blocks
		{
			cache.s[i].bl[j].dirty = 0;
			cache.s[i].bl[j].tag = 0;
			cache.s[i].bl[j].valid = 0;
			cache.s[i].bl[j].used = 0;
		}
	}
	

	/*
	-------------- Write-Through, No write allocate --------------
	*/
	i = 0;
	int num_ref = 0;										// number of references
	int hits = 0;											// number of hits
	int misses = 0;											// number of misses
	int mem_refs = 0;										// number of memory references
	while (ref[i].inst != 0)								// loop through references
	{
		//printf("Instruction type: %c, tag: %d, idx: %d\n", ref[i].inst, ref[i].tag, ref[i].idx);								

		// get index, tag, instruction type from reference
		int index = ref[i].idx;
		int tag = ref[i].tag;
		char inst = ref[i].inst;
		int found = cache_search(s_size, index, tag);		// find address in cache

		if (found)											// if found increment hits
		{
			//printf("HIT\n");
			hits++;
			if (inst == 'W')								// if write instruction, increment memory references
			{
				//printf("MEMREF\n");
				mem_refs++;	
			}
		}
		else												// else not found, increment misses and memory references
		{
			//printf("MISS\n");
			//printf("MEMREF\n");
			misses++;
			mem_refs++;
			if (inst == 'R')								// if instruction is a read, put address in cache
			{
				int idx_LRU = 0;							// hold index of the LRU block
				int highest_used = 0;						// hold highest used found
				int placed = 0;								// hold if the address was placed in cache
				int j;
				for (j=0; j < s_size; j++)
				{
					if (cache.s[index].bl[j].used > highest_used)	// if the current used is higher than the highest found, set the index of the LRU to the current iteration
					{
						highest_used = cache.s[index].bl[j].used;
						idx_LRU = j;
					}
					if (!cache.s[index].bl[j].valid && !placed)		// if the block is not valid, set it to this address
					{
						cache.s[index].bl[j].valid = 1;
						cache.s[index].bl[j].tag = tag;
						cache.s[index].bl[j].used = 0;
						placed = 1;
					}
				}
				if (!placed)								// if not placed, put in the LRU
				{
					cache.s[index].bl[idx_LRU].valid = 1;
					cache.s[index].bl[idx_LRU].tag = tag;
					cache.s[index].bl[idx_LRU].used = 0;
				}
			}
		}
		//printf("\n");
		num_ref++;	
		i++;
	}

	// print write through no write allocate results
	printf("****************************************\n");
	printf("Write-through with No Write Allocate\n");
	printf("****************************************\n");
	printf("Total number of references: %d\n", num_ref);
	printf("Hits: %d\n", hits);
	printf("Misses: %d\n", misses);
	printf("Memory References: %d\n", mem_refs);


	// zero out cache
	for (i=0; i < num_sets; i++)
	{
		int j;
		for (j = 0; j < s_size; j++)						
		{
			cache.s[i].bl[j].dirty = 0;
			cache.s[i].bl[j].tag = 0;
			cache.s[i].bl[j].valid = 0;
			cache.s[i].bl[j].used = 0;
		}
	}

	// zero out hits, misses, mem references
	hits = 0;
	misses = 0;
	mem_refs = 0;


	/*
	-------------- Write-back, Write allocate --------------
	*/
	//printf("\n");
	i = 0;
	while (ref[i].inst != 0)								// loop through references
	{
		//printf("Instruction type: %c, tag: %d, idx: %d\n", ref[i].inst, ref[i].tag, ref[i].idx);								

		// get index, tag, instruction type from reference
		int index = ref[i].idx;
		int tag = ref[i].tag;
		char inst = ref[i].inst;
		int found = cache_search(s_size, index, tag);		// find address in cache

		if (found)											// if found increment hits
		{
			//printf("HIT\n");
			hits++;
			if (inst == 'W')								// if write instruction, make dirty
			{
				//printf("DIRTY\n");
				int j;
				for (j = 0; j < s_size; j++)
				{
					if (cache.s[index].bl[j].tag == tag)
					{
						cache.s[index].bl[j].dirty = 1;
					}
				}
			}
		}
		else												// else not found, increment misses and memory references
		{
			//printf("MISS\n");
			//printf("MEMREF\n");
			misses++;
			mem_refs++;

			// place address in cache
			int idx_LRU = 0;							// hold index of the LRU block
			int highest_used = 0;						// hold highest used found
			int placed = 0;								// hold if the address was placed in cache
			int j;
			for (j=0; j < s_size; j++)
			{
				if (cache.s[index].bl[j].used > highest_used)	// if the current used is higher than the highest found, set the index of the LRU to the current iteration
				{
					highest_used = cache.s[index].bl[j].used;
					idx_LRU = j;
				}
				if (!cache.s[index].bl[j].valid && !placed)		// if the block is not valid, set it to this address
				{
					cache.s[index].bl[j].valid = 1;
					cache.s[index].bl[j].tag = tag;
					cache.s[index].bl[j].used = 0;
					if (inst == 'W')							// if is a write, make dirty
					{
						//printf("DIRTY\n");
						cache.s[index].bl[j].dirty = 1;
					}
					else
					{
						cache.s[index].bl[j].dirty = 0;
					}
					placed = 1;
				}
			}
			if (!placed)								// if not placed, put in the LRU
			{
				if (cache.s[index].bl[idx_LRU].dirty)	// if data replacing is dirty, increment mem references
				{
					//printf("MEMREF\n");
					mem_refs++;
				}
				cache.s[index].bl[idx_LRU].valid = 1;
				cache.s[index].bl[idx_LRU].tag = tag;
				cache.s[index].bl[idx_LRU].used = 0;
				if (inst == 'W')							// if is a write, make dirty
				{
					//printf("DIRTY\n");
					cache.s[index].bl[j].dirty = 1;
				}
				else
				{
					cache.s[index].bl[j].dirty = 0;
				}
			}
		}
		//printf("\n");
		i++;
	}

	// print write back, write allocate results
	printf("****************************************\n");
	printf("Write-back with Write Allocate\n");
	printf("****************************************\n");
	printf("Total number of references: %d\n", num_ref);
	printf("Hits: %d\n", hits);
	printf("Misses: %d\n", misses);
	printf("Memory References: %d\n", mem_refs);

	// free dynamically allocated blocks and sets
	free(cache.s->bl);
	free(cache.s);
}