#include <random>

#include "RandomGenerator.h"

int RandomGenerator::getUniformInt(int thru)
{
    int retval;
    retval = floor(get01() * thru);
    return retval;
}

double RandomGenerator::get01()
{
    double retval;

    pthread_spin_lock(&getrng_lock);
	std::uniform_real_distribution<double> dist(0, 1);
    retval = dist(rng);
    pthread_spin_unlock(&getrng_lock);

    return retval;
}

double RandomGenerator::getGaussian(double mean, double sd)
{
    double retval;

    pthread_spin_lock(&getrng_lock);
	std::normal_distribution<double> dist(mean, sd);
    retval = dist(rng);
    pthread_spin_unlock(&getrng_lock);

    return retval;
}

unsigned int RandomGenerator::getrng() {
    unsigned int retval;

    pthread_spin_lock(&getrng_lock);
    retval = rng();
    pthread_spin_unlock(&getrng_lock);

    return retval;
}
