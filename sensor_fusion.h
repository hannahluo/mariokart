//=============================================================================================
// Madgwick's implementation of Mayhony's AHRS algorithm.
// See: http://www.x-io.co.uk/open-source-imu-and-ahrs-algorithms/
//
// Date			Author			Notes
// 29/09/2011	SOH Madgwick    Initial release
// 02/10/2011	SOH Madgwick	Optimised for reduced CPU load
//
// Converted from C++ to C for ECE by Bernie Roehl, March 2017
//
//=============================================================================================
#ifndef sensor_fusion_h
#define sensor_fusion_h
#include <math.h>

void sensor_fusion_init(void);
void sensor_fusion_begin(float sampleFrequency);
void sensor_fusion_update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void sensor_fusion_updateIMU(float gx, float gy, float gz, float ax, float ay, float az);
float sensor_fusion_getRoll(void);
float sensor_fusion_getPitch(void);
float sensor_fusion_getYaw(void);
float sensor_fusion_getRollRadians(void);
float sensor_fusion_getPitchRadians(void);
float sensor_fusion_getYawRadians(void);
#endif
