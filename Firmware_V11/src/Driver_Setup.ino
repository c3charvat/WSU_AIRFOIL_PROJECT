// Driver setup config
#include <TMCStepper.h>
#include <TMCStepper_UTILITY.h>
void DRIVER_SETUP()
{
driverX.beginSerial(115200); // X driver Coms begin
Serial.println("Driver X Enabled\n");
driverX.begin();
driverX.rms_current(1100); // mA
driverX.microsteps(64);
//driverX.en_spreadCycle(0); // Page 44 use stealth chop
driverX.pwm_ofs_auto ();
driverX.pwm_autograd(1);
driverX.pwm_autoscale(1);

driverX2.beginSerial(115200);
Serial.println("Driver X2 Enabled\n");
driverX2.begin();
driverX2.rms_current(1100); // mA
driverX2.microsteps(64);
driverX2.pwm_ofs_auto ();
driverX2.pwm_autograd(1);
driverX2.pwm_autoscale(1);

driverY0.beginSerial(115200);
Serial.println("Driver Y0 Enabled\n");
driverY0.begin();
driverY0.rms_current(1100); // mA
driverY0.microsteps(64);
//driverY0.en_spreadCycle(0);
driverY0.pwm_ofs_auto ();
driverY0.pwm_autoscale(1);
driverY0.pwm_autograd(1);

driverY1.beginSerial(115200);
Serial.println("Driver Y12 Enabled\n");
driverY1.begin();
driverY1.rms_current(900); // mA
driverY1.microsteps(64);
driverY1.pwm_ofs_auto ();

driverY2.beginSerial(115200);
Serial.println("Driver Y12 Enabled\n");
driverY2.begin();
driverY2.rms_current(900); // mA
driverY2.microsteps(64);
driverY2.pwm_ofs_auto ();

driverY3.beginSerial(115200);
Serial.println("Driver Y3 Enabled\n");
driverY3.begin();
driverY3.rms_current(850); // mA
driverY3.microsteps(64);
driverY3.pwm_ofs_auto ();

driverAOAT.beginSerial(115200);
Serial.println("driver e1 enabled\n");
driverAOAT.begin();
driverAOAT.rms_current(900); // ma
driverAOAT.microsteps(64);
driverAOAT.pwm_ofs_auto ();

driverAOAB.beginSerial(115200);
Serial.println("Driver E2 Enabled\n");
driverAOAB.begin();
driverAOAB.rms_current(900); // mA
driverAOAB.microsteps(64);
driverAOAB.pwm_ofs_auto ();

}

