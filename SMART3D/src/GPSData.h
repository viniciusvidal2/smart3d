#pragma once

class GPSData {
private:
	double latitude = -1;
	double longitude = -1;
	double altitude = -1;
public:
	GPSData() {};
	~GPSData() {};

	void setLongitude(double longitude) { this->longitude = longitude; };
	double getLongitude() const { return longitude; };

	void setLatitude(double latitude) { this->latitude = latitude; };
	double getLatitude() const { return latitude; };

	void setAltitude(double altitude) { this->altitude = altitude; };
	double getAltitude() const { return altitude; };

  /*
  Compute the distance bewtween two GPSData, it uses the latitude, longitude and altitude
  */
  double getDistanceBetweenGPSData(GPSData* gpsData);
};