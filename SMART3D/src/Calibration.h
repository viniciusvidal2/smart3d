#pragma once
#include <sstream>
#include <iomanip>
#include <string>

class Calibration
{
private:
	double scaleFactor = 1.0;
	const std::string measureUnit;

	//GPS
	bool calibratedUsingGPSData = false;
	double minAltitude = -1;
	double maxAltitude = -1;

public:
	Calibration() {};
	Calibration(double scaleFactor, const std::string& measureUnit) : measureUnit(measureUnit)
	{
		if (scaleFactor > 0.0)
		{
			this->scaleFactor = scaleFactor;
		}
	};
	~Calibration() {};

	//Return the distance with the right scale
	double getCalibratedDistance(double dist) const { return dist * scaleFactor; };
	//Return a text with the right distance and the measure unit
	std::string getCalibratedText(double dist) const
	{
		std::stringstream s;
		s << std::fixed << std::setprecision(3) << dist * scaleFactor << measureUnit;
		return s.str();
	};
	//Return a text with the right distance and the measure unit
	std::string getCalibratedText(double dist, double deltaX, double deltaY, double deltaZ) const
	{
		std::stringstream s;
		s << std::fixed << std::setprecision(3) << dist * scaleFactor << measureUnit
			<< " (" << deltaX * scaleFactor
			<< ", " << deltaY * scaleFactor
			<< ", " << deltaZ * scaleFactor << ")";
		return s.str();
	};
	//Return the scale factor
	double getScaleFactor() const { return scaleFactor; };
	//Return the measure unit
	std::string getMeasureUnit() const { return measureUnit; };

	//Set if the calibration was done by GPSData
	void setCalibratedUsingGPSData(bool calibratedUsingGPSData) { this->calibratedUsingGPSData = calibratedUsingGPSData; };
	//Get if the calibration was done by GPSData
	bool getCalibratedUsingGPSData() const { return calibratedUsingGPSData; };
	//Set the minimum altitude
	void setMinimumAltitude(double minAltitude) { this->minAltitude = minAltitude; };
	//Return minimum altitude
	double getMinimumAltitude() const { return minAltitude; };
	//Set the maximum altitude
	void setMaximumAltitude(double maxAltitude) { this->maxAltitude = maxAltitude; };
	//Return minimum altitude
	double getMaximumAltitude() const { return maxAltitude; };
};