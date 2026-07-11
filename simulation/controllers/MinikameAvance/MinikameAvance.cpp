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
#include "Oscillator/Oscillator.h"
#include <string>
#include <cmath>
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
  const int NUM_LEGS = 4;
  Motor *hip[NUM_LEGS];
  Motor *knee[NUM_LEGS];
  for (int i = 0; i < NUM_LEGS; i++) {
    std::string hip_name = "hip_motor" + std::to_string(i + 1);
    std::string knee_name = "knee_motor" + std::to_string(i + 1);
    hip[i] = robot->getMotor(hip_name.c_str());
    knee[i] = robot->getMotor(knee_name.c_str());
  }

  const int NUM_MOTORS = NUM_LEGS * 2;
  Motor* motors[NUM_MOTORS];
  for (int i = 0; i < NUM_LEGS; i++) {
    motors[i] = hip[i];
    motors[NUM_LEGS + i] = knee[i];
  }
  WebotsHAL hal(robot, motors, NUM_MOTORS, timeStep);

  // Trot diagonal : pattes 1+4 en phase, pattes 2+3 en opposition de phase
  const int phaseDeg[NUM_LEGS] = {0, 180, 180, 0};
  const int period = 2000;         // ms (0.5 Hz)
  const int hip_amp_deg = 29;      // ~0.5 rad — balancement avant/arrière
  const int neutral_deg = 90;      // convention HAL : 90° = neutre (hanches)
  const int knee_neutral_deg = 45; // position "debout" validée en calibration (tient entre 40 et 50°)
  const int knee_amp_deg = 15;     // pliure autour de la position debout (30° à 60°)

  // Pattes 1 et 3 (droite) montées en miroir des pattes 2 et 4 (gauche) dans
  // MiniKame.wbt — confirmé visuellement en calibration (1 et 3 pliaient le
  // genou dans le mauvais sens). On inverse le sens de pliure pour ces deux-là.
  const int kneeMirror[NUM_LEGS] = {-1, 1, -1, 1}; // legs 1,2,3,4

  // un Oscillator par servo, motors[0..3] = hips, motors[4..7] = knees
  Oscillator* osc[NUM_MOTORS];
  for (int i = 0; i < NUM_LEGS; i++) {
    osc[i] = new Oscillator(hal);
    osc[i]->setPeriod(period);
    osc[i]->setAmplitude(hip_amp_deg);
    osc[i]->setPhase(phaseDeg[i]);
    osc[i]->setOffset(neutral_deg);
    osc[i]->start();

    osc[NUM_LEGS + i] = new Oscillator(hal);
    osc[NUM_LEGS + i]->setPeriod(period);
    osc[NUM_LEGS + i]->setAmplitude(knee_amp_deg);
    osc[NUM_LEGS + i]->setPhase(phaseDeg[i]);
    osc[NUM_LEGS + i]->setOffset(knee_neutral_deg);
    osc[NUM_LEGS + i]->start();
  }

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
      const int numSteps = (int)(neutral_deg / stepDeg) + 1; // 90,80,...,0
      double t = robot->getTime();
      int stepIndex = ((int)(t / holdSec)) % numSteps;
      static int lastStepIndex = -1;
      double kneeDeviation = -stepIndex * stepDeg;
      if (stepIndex != lastStepIndex) {
        printf("Calibration : genou cible = %.0f deg\n", neutral_deg + kneeDeviation);
        lastStepIndex = stepIndex;
      }
      for (int i = 0; i < NUM_LEGS; i++) {
        hal.setServo(i, neutral_deg);         // hanches figées
        double kneeAngle = neutral_deg + kneeMirror[i] * kneeDeviation;
        hal.setServo(NUM_LEGS + i, kneeAngle); // genoux par paliers
      }
    } else {
      // hanches : oscillation libre autour du neutre
      for (int i = 0; i < NUM_LEGS; i++) {
        hal.setServo(i, osc[i]->refresh());
      }
      // genoux : le miroir droite/gauche se reflète autour de 90° (le centre
      // mécanique réel du joint, pas autour de knee_neutral_deg) — même
      // transformation que celle validée en calibration.
      for (int i = 0; i < NUM_LEGS; i++) {
        double kneeAngle = osc[NUM_LEGS + i]->refresh();
        if (kneeMirror[i] < 0) kneeAngle = 2 * neutral_deg - kneeAngle;
        hal.setServo(NUM_LEGS + i, kneeAngle);
      }
    }
  };

  // Enter here exit cleanup code.
  for (int i = 0; i < NUM_MOTORS; i++) {
    delete osc[i];
  }
  delete robot;
  return 0;
}
