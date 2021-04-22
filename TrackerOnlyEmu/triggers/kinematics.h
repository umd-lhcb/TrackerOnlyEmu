#ifndef _KINEMATICS_
#define _KINEMATICS_

#include <TMath.h>

double phi( double PX, double PY ) { return TMath::ATan( PY / PX ); }

double theta( double PZ, double P ) { return TMath::ACos( PZ / P ); }

#endif
