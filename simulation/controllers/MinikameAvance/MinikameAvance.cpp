// File:          MinikameAvance.cpp
// Date:
// Description:
// Author:
// Modifications:

// You may need to add webots include files such as
// <webots/DistanceSensor.hpp>, <webots/Motor.hpp>, etc.
// and/or to add some other includes
#include <webots/Robot.hpp>
#include <webots/Motor.hpp>
#include "WebotsHAL.h"
#include "Kame.h"
#include "Gaits.h"
#include <string>
#include <cstdio>

// All the webots classes are defined in the "webots" namespace
using namespace webots;

// This is the main program of your controller.
// It creates an instance of your Robot instance, launches its
// function(s) and destroys it at the end of the execution.
// Note that only one instance of Robot should be created in
// a controller program.
// The arguments of the main function can be specified by the
// "controllerArgs" field of the Robot node
int main(int argc, char **argv) {
  // create the Robot instance.
  Robot *robot = new Robot();

  // get the time step of the current world.
  int timeStep = (int)robot->getBasicTimeStep();

  // You should insert a getDevice-like function in order to get the
  // instance of a device of the robot. Something like:
  //  Motor *motor = robot->getMotor("motorname");
  //  DistanceSensor *ds = robot->getDistanceSensor("dsname");
  //  ds->enable(timeStep);
  // 4 pattes, 2 servos chacune (hip + knee), numérotées 1 à 4 :
  // 1 = avant-droite, 2 = avant-gauche, 3 = arrière-droite, 4 = arrière-gauche
  const int NUM_LEGS = Kame::NUM_LEGS;
  Motor *hip[NUM_LEGS];
  Motor *knee[NUM_LEGS];
  for (int i = 0; i < NUM_LEGS; i++) {
    std::string hip_name = "hip_motor" + std::to_string(i + 1);
    std::string knee_name = "knee_motor" + std::to_string(i + 1);
    hip[i] = robot->getMotor(hip_name.c_str());
    knee[i] = robot->getMotor(knee_name.c_str());
  }

  const int NUM_MOTORS = Kame::NUM_MOTORS;
  Motor* motors[NUM_MOTORS];
  for (int i = 0; i < NUM_LEGS; i++) {
    motors[i] = hip[i];
    motors[NUM_LEGS + i] = knee[i];
  }
  WebotsHAL hal(robot, motors, NUM_MOTORS, timeStep);

  GaitParams gait = waveGait();
  Kame kame(hal);
  kame.setGait(gait);

  // Calibration terminée (angle debout validé entre 40 et 50°) — repasser à
  // true pour retester une nouvelle plage si besoin.
  const bool calibrationMode = false;

  // Main loop:
  // - perform simulation steps until Webots is stopping the controller
  while (robot->step(timeStep) != -1) {
    if (calibrationMode) {
      // Paliers fixes plutôt qu'un balayage continu : le genou tient chaque
      // angle 2s, valeur affichée dans la console Webots, pour repérer
      // précisément l'angle qui tient debout (le balayage continu passait
      // devant trop vite pour l'observer).
      const double stepDeg = 10.0;
      const double holdSec = 2.0;
      const int numSteps = (int)(gait.hipNeutralDeg / stepDeg) + 1; // 90,80,...,0
      double t = robot->getTime();
      int stepIndex = ((int)(t / holdSec)) % numSteps;
      static int lastStepIndex = -1;
      double kneeDeviation = -stepIndex * stepDeg;
      if (stepIndex != lastStepIndex) {
        printf("Calibration : genou cible = %.0f deg\n", gait.hipNeutralDeg + kneeDeviation);
        lastStepIndex = stepIndex;
      }
      for (int i = 0; i < NUM_LEGS; i++) {
        hal.setServo(i, gait.hipNeutralDeg);   // hanches figées
        double kneeAngle = gait.hipNeutralDeg + gait.kneeMirror[i] * kneeDeviation;
        hal.setServo(NUM_LEGS + i, kneeAngle); // genoux par paliers
      }
    } else {
      kame.update();
    }
  };

  // Enter here exit cleanup code.
  delete robot;
  return 0;
}
