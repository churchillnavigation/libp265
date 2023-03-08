/*
 * H.265 video codec.
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of libde265.
 *
 * libde265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libde265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libde265.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "libp265/threads.h"
#include <assert.h>
#include <string.h>

#if defined(_MSC_VER) || defined(__MINGW32__)
# include <malloc.h>
#elif defined(HAVE_ALLOCA_H)
# include <alloca.h>
#endif


#ifndef _WIN32
// #include <intrin.h>

#define THREAD_RESULT_TYPE  void*
#define THREAD_PARAM_TYPE   void*
#define THREAD_CALLING_CONVENTION

#include <stdio.h>

BEGIN_NAMESPACE_LIBP265

int  P265_thread_create(P265_thread* t, void *(*start_routine) (void *), void *arg) { return pthread_create(t,NULL,start_routine,arg); }
void P265_thread_join(P265_thread t) { pthread_join(t,NULL); }
void P265_thread_destroy(P265_thread* t) { }
void P265_mutex_init(P265_mutex* m) { pthread_mutex_init(m,NULL); }
void P265_mutex_destroy(P265_mutex* m) { pthread_mutex_destroy(m); }
void P265_mutex_lock(P265_mutex* m) { pthread_mutex_lock(m); }
void P265_mutex_unlock(P265_mutex* m) { pthread_mutex_unlock(m); }
void P265_cond_init(P265_cond* c) { pthread_cond_init(c,NULL); }
void P265_cond_destroy(P265_cond* c) { pthread_cond_destroy(c); }
void P265_cond_broadcast(P265_cond* c,P265_mutex* m) { pthread_cond_broadcast(c); }
void P265_cond_wait(P265_cond* c,P265_mutex* m) { pthread_cond_wait(c,m); }
void P265_cond_signal(P265_cond* c) { pthread_cond_signal(c); }
#else  // _WIN32

#define THREAD_RESULT_TYPE    DWORD
#define THREAD_CALLING_CONVENTION WINAPI
#define THREAD_PARAM_TYPE        LPVOID

int  P265_thread_create(P265_thread* t, LPTHREAD_START_ROUTINE start_routine, void *arg) {
    HANDLE handle = CreateThread(NULL, 0, start_routine, arg, 0, NULL);
    if (handle == NULL) {
        return -1;
    }
    *t = handle;
    return 0;
}
void P265_thread_join(P265_thread t) { WaitForSingleObject(t, INFINITE); }
void P265_thread_destroy(P265_thread* t) { CloseHandle(*t); *t = NULL; }
void P265_mutex_init(P265_mutex* m) { *m = CreateMutex(NULL, FALSE, NULL); }
void P265_mutex_destroy(P265_mutex* m) { CloseHandle(*m); }
void P265_mutex_lock(P265_mutex* m) { WaitForSingleObject(*m, INFINITE); }
void P265_mutex_unlock(P265_mutex* m) { ReleaseMutex(*m); }
void P265_cond_init(P265_cond* c) { win32_cond_init(c); }
void P265_cond_destroy(P265_cond* c) { win32_cond_destroy(c); }
void P265_cond_broadcast(P265_cond* c,P265_mutex* m)
{
  P265_mutex_lock(m);
  win32_cond_broadcast(c);
  P265_mutex_unlock(m);
}
void P265_cond_wait(P265_cond* c,P265_mutex* m) { win32_cond_wait(c,m); }
void P265_cond_signal(P265_cond* c) { win32_cond_signal(c); }
#endif // _WIN32




P265_progress_lock::P265_progress_lock()
{
  mProgress = 0;

  P265_mutex_init(&mutex);
  P265_cond_init(&cond);
}

P265_progress_lock::~P265_progress_lock()
{
  P265_mutex_destroy(&mutex);
  P265_cond_destroy(&cond);
}

void P265_progress_lock::wait_for_progress(int progress)
{
  if (mProgress >= progress) {
    return;
  }

  P265_mutex_lock(&mutex);
  while (mProgress < progress) {
    P265_cond_wait(&cond, &mutex);
  }
  P265_mutex_unlock(&mutex);
}

void P265_progress_lock::set_progress(int progress)
{
  P265_mutex_lock(&mutex);

  if (progress>mProgress) {
    mProgress = progress;

    P265_cond_broadcast(&cond, &mutex);
  }

  P265_mutex_unlock(&mutex);
}

void P265_progress_lock::increase_progress(int progress)
{
  P265_mutex_lock(&mutex);

  mProgress += progress;
  P265_cond_broadcast(&cond, &mutex);

  P265_mutex_unlock(&mutex);
}

int  P265_progress_lock::get_progress() const
{
  return mProgress;
}

END_NAMESPACE_LIBP265




#include "libp265/context.h"

#if 0
const char* line="--------------------------------------------------";
void printblks(const thread_pool* pool)
{
  int w = pool->tasks[0].data.task_ctb.ctx->current_sps->PicWidthInCtbsY;
  int h = pool->tasks[0].data.task_ctb.ctx->current_sps->PicHeightInCtbsY;

  printf("active threads: %d  queue len: %d\n",pool->num_threads_working,pool->num_tasks);

  char *const p = (char *)alloca(w * h * sizeof(char));
  assert(p != NULL);
  memset(p,' ',w*h);

  for (int i=0;i<pool->num_tasks;i++) {
    int b = 0; //pool->tasks[i].num_blockers;
    int x = pool->tasks[i].data.task_ctb.ctb_x;
    int y = pool->tasks[i].data.task_ctb.ctb_y;
    p[y*w+x] = b+'0';
  }

  for (int i=0;i<pool->num_threads_working;i++) {
    int x = pool->ctbx[i];
    int y = pool->ctby[i];
    p[y*w+x] = '*';
  }

  printf("+%s+\n",line+50-w);
  for (int y=0;y<h;y++)
    {
      printf("|");
      for (int x=0;x<w;x++)
        {
          printf("%c",p[x+y*w]);
        }
      printf("|\n");
    }
  printf("+%s+\n",line+50-w);
}
#endif

BEGIN_NAMESPACE_LIBP265

static THREAD_RESULT_TYPE THREAD_CALLING_CONVENTION worker_thread(THREAD_PARAM_TYPE pool_ptr)
{
  thread_pool* pool = (thread_pool*)pool_ptr;


  P265_mutex_lock(&pool->mutex);

  while(true) {

    // wait until we can pick a task or until the pool has been stopped

    for (;;) {
      // end waiting if thread-pool has been stopped or we have a task to execute

      if (pool->stopped || pool->tasks.size()>0) {
        break;
      }

      //printf("going idle\n");
      P265_cond_wait(&pool->cond_var, &pool->mutex);
    }

    // if the pool was shut down, end the execution

    if (pool->stopped) {
      P265_mutex_unlock(&pool->mutex);
      return (THREAD_RESULT_TYPE)0;
    }


    // get a task

    thread_task* task = pool->tasks.front();
    pool->tasks.pop_front();

    pool->num_threads_working++;

    //printblks(pool);

    P265_mutex_unlock(&pool->mutex);


    // execute the task

    task->work();

    // end processing and check if this was the last task to be processed

    P265_mutex_lock(&pool->mutex);

    pool->num_threads_working--;
  }
  P265_mutex_unlock(&pool->mutex);

  return (THREAD_RESULT_TYPE)0;
}


P265_error start_thread_pool(thread_pool* pool, int num_threads)
{
  P265_error err = P265_OK;

  // limit number of threads to maximum

  if (num_threads > MAX_THREADS) {
    num_threads = MAX_THREADS;
    err = P265_WARNING_NUMBER_OF_THREADS_LIMITED_TO_MAXIMUM;
  }

  pool->num_threads = 0; // will be increased below

  P265_mutex_init(&pool->mutex);
  P265_cond_init(&pool->cond_var);

  P265_mutex_lock(&pool->mutex);
  pool->num_threads_working = 0;
  pool->stopped = false;
  P265_mutex_unlock(&pool->mutex);

  // start worker threads

  for (int i=0; i<num_threads; i++) {
    int ret = P265_thread_create(&pool->thread[i], worker_thread, pool);
    if (ret != 0) {
      // cerr << "pthread_create() failed: " << ret << endl;
      return P265_ERROR_CANNOT_START_THREADPOOL;
    }

    pool->num_threads++;
  }

  return err;
}


void stop_thread_pool(thread_pool* pool)
{
  P265_mutex_lock(&pool->mutex);
  pool->stopped = true;
  P265_mutex_unlock(&pool->mutex);

  P265_cond_broadcast(&pool->cond_var, &pool->mutex);

  for (int i=0;i<pool->num_threads;i++) {
    P265_thread_join(pool->thread[i]);
    P265_thread_destroy(&pool->thread[i]);
  }

  P265_mutex_destroy(&pool->mutex);
  P265_cond_destroy(&pool->cond_var);
}


void   add_task(thread_pool* pool, thread_task* task)
{
  P265_mutex_lock(&pool->mutex);
  if (!pool->stopped) {

    pool->tasks.push_back(task);

    // wake up one thread

    P265_cond_signal(&pool->cond_var);
  }
  P265_mutex_unlock(&pool->mutex);
}

END_NAMESPACE_LIBP265
