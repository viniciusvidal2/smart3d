#include "GPSData.h"

#include "Utils.h"

double GPSData::getDistanceBetweenGPSData(GPSData * gpsData)
{
    return Utils::getDistanceBetweenGeographicCoordinate(this->latitude, this->longitude, this->altitude, gpsData->latitude, gpsData->longitude, gpsData->altitude);
}
