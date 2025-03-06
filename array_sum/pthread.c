#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <immintrin.h>
#include <string.h>

#define THREAD_NUMBER 5
#define ALIGNMENT 32  // AVX2 需要 32 字节对齐

typedef struct {
    int* parent_array;
    int begin;
    int end;
} ThreadTask;

// 函数声明
static int parallel_sum(int* array, int length);
static void* thread_sum(void* arg);
static void create_threads(pthread_t* threads, ThreadTask* tasks, int* array, int length);
static void calculate_subranges(int length, int* starts, int* ends);

// 主逻辑函数
static int parallel_sum(int* array, int length) {
    pthread_t threads[THREAD_NUMBER];
    ThreadTask tasks[THREAD_NUMBER];

    // 初始化任务结构
    int starts[THREAD_NUMBER], ends[THREAD_NUMBER];
    calculate_subranges(length, starts, ends);
    
    for (int i = 0; i < THREAD_NUMBER; ++i) {
        tasks[i] = (ThreadTask){
            .parent_array = array,
            .begin = starts[i],
            .end = ends[i]
        };
    }

    create_threads(threads, tasks, array, length);

    // 收集结果
    int total = 0;
    for (int i = 0; i < THREAD_NUMBER; ++i) {
        void* retval;
        pthread_join(threads[i], &retval);
        total += *(int*)retval;
        free(retval);
    }

    return total;
}

// 线程处理函数（SIMD优化版本）
static void* thread_sum(void* arg) {
    ThreadTask* task = (ThreadTask*)arg;
    const int chunk_size = task->end - task->begin;

    // 内存对齐处理
    const size_t aligned_size = (chunk_size + 7) & ~7;  // 向上对齐到8的倍数
    int* aligned_data = aligned_alloc(ALIGNMENT, aligned_size * sizeof(int));
    
    if (!aligned_data) {
        fprintf(stderr, "Aligned memory allocation failed\n");
        return NULL;
    }

    // 数据拷贝和填充
    memcpy(aligned_data, task->parent_array + task->begin, chunk_size * sizeof(int));
    memset(aligned_data + chunk_size, 0, (aligned_size - chunk_size) * sizeof(int));

    // AVX2 向量化计算
    __m256i sum = _mm256_setzero_si256();
    for (size_t i = 0; i < aligned_size; i += 8) {
        __m256i vec = _mm256_load_si256((__m256i*)(aligned_data + i));
        sum = _mm256_add_epi32(sum, vec);
    }
    free(aligned_data);

    // 向量结果归约
    __m128i low  = _mm256_castsi256_si128(sum);
    __m128i high = _mm256_extracti128_si256(sum, 1);
    __m128i res  = _mm_add_epi32(low, high);
    res = _mm_hadd_epi32(res, res);
    res = _mm_hadd_epi32(res, res);

    // 返回结果
    int* result = malloc(sizeof(int));
    *result = _mm_cvtsi128_si32(res);
    return result;
}

// 计算每个线程处理的子数组范围
static void calculate_subranges(int length, int* starts, int* ends) {
    const int base_chunk = length / THREAD_NUMBER;
    const int remainder = length % THREAD_NUMBER;
    int current_start = 0;

    for (int i = 0; i < THREAD_NUMBER; ++i) {
        starts[i] = current_start;
        ends[i] = current_start += base_chunk + (i < remainder);
    }
}

// 创建并管理线程
static void create_threads(pthread_t* threads, ThreadTask* tasks, int* array, int length) {
	(void)length;	(void)array;
	for (int i = 0; i < THREAD_NUMBER; ++i) {
        if (pthread_create(&threads[i], NULL, thread_sum, &tasks[i])) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }
}

// 测试用例
int main() {
    int test_array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 44, 55, 6, -2, 45, 
                       89, 56, 55, 44, 55, 43, 66, 22, 33, 0xAA, 'h', 23, 
                       35, 35, 11, 1, 1, 1};
    const int length = sizeof(test_array) / sizeof(test_array[0]);
    
    printf("Array sum: %d\n", parallel_sum(test_array, length));
    return 0;
}

