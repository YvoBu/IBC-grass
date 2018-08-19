#ifndef SRC_RANDOMGENERATOR_H_
#define SRC_RANDOMGENERATOR_H_

#include <random>
#include <pthread.h>

class RandomGenerator
{

public:

    unsigned int getrng();
	int getUniformInt(int thru);
	double get01();
	double getGaussian(double mean, double sd);

    RandomGenerator() : rng(std::random_device()()) {
        pthread_spin_init(&ui_lock, PTHREAD_PROCESS_PRIVATE);
        pthread_spin_init(&get01_lock, PTHREAD_PROCESS_PRIVATE);
        pthread_spin_init(&getG_lock, PTHREAD_PROCESS_PRIVATE);
        pthread_spin_init(&getrng_lock, PTHREAD_PROCESS_PRIVATE);
    }

    inline std::mt19937 getRNG() { return rng; }
    pthread_spinlock_t ui_lock;
    pthread_spinlock_t get01_lock;
    pthread_spinlock_t getG_lock;
    pthread_spinlock_t getrng_lock;
private:
    std::mt19937 rng;
};

#endif /* SRC_RANDOMGENERATOR_H_ */

