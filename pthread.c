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

void* sum_subarray(void* arg )/*用于计算加法的函数，未来会用SMID优化*/
 	{
 	        subay sub_array=(subay)arg;
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
		 pthread_t *THREAD_ID = malloc(THREAD_NUMBER * sizeof(pthread_t));
    if (THREAD_ID == NULL) {
        perror("Failed to allocate memory for THREAD_ID");
        return -1;
    }

    subay subarrayi = malloc(sizeof(struct subarray) * THREAD_NUMBER);
    if (subarrayi == NULL) {
        perror("Failed to allocate memory for subarrayi");
        free(THREAD_ID);
        return -1;
    }
		
		int last=len%THREAD_NUMBER;
		int once=len/THREAD_NUMBER;
		int begin=0;
		for(int i=0;i<THREAD_NUMBER;i++)
		{
		      (subarrayi+i)->parentarray=ary;
		      (subarrayi+i)->begin=begin;
		      if(i<last)
		      begin+=once+1;
		      else 
		      begin+=once;
		      (subarrayi+i)->end=begin;
		}
		for(int i=0;i<THREAD_NUMBER;i++)
			{
				if (pthread_create(THREAD_ID + i, NULL, sum_subarray, (void*)(subarrayi + i)) != 0) {
            perror("Failed to create thread");
            free(THREAD_ID);
            free(subarrayi);
            return -1;
        }
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
	
	int main() {
    int array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int length = sizeof(array) / sizeof(array[0]);
    int result = sub_array(array, length);
    printf("Sum: %d\n", result);
    return 0;
}

