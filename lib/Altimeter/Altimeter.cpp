#include "Altimeter.h"
#include "KalmanFilter.h"
#include <math.h>
// #include <cstdio>

#define STANDARD_SEA_LEVEL_PRESSURE 101325.0

Altimeter::Altimeter()
{
    setSealevelPressure(STANDARD_SEA_LEVEL_PRESSURE);
    m_kalmanFilter = new KalmanFilter(1.0);
    m_positionNoise = 0.8;
    m_altDamp = 0.05;
}
Altimeter::~Altimeter()
{
    delete m_kalmanFilter;
}
void Altimeter::setSealevelPressure(double pressure)
{
    m_rawPressure = pressure;
    m_seaLevelPressure = pressure;
    m_dampedAltStarted = false;
}
void Altimeter::addPressure(double pressure, double time)
{
    m_rawPressure = pressure;
    m_rawAltitude = 44330.0 * (1.0 - pow((m_rawPressure / m_seaLevelPressure), 0.190295));
    m_kalmanFilter->update(m_rawAltitude, m_positionNoise, time);
    if (m_dampedAltStarted)
    {
        m_dampedAltitude = m_dampedAltitude + m_altDamp * (m_kalmanFilter->getXAbs() - m_dampedAltitude);
    }
    else
    {
        m_dampedAltitude = m_kalmanFilter->getXAbs();
        m_dampedAltStarted = true;
    }
    // m_kalmanFilter->update(m_dampedAltitude, m_positionNoise, time);
}
void Altimeter::setAltitude(double alt)
{
    m_dampedAltitude = alt;
    m_dampedAltStarted = true;
    m_seaLevelPressure = m_rawPressure / pow(1.0 - (m_dampedAltitude / 44330.0), 5.255);
}
double Altimeter::altitude()
{
    return m_dampedAltitude;
}
double Altimeter::varioValue()
{
    return m_kalmanFilter->getXVel();
}
