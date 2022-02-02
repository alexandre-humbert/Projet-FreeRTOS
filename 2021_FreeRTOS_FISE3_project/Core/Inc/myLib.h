/*
 * myLib.h
 *
 *  Created on: Feb 1, 2021
 *      Author: florentgoutailler
 */

#ifndef INC_MYLIB_H_
#define INC_MYLIB_H_

void I2C_Scan(I2C_HandleTypeDef*,char*);
void Init_MPU9250(I2C_HandleTypeDef*);
void Init_BMP280(I2C_HandleTypeDef*);
void Measure_T(I2C_HandleTypeDef*,double*);
void Measure_T_BMP280(I2C_HandleTypeDef*,double*,long signed int*);
uint8_t Measure_P(I2C_HandleTypeDef*,double*,long signed int*);
void Measure_A(I2C_HandleTypeDef*,double*);
void Measure_G(I2C_HandleTypeDef*,double*,double*);
void Measure_M(I2C_HandleTypeDef*,double*,double*,double*);

#endif /* INC_MYLIB_H_ */
