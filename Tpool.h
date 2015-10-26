#ifndef TPOOL_H
#define TPOOL_H
#include"EZweb.h"
#include <pthread.h>
#include <queue>
#include <vector>
#include <set>

using std::set;
using std::queue;
using std::vector;

class Tpool;
void* thread_ctrl(void* arg);
class J_{
	friend void* thread_ctrl(void* arg);
	friend class Tpool;
	J_(void*(*p)(void*), void* a):process(p), arg(a){};
	void *(*process)(void*);
	void *arg;
};

class Tpool
{
	friend void* thread_ctrl(void* arg);
	public:
		Tpool(size_t thread_n):max_thread_num(thread_n), thread_ids(thread_n)
		{
			pthread_mutex_init(&jobQueue_lock, NULL);
			pthread_mutex_init(&added_fds_lock, NULL);
			pthread_cond_init(&jobQueue_cond, NULL);
			shutdown = false;
		}

		void start()
		{
			for(auto viter = thread_ids.begin(); viter != thread_ids.end(); ++ viter)
			{
				pthread_create(&(*viter), NULL, thread_ctrl, this);
				pthread_detach(*viter);
			}
		}

		void add(void *(*process)(void*), void* arg)
		{
			POD_arg *arg_real = new POD_arg(*reinterpret_cast<POD_arg*>(arg));
			pthread_mutex_lock(&added_fds_lock);
			if(added_fds.count(arg_real->sock_fd))
			{
				pthread_mutex_unlock(&added_fds_lock);
				return;
			}
			added_fds.insert(arg_real->sock_fd);
			pthread_mutex_unlock(&added_fds_lock);

			J_ *tmp = new J_(process, arg_real);
			pthread_mutex_lock(&jobQueue_lock);
			jobQueue.push(tmp);
			pthread_cond_signal(&jobQueue_cond);
			pthread_mutex_unlock(&jobQueue_lock);
		}

		~Tpool()
		{
			if(shutdown)
				return;
			for(auto n : thread_ids)
			{
				pthread_cancel(n);
			}

			while(!jobQueue.empty())
			{
				delete jobQueue.front();
				jobQueue.pop();
			}

			pthread_mutex_destroy(&jobQueue_lock);
			pthread_cond_destroy(&jobQueue_cond);
			
		}

	private:
		size_t max_thread_num;
		queue<J_*> jobQueue;
		pthread_mutex_t jobQueue_lock;
		pthread_mutex_t added_fds_lock;
		pthread_cond_t jobQueue_cond;
		vector<pthread_t> thread_ids;
		set<int> added_fds;
		bool shutdown;
};


#endif

