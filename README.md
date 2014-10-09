#LSM303DLHC sensor for gesture recognition - exploration of the LSM303DLHC sensor.

This sketch makes the use of the compass and accelerometer LSM303DLHC
breakout board from adafruit. It also uses the libraries for this breakout
board.  It recognizes gestures in a simplified maner by checking if certain
thresholds is breached.

It also uses an adapted smoothing method, smoothing all 6 axis from the
LSM303 sensor. It puts several readings in an array and calculates the
average.
