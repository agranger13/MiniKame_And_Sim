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
#include <algorithm>
#include <cmath>
#include <string>

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

  // Trot diagonal : pattes 1+4 en phase, pattes 2+3 en opposition de phase
  const double phase[NUM_LEGS] = {0.0, M_PI, M_PI, 0.0};

  // Main loop:
  // - perform simulation steps until Webots is stopping the controller
  while (robot->step(timeStep) != -1) {
    double t = robot->getTime();
    const double freq = 0.5;       // Hz
    const double hip_amp = 0.5;    // rad — balancement avant/arrière
    const double knee_amp = 0.5;   // rad — levée de patte

    for (int i = 0; i < NUM_LEGS; i++) {
      double phi = 2 * M_PI * freq * t + phase[i];
      double hip_angle = hip_amp * sin(phi);
      // le genou lève la patte pendant la moitié "retour" du cycle
      double knee_angle = knee_amp * std::max(0.0, sin(phi));
      hip[i]->setPosition(hip_angle);
      knee[i]->setPosition(knee_angle);
    }
  };

  // Enter here exit cleanup code.

  delete robot;
  return 0;
}
