#include <random>

#include "RandomGenerator.h"

int RandomGenerator::getUniformInt(int thru)
{
    int retval;

    pthread_spin_lock(&ui_lock);
    retval = floor(get01() * thru);
    pthread_spin_unlock(&ui_lock);

    return retval;
}

double RandomGenerator::get01()
{
    double retval;

    pthread_spin_lock(&get01_lock);
	std::uniform_real_distribution<double> dist(0, 1);
    retval = dist(rng);
    pthread_spin_unlock(&get01_lock);

    return retval;
}

double RandomGenerator::getGaussian(double mean, double sd)
{
    double retval;

    pthread_spin_lock(&getG_lock);
	std::normal_distribution<double> dist(mean, sd);
    retval = dist(rng);
    pthread_spin_unlock(&getG_lock);

    return retval;
}
