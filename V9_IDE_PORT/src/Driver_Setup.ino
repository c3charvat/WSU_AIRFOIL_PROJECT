// Driver setup config
#include <TMCStepper.h>
#include <TMCStepper_UTILITY.h>
void DRIVER_SETUP(){
driverX.beginSerial(115200); // X driver Coms begin
Serial.println("Driver X Enabled\n");
driverX.begin();
driverX.rms_current(850); // mA
driverX.microsteps(64);
driverX.pwm_ofs_auto ();

driverY.beginSerial(115200);
Serial.println("Driver Y Enabled\n");
driverY.begin();
driverY.rms_current(850); // mA
driverY.microsteps(64);
driverY.pwm_ofs_auto ();

driverZ.beginSerial(115200);
Serial.println("Driver Z Enabled\n");
driverZ.begin();
driverZ.rms_current(850); // mA
driverZ.microsteps(64);
driverZ.pwm_ofs_auto ();

driverE0.beginSerial(115200);
Serial.println("Driver X Enabled\n");
driverE0.begin();
driverE0.rms_current(850); // mA
driverE0.microsteps(64);
driverE0.pwm_ofs_auto ();

driverE1.beginSerial(115200);
Serial.println("Driver X Enabled\n");
driverE1.begin();
driverE1.rms_current(850); // mA
driverE1.microsteps(64);
driverE1.pwm_ofs_auto ();
}

