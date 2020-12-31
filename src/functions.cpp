/* START OF functions.c --------------------------------------------------------
 *
 *      Memory function wrappers mostly.
 *
 *	---
 *	Written by David Eggleston (2006) <deggleston@users.sourceforge.net>
 *	for the 'BeebEm' Acorn BBC Model B, Integra-B, Model B Plus and
 *	Master 128 emulator.
 *
 *	This file is part of 'Economy GUI' and may be copied only under the
 *	terms of either the GNU General Public License (GPL) or Dr. David
 *	Alan Gilbert's BeebEm license.
 *
 *	For more details please visit:
 *
 *	http://www.gnu.org/copyleft/gpl.html
 *	---
 */


#include "include/functions.h"


#define EG_DEBUG_MAXMALLOCS 50000


/* malloc and free:
 */

#ifdef EG_DEBUG
typedef struct{
	void *ptr;
	size_t size;
}MallocAllocList;

static MallocAllocList EG_Malloc_AllocationList[EG_DEBUG_MAXMALLOCS];
static unsigned long EG_Malloc_AllocationCount = 0;

static void EG_Malloc_InitialiseList()
{
	long i;
	for(i=0; i<EG_DEBUG_MAXMALLOCS; i++){
		EG_Malloc_AllocationList[i].ptr = NULL;
		EG_Malloc_AllocationList[i].size = 0;
	}
}

static void EG_Malloc_AddAllocationToList(void *p, size_t size)
{
	long i;
	for(i=0; i<EG_DEBUG_MAXMALLOCS; i++)
		if (EG_Malloc_AllocationList[i].ptr == NULL){
			EG_Malloc_AllocationList[i].ptr = p;
			EG_Malloc_AllocationList[i].size = size;
			return;
		}
}

static size_t EG_Malloc_GetSize(void *p)
{
	long i;
	for(i=0; i<EG_DEBUG_MAXMALLOCS; i++)
		if (EG_Malloc_AllocationList[i].ptr == p)
			return(EG_Malloc_AllocationList[i].size);

	return(0);
}

/* Returns TRUE if removed, FALSE if not found.
 */
static EG_BOOL EG_Malloc_RemoveAllocationFromList(void *p)
{
	long i;
	for(i=0; i<EG_DEBUG_MAXMALLOCS; i++)
		if (EG_Malloc_AllocationList[i].ptr == p){
			EG_Malloc_AllocationList[i].ptr = NULL;
			EG_Malloc_AllocationList[i].size = 0;
			return(EG_TRUE);
		}

	return(EG_FALSE);
}

EG_BOOL EG_Malloc_CanFindAllocationInList(void *p)
{
	EG_PTR_AS_INT pointer, pointer_lowerboundry, pointer_upperboundry;
	long i;

	for(i=0; i<EG_DEBUG_MAXMALLOCS; i++){
		pointer = EG_CAST_PTR_TO_INT(p);
		pointer_lowerboundry =
		 EG_CAST_PTR_TO_INT(EG_Malloc_AllocationList[i].ptr);

		pointer_upperboundry = pointer_lowerboundry
		 + (EG_PTR_AS_INT) EG_Malloc_AllocationList[i].size -1;

		if (pointer >= pointer_lowerboundry && pointer
		 <= pointer_upperboundry)
			return(EG_TRUE);
	}
	return(EG_FALSE);
}
#endif



void* EG_Malloc(size_t size)
{
#	ifdef EG_DEBUG
	static EG_BOOL b = EG_FALSE;

	if (b == EG_FALSE){
		EG_Malloc_InitialiseList();
		b=EG_TRUE;
	}
#	endif
	void *ptr;

	if ( (ptr=malloc(size)) != NULL){
#		ifdef EG_DEBUG
		unsigned long i;
		char *ptr2 = (char*) ptr;

		EG_Malloc_AllocationCount++;
		
		EG_Log(EG_LOG_DEBUG5, dL"malloc(%ld) = "EG_PTRFMT" [%lu]",dR
		 , (long) size, EG_CAST_PTR_TO_INT(ptr), EG_Malloc_AllocationCount);

		EG_Malloc_AddAllocationToList(ptr, size);

		for(i=0; i< (unsigned long) size; i++)
			*(ptr2++)=EG_MALLOCFILLER;
	}else{
		EG_Log(EG_LOG_DEBUG
		 , dL"Unable to malloc %ld bytes (ALL YOUR BYTES ARE BELONG TO US)."
		 , dR, (long) size);
#		endif
	}
	
	return(ptr);
}

/* System free wrapper.
 */
void EG_Free(void *ptr)
{
#	ifdef EG_DEBUG
	size_t i, len;
	char *c_ptr = (char*) ptr;
	// Set buffer to a known state. (if we're freeing stuff that doesn't
	// want to be free'd, then we'll be making the program more unstable in
	// debug mode..  Bonus!
	if ( (len=EG_Malloc_GetSize(ptr)) >0){
		for(i=0; i<len; i++)
			*(c_ptr++) = EG_FREEFILLER;

		EG_Malloc_AllocationCount--;
	
		EG_Log(EG_LOG_DEBUG5, dL"free("EG_PTRFMT") [%lu]",dR
		 , EG_CAST_PTR_TO_INT(ptr), EG_Malloc_AllocationCount);
	}else
		EG_Log(EG_LOG_FATAL, dL"Could not find ["EG_PTRFMT"] in malloc ref, shit.."
	 	 , dR, EG_CAST_PTR_TO_INT(ptr) );

	// Not sure about this..
	if (EG_Malloc_RemoveAllocationFromList(ptr) == EG_TRUE){
		free(ptr);
	}
#	else
	free(ptr);
#	endif
}

/* Returns an unsigned long containing a numerical hash of the passed
 * string using the following:
 */
unsigned long EG_MakeStringHash(char *p)
{
        unsigned long h;
        size_t i;

        h = 0; for (i = 0; i < strlen(p); i++)
                h = 31UL * h + ( (unsigned long) p[i]);

        return(h);
}
