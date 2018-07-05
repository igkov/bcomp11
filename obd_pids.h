#ifndef __OBD_PIDS_H__
#define __OBD_PIDS_H__

/* Details from http://en.wikipedia.org/wiki/OBD-II_PIDs */
#define PID_0_20            0x0100    // PID 0x00 - 0x20 Supported
#define STATUS_DTC          0x0101    // Status since DTC Cleared
#define FREEZE_DTC          0x0102    // Freeze Diagnostic Trouble Code
#define FUEL_SYS_STATUS     0x0103    // Fuel System Status
#define ENGINE_LOAD         0x0104    // Calculated Engine Load
#define ENGINE_COOLANT_TEMP 0x0105    // Engine Coolant Temperature
#define ST_FUEL_TRIM_1      0x0106    // Short Term Fuel % Trim - Bank 1
#define LT_FUEL_TRIM_1      0x0107    // Long Term Fuel % Trim - Bank 1
#define ST_FUEL_TRIM_2      0x0108    // Short Term Fuel % Trim - Bank 2
#define LT_FUEL_TRIM_2      0x0109    // Long Term Fuel % Trim - Bank 2
#define FUEL_PRESSURE       0x010A    // Fuel Pressure
#define INTAKE_PRESSURE     0x010B    // Intake Manifold Absolute Pressure
#define ENGINE_RPM          0x010C    // Engine RPM
#define VEHICLE_SPEED       0x010D    // Vehicle Speed
#define TIMING_ADVANCE      0x010E    // Timing Advance
#define INTAKE_TEMP         0x010F    // Intake Air Temperature
#define MAF_SENSOR          0x0110    // MAF Sensor Air Flow Rate
#define THROTTLE            0x0111    // Throttle Position
#define COMMANDED_SEC_AIR   0x0112    // Commanded Secondary Air Status
#define O2_SENS_PRES        0x0113    // Detected O2 Sensors
#define O2_B1S1_VOLTAGE     0x0114    // O2 Sensor Voltage - Bank 1 Sensor 1
#define O2_B1S2_VOLTAGE     0x0115    // O2 Sensor Voltage - Bank 1 Sensor 2
#define O2_B1S3_VOLTAGE     0x0116    // O2 Sensor Voltage - Bank 1 Sensor 3
#define O2_B1S4_VOLTAGE     0x0117    // O2 Sensor Voltage - Bank 1 Sensor 4
#define O2_B2S1_VOLTAGE     0x0118    // O2 Sensor Voltage - Bank 2 Sensor 1
#define O2_B2S2_VOLTAGE     0x0119    // O2 Sensor Voltage - Bank 2 Sensor 2
#define O2_B2S3_VOLTAGE     0x011A    // O2 Sensor Voltage - Bank 2 Sensor 3
#define O2_B2S4_VOLTAGE     0x011B    // O2 Sensor Voltage - Bank 2 Sensor 4
#define OBDII_STANDARDS     0x011C    // Supported OBDII Standards
#define O2_SENS_PRES_ALT    0x011D    // Detected O2 Sensors - Alternate Grouping
#define AUX_IN_STATUS       0x011E    // Auxiliary Input Status
#define ENGINE_RUNTIME      0x011F    // Run Time Since Engine Started
#define PID_21_40           0x0120    // PID 0x21 - 0x40 Supported
#define DIST_TRAVELED_MIL   0x0121    // Distance Traveled with MIL On
#define FUEL_RAIL_PRESSURE  0x0122    // Fuel Rail Pressure Relative to Manifold
#define FUEL_RAIL_PRES_ALT  0x0123    // MPI/Diesel Fuel Rail Pressure
#define O2S1_WR_LAMBDA_V    0x0124    // O2 Sensor 1 Equivalence Ratio Voltage
#define O2S2_WR_LAMBDA_V    0x0125    // O2 Sensor 2 Equivalence Ratio Voltage
#define O2S3_WR_LAMBDA_V    0x0126    // O2 Sensor 3 Equivalence Ratio Voltage
#define O2S4_WR_LAMBDA_V    0x0127    // O2 Sensor 4 Equivalence Ratio Voltage
#define O2S5_WR_LAMBDA_V    0x0128    // O2 Sensor 5 Equivalence Ratio Voltage
#define O2S6_WR_LAMBDA_V    0x0129    // O2 Sensor 6 Equivalence Ratio Voltage
#define O2S7_WR_LAMBDA_V    0x012A    // O2 Sensor 7 Equivalence Ratio Voltage
#define O2S8_WR_LAMBDA_V    0x012B    // O2 Sensor 8 Equivalence Ratio Voltage
#define COMMANDED_EGR       0x012C    // Commanded EGR
#define EGR_ERROR           0x012D    // EGR Error
#define COMMANDED_EVAP_P    0x012E    // Commanded Evaporative Purge
#define FUEL_LEVEL          0x012F    // Fuel Level Input
#define WARMUPS_SINCE_CLR   0x0130    // Number of Warmups since DTC Cleared
#define DIST_SINCE_CLR      0x0131    // Distance Traveled Since DTC Cleared
#define EVAP_PRESSURE       0x0132    // Evap. System Vapor Pressure
#define BAROMETRIC_PRESSURE 0x0133    // Barometric Pressure
#define O2S1_WR_LAMBDA_I    0x0134    // O2 Sensor 1 Equivalence Ratio Current
#define O2S2_WR_LAMBDA_I    0x0135    // O2 Sensor 2 Equivalence Ratio Current
#define O2S3_WR_LAMBDA_I    0x0136    // O2 Sensor 3 Equivalence Ratio Current
#define O2S4_WR_LAMBDA_I    0x0137    // O2 Sensor 4 Equivalence Ratio Current
#define O2S5_WR_LAMBDA_I    0x0138    // O2 Sensor 5 Equivalence Ratio Current
#define O2S6_WR_LAMBDA_I    0x0139    // O2 Sensor 6 Equivalence Ratio Current
#define O2S7_WR_LAMBDA_I    0x013A    // O2 Sensor 7 Equivalence Ratio Current
#define O2S8_WR_LAMBDA_I    0x013B    // O2 Sensor 8 Equivalence Ratio Current
#define CAT_TEMP_B1S1       0x013C    // Catalyst Temperature Bank 1 Sensor 1
#define CAT_TEMP_B1S2       0x013E    // Catalyst Temperature Bank 1 Sensor 2
#define CAT_TEMP_B2S1       0x013D    // Catalyst Temperature Bank 2 Sensor 1
#define CAT_TEMP_B2S2       0x013F    // Catalyst Temperature Bank 2 Sensor 2
#define PID_41_60           0x0140    // PID 0x41 - 0x60 Supported
#define MONITOR_STATUS      0x0141    // Monitor Status This Drive Cycle
#define ECU_VOLTAGE         0x0142    // Control Module Voltage
#define ABSOLUTE_LOAD       0x0143    // Absolute Load Value
#define COMMANDED_EQUIV_R   0x0144    // Commanded Equivalence Ratio
#define REL_THROTTLE_POS    0x0145    // Relative Throttle Position
#define AMB_AIR_TEMP        0x0146    // Ambient Air Temperature
#define ABS_THROTTLE_POS_B  0x0147    // Absolute Throttle Position B
#define ABS_THROTTLE_POS_C  0x0148    // Absolute Throttle Position C
#define ACCEL_POS_D         0x0149    // Accelerator Pedal Position D
#define ACCEL_POS_E         0x014A    // Accelerator Pedal Position E
#define ACCEL_POS_F         0x014B    // Accelerator Pedal Position F
#define COMMANDED_THROTTLE  0x014C    // Commanded Throttle Actuator
#define TIME_RUN_WITH_MIL   0x014D    // Time Run with MIL on
#define TIME_SINCE_CLR      0x014E    // Time Since DTC Cleared
#define MAX_R_O2_VI_PRES    0x014F    // Maximum Value - Equivalence ratio, O2 Voltage, O2 Current, Intake Manifold Pressure
#define MAX_AIRFLOW_MAF     0x0150    // Maximum MAF Airflow Value
#define FUEL_TYPE           0x0151    // Fuel Type
#define ETHANOL_PERCENT     0x0152    // Ethanol fuel %
#define ABS_EVAP_SYS_PRES   0x0153    // absolute Evap. System Vapor Pressure
#define EVAP_SYS_PRES       0x0154    // Evap. System Vapor Pressure
#define ST_O2_TRIM_B1B3     0x0155    // Short Term Secondary O2 Sensor Trim - Bank 1 and 3
#define LT_O2_TRIM_B1B3     0x0156    // Long Term Secondary O2 Sensor Trim - Bank 1 and 3
#define ST_02_TRIM_B2B4     0x0157    // Short Term Secondary O2 Sensor Trim - Bank 2 and 4
#define LT_O2_TRIM_B2B4     0x0158    // Long Term Secondary O2 Sensor Trim - Bank 2 and 4
#define ABS_FUEL_RAIL_PRES  0x0159    // Absolute Fuel Rail Pressure
#define REL_ACCEL_POS       0x015A    // Relative Accelerator Pedal Position
#define HYBRID_BATT_PCT     0x015B    // Hybrid Battery Pack Charge Percent
#define ENGINE_OIL_TEMP     0x015C    // Engine Oil Temperature
#define FUEL_TIMING         0x015D    // Fuel Injection Timing
#define FUEL_RATE           0x015E    // Engine Fuel Rate
#define EMISSIONS_STANDARD  0x015F    // Emmissions Requirements
#define DEMANDED_TORQUE     0x0161    // Driver's Demanded Torque - Percent
#define ACTUAL_TORQUE       0x0162    // Actual Engine Torque - Percent
#define REFERENCE_TORQUE    0x0163    // Engine Reference Torque
#define ENGINE_PCT_TORQUE   0x0164    // Engine Percent Torque
#define AUX_IO_SUPPORTED    0x0165    // Auxiliary Input/Output Supported

#define GET_VIN             0x0902    // Get VIN
 
#define PID_REQUEST         0x7DF     // Standart
#define PID_REQUEST_ENGINE  0x7E0     // Engine address
#define PID_REPLY           0x7E8     // Engine reply address

// Vendor specific:

#define PAJERO_AT_INFO      0x2102    // Automatic transmission status
#define PAJERO_ODO_INFO     0x2103    // Distation status

#define NISSAN_AT_INFO      0x2101    // Automatic transmission status

#define PID_REQUEST_AT      0x7E1     // Automatic transmission address
#define PID_REPLY_AT        0x7E9     // Automatic transmission reply address

#endif
