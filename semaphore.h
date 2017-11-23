#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <mutex>
#include <condition_variable>

typedef std::unique_lock<std::mutex> Uniqlo;
class Semaphore{
	private:
	int _pval;
	std::mutex _sem_lk;
	Uniqlo *_ulk;
	std::condition_variable _cv;
	public: 
	Semaphore(int);
	~Semaphore();
	void P();
	void V();
};


Semaphore::Semaphore(int pval):_pval(pval)
{
	_ulk = new Uniqlo(_sem_lk);
	_sem_lk.unlock();
	
}
void Semaphore::P()
{
	_sem_lk.lock();	
	while(_pval<=0){
		_cv.wait(*_ulk);
	}	
	_pval-=1;
	_sem_lk.unlock();
	
}


void Semaphore::V()
{
	_sem_lk.lock();
	_pval+=1;
	_sem_lk.unlock();
	_cv.notify_one();
	
}

Semaphore::~Semaphore(){
}

/*
Semaphore *test_sem;
Semaphore *donesem;
static unsigned long semtest_current;
void sem_test_thread( int num){

	std::this_thread::yield();
	test_sem->P();
	semtest_current = num;
	fprintf(stderr, "Thread %d: ", num);
	for(int i = 0; i < 10; i++){
		fprintf(stderr, ".");
		printf("%d", num);
		std::this_thread::yield();
		ASSERT(semtest_current == num);
	}
	printf("\n");
	donesem->V();
}

void sem_test(){
	
	
	printf("Starting sem test...\n");
	for(int i = 0; i < 8; i++){
		printf(".");
		test_sem = new Semaphore(2);
		ERRORIF(test_sem == NULL, "create sem");
		donesem = new Semaphore(0);
		ERRORIF(test_sem == NULL, "create sem");
		if(i!=7){
			delete test_sem;
			delete donesem;
		}
	}
	printf("start creating threads...");
	test_sem->P();
	test_sem->P();
	
	printf("OK\n");
	std::vector<std::thread> trds;

	for(int i = 0; i < 32; i++){
		fprintf(stderr,".");
		trds.emplace_back(sem_test_thread, i);
	}
	fprintf(stderr,"start running threads...\n");
	for (int i=0; i<32; i++) {
		fprintf(stderr, ".");
		test_sem->V();
		donesem->P();
	}
	
	printf("OK\n");
	delete test_sem;
	delete donesem;
	printf("\n");
	for(auto& i: trds){
		i.join();
	}

}

*/
#endif