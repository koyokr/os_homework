#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TABLE_SIZE (199)
#define WORD_MAXSIZE (16)
#define N_THREAD (4)

typedef struct {
    const char *key;
} element;

typedef struct _List {
    element item;
    struct _List *link;
} List;

typedef struct {
    int count;
    List *value;
} ListRoot;

typedef struct {
    char bytes[WORD_MAXSIZE];
} Word;

typedef struct {
    Word *beg;
    Word *end;
} Range;

pthread_mutex_t MUTEX_LOCK; // Critical-section 객체
ListRoot HT[TABLE_SIZE]; // 해시 테이블

void unix_error(char const *s)
{
	perror(s);
	exit(1);
}

void hash_chain_count_print()
{
    printf("\n====================\n");
    for (int i = 0; i < TABLE_SIZE; i++)
        printf("[%d]->%d\n", i, HT[i].count);
    printf("====================\n");
}

int hash_function(const char *key)
{
    // horner's method
    int g = 31;
    int hash_index = 0;

    while (*key)
        hash_index = g * hash_index + *key++;
    // 제산법
    hash_index %= TABLE_SIZE;
    if (hash_index < 0)
        hash_index += TABLE_SIZE;
    return hash_index;
}

void hash_chain_add(const element item)
{
    int hash_value = hash_function(item.key);
    List *node = HT[hash_value].value;
    List *node_before = NULL;
    List *ptr;

    for (; node; node_before = node, node = node->link) {
        if (node->item.key == item.key) {
            // fprintf(stderr, "이미 탐색키가 저장되어 있음\n");
            return;
        }
    }
    ptr = malloc(sizeof(ptr));
    if (!ptr)
        return;

    ptr->item = item;
    ptr->link = NULL;
    if (node_before)
        node_before->link = ptr;
    else
        HT[hash_value].value = ptr;
    HT[hash_value].count++; // 해시 테이블의 item 갯수 증가
}

void rand_word(Word *word)
{
    int wordlen = rand() % (sizeof(Word) - 1) + 1;
    int i;
    for (i = 0; i < wordlen; i++)
        word->bytes[i] = rand() % 26 + 97;
    word->bytes[i] = 0;
}

Word *create_random_words(const int n)
{
    Word *words = malloc(n * sizeof(Word));
    if (!words)
        return words;

    for (int i = 0; i < n; i++)
        rand_word(words + i);
    return words;
}

void free_words(Word *words)
{
    free(words);
}

int input_n()
{
    int n;
    while (1)
    {
        printf("n: ");
        scanf("%d", &n);
        if (n >= 1000)
            break;
    }
    return n;
}

void *work(void *param)
{
    Range *r = param;
    for (Word *p = r->beg; p != r->end; p++) {
        element e = { p->bytes };
        pthread_mutex_lock(&MUTEX_LOCK); // Critical-section 시작
        hash_chain_add(e);
        pthread_mutex_unlock(&MUTEX_LOCK); // Critical-section 종료
    }
    return NULL;
}

int main()
{
    int n = input_n(); // 스트링의 개수 입력 (1,000개 이상)
    Word *words = create_random_words(n); // 랜덤 단어 목록 생성
    if (!words)
        unix_error("create_words");

    Range rs[N_THREAD];
    pthread_t ts[N_THREAD];

    for (int i = 0; i < N_THREAD; i++)
        rs[i] = (Range){ words + (n * i / N_THREAD), words + (n * (i + 1) / N_THREAD) };

    pthread_mutex_init(&MUTEX_LOCK, NULL); // pthread_mutex 객체 초기화
    for (int i = 0; i < N_THREAD; i++) {
        int id = pthread_create(&ts[i], NULL, work, &rs[i]);
        if (id < 0)
            unix_error("pthread_create");
    }
    for (int i = 0; i < N_THREAD; i++)
        pthread_join(ts[i], NULL); // 스레드 핸들 제거
    hash_chain_count_print();

    pause();

    pthread_mutex_destroy(&MUTEX_LOCK); // pthread_mutex 객체 제거
    free_words(words); // 단어 목록 메모리 회수
    return 0;
}
