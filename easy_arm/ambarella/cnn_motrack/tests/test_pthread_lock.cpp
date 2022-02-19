// #include<pthread.h>
// #include<unistd.h>
// #include<stdio.h>
// #include<stdlib.h>
 
// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; /*初始化互斥锁*/
// pthread_cond_t cond = PTHREAD_COND_INITIALIZER; // init cond
 
// void *thread1(void*);
// void *thread2(void*);
 
// struct GlobalParam
// {
//     int i;
//     int run_flag;
// };
 
// void *thread1(void *junk){
//     // for(i = 1;i<= 9; i++){
//     //     pthread_mutex_lock(&mutex); //互斥锁
//     //     printf("call thread1 \n");
//     //     if(i%3 == 0)
//     //     	{
//     //             pthread_cond_signal(&cond); //send sianal to t_b
//     //             printf("thread1:******i=%d\n", i);
//     //     	}
//     //     else
//     //         printf("thread1: %d\n",i);
//     //     pthread_mutex_unlock(&mutex);
 
// 	// 	printf("thread1: sleep i=%d\n", i);
//     //     sleep(1);
// 	// 	printf("thread1: sleep i=%d******end\n", i);
//     // }
//     while(1)
//     {
//         GlobalParam *global_param = (GlobalParam *)junk;
//         printf("call thread1 \n");
//         pthread_mutex_lock(&mutex); //互斥锁
//         global_param->i = global_param->i + 1;
//         global_param->run_flag = 1;
//         if(global_param->run_flag == 1)
//         {
//             pthread_cond_signal(&cond); //send sianal to t_b
//         } 
//         pthread_mutex_unlock(&mutex);
//         // pthread_cond_signal(&cond);
//         printf("I'm writing, and i = %d\n", global_param->i);
//         sleep(5);
//     }
// }
 
// void *thread2(void* junk){
//     while(1)
//     {
//         GlobalParam *global_param = (GlobalParam *)junk;
//         pthread_mutex_lock(&mutex);
//         printf("call thread2 \n");
//         if(global_param->run_flag == 1)
//             pthread_cond_wait(&cond, &mutex); //wait
//         printf("I'm reading, and i= %d\n", global_param->i);
//         global_param->run_flag = 0;
//         pthread_mutex_unlock(&mutex);
//         sleep(1);
 
// 		// printf("thread2: sleep i=%d\n", i);
//         // sleep(1);
// 		// printf("thread2: sleep i=%d******end\n", i);		
//     }
// }

// int main(void){
//     pthread_t t_a;
//     pthread_t t_b;//two thread

//     GlobalParam global_param;
//     global_param.i = 1;
 
//     pthread_create(&t_b, NULL, thread1, &global_param);//Create thread
//     pthread_create(&t_a, NULL, thread2, &global_param);
    
//     printf("t_a:0x%x, t_b:0x%x:", t_a, t_b);
//     pthread_join(t_a, NULL);
//     pthread_join(t_b, NULL);//wait a_b thread end
//     pthread_mutex_destroy(&mutex);
//     pthread_cond_destroy(&cond);
//     exit(0);
//     return 0;
// }



#include <iostream>
#include <pthread.h>
#include <unistd.h>
int Money = 0;
int run_flag = 0;
 
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
 
void* readThread(void* argv)
{
    while (1)
    {
        if (run_flag < 1)
        {
            // printf("Waiting.........................\n");
            continue;
        }
        else
        {
            pthread_rwlock_rdlock(&rwlock);
            printf("tid is: %d, have money:%ld \n", pthread_self(), Money);
            sleep(1);
            pthread_rwlock_unlock(&rwlock);
            run_flag = 0;
        }
    }
    return NULL;
}
 
void* writeThread(void* argv)
{
    while (1)
    {
        printf("emmmmm, I am in write. Thread:%ld \n",pthread_self());
        sleep(10);
        pthread_rwlock_wrlock(&rwlock);
        printf("I am writing Money\n");
        Money+=1;
        sleep(2);
        run_flag = 1;
        pthread_rwlock_unlock(&rwlock);
    }
    return NULL;
}
 
int main() {
    pthread_t pid[2];
    // for (int i = 0; i < 4; ++i) {
        
    // }
    pthread_create(&pid[0],NULL,readThread,NULL);
    pthread_create(&pid[1],NULL,writeThread,NULL);
 
    // for (int j = 0; j <5 ; ++j) {
    pthread_join(pid[0],NULL);
    pthread_join(pid[1],NULL);
    // }
 
    return 0;
}