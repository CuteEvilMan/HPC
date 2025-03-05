#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define THREAD_NUMBER 5
typedef struct subarray
{
	int *parentarray;
	int begin;
	int end;
} *subay;

void* sum_subarray(void* sub_array )/*用于计算加法的函数，未来会用SMID优化*/
 	{
		int *result=(int*)malloc(sizeof(int));
		*result=0;
		for(int i=sub_array->begin;i<sub_array->end;i++)
		{
			*result+=(sub_array->parentarray)[i];
		}
		return result;
	}
  int sub_array(int *ary,int len)
	{
		pthread_t *THREAD_ID	    =(pthread_t*)malloc(THREAD_NUMBER*sizeof(pthread_t) );
		subay subarrayi = (subay)malloc(sizeof(struct subarray) *THREAD_NUMBER ) ;
		
		int last=len%THREAD_NUMBER;
		int once=len/THREAD_NUMBER;
		++once;
		for(int i=THREAD_NUMBER-last;i<THREAD_NUMBER;++i)
		{
			(subarrayi+i)->parentarray=ary;
			(subarrayi+i)->begin=len-once*(THREAD_NUMBER-i)-once;
			(subarrayi+i)->end=(subarrayi+i)->begin+once;
		}
		once--;
		for(int i=0;i<THREAD_NUMBER-last;++i)
		{
			(subarrayi+i)->parentarray=ary;
			(subarrayi+i)->begin=once*i;
			(subarrayi+i)->end=(subarrayi+i)->begin+once;
		
		}
		for(int i=0;i<THREAD_NUMBER;i++)
			{
				pthread_create(THREAD_ID+i,NULL,sum_subarray,(void*)(subarrayi+i));
		}
		int result=0;
		void * usefor;
		for(int i=0;i<THREAD_NUMBER;i++)
		{
			pthread_join(THREAD_ID[i],&usefor);
			result+=*(int*)usefor;
			free(usefor);
		}
		free(THREAD_ID);
		free(subarrayi);
		return result;
	}  
