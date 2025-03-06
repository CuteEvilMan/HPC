#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <immintrin.h>
#include <string.h>
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
		int n = sub_array->end - sub_array->begin ;
		size_t new_n = (n + 7) & ~((size_t)7);
                  int *dst = aligned_alloc(32, new_n * sizeof(int));
                  
               if (!dst) {
              perror("内存分配失败");
              return result;
                }    
                
                memcpy(dst, sub_array->parentarray + sub_array->begin, n * sizeof(int));
                
                 for (size_t i = n; i < new_n; i++) {
          dst[i] = 0;
         }   
    __m256i sum_vec = _mm256_setzero_si256();
          
		   for (size_t i = 0; i < new_n; i+=8) {
        // 使用对齐加载，因为我们保证了数据对齐
        __m256i vec = _mm256_load_si256(  (__m256i*) ( dst + i ) );
        sum_vec = _mm256_add_epi32(sum_vec, vec);
    }
      free(dst);
	  __m128i low128 = _mm256_castsi256_si128(sum_vec);
    __m128i high128 = _mm256_extracti128_si256(sum_vec, 1);
    __m128i sum128 = _mm_add_epi32(low128, high128);
    
    // 2. 使用水平加法，将128位向量内的4个32位整数归约为一个标量
    sum128 = _mm_hadd_epi32(sum128, sum128);  // 将4个数变为两个数： [a0+a1, a2+a3, a0+a1, a2+a3]
    sum128 = _mm_hadd_epi32(sum128, sum128);  // 将两个数相加，所有元素相同

   *result= _mm_cvtsi128_si32(sum128);  // 提取第一个元素作为最终和
      
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
	
	
	int main() {//该main函数仅用于测试
    int array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,44,55,6,-2,45,89,56,55,44,55,43,66,22,33,0xAA,'h',23,35,35,11,1,1,1};
    int length = sizeof(array) / sizeof(array[0]);
    int result = sub_array(array, length);
    printf("Sum: %d\n", result);
    return 0;
}

