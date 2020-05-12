#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

 void *thread_function(void *arg) {
  int i;
  for ( i=0; i<20; i++) {
    printf("Thread says hi!\n");
    sleep(1);
  }
  return NULL;
}

 void *thread_function2(void *arg) {
  int i;
  for ( i=0; i<20; i++) {
    printf("Thread 2 says hi!\n");
    sleep(1);
  }
  return NULL;
}

int main(void) {

  pthread_t mythread, mythread2;

  if ( pthread_create( &mythread, NULL, thread_function, NULL) ) {
    printf("error creating thread.");
    abort();
  }

  if ( pthread_create( &mythread2, NULL, thread_function2, NULL) ) {
    printf("error creating thread.");
    abort();
  }

  if ( pthread_join ( mythread, NULL ) ) {
    printf("error joining thread.");
    abort();
  }
  printf("joining thread ok.");

  if ( pthread_join ( mythread2, NULL ) ) {
    printf("error joining thread.");
    abort();
  }

  printf("joining thread 2 ok.");

  exit(0);

}
