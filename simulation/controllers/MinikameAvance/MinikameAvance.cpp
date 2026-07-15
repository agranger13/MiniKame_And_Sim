// File:          MinikameAvance.cpp
// Date:
// Description:
// Author:
// Modifications:

// You may need to add webots include files such as
// <webots/DistanceSensor.hpp>, <webots/Motor.hpp>, etc.
// and/or to add some other includes
#include <webots/Robot.hpp>
#include <webots/Supervisor.hpp>
#include <webots/Motor.hpp>
#include <webots/Node.hpp>
#include "WebotsHAL.h"
#include "Kame.h"
#include "Gaits.h"
#include <string>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <sstream>

// All the webots classes are defined in the "webots" namespace
using namespace webots;

// Pont minimal vers le serveur MCP (webots-youbot-mcp/) : le contrôleur est
// un Supervisor pour pouvoir exporter des captures d'écran et piloter la
// simulation depuis Claude Code. Format JSON fixe, écrit uniquement par ce
// serveur MCP -> extraction de valeurs par recherche de sous-chaîne au lieu
// d'une dépendance JSON complète.
namespace {
const char* kCommandsPath = "../../../webots-youbot-mcp/data/commands.json";
const char* kScreenshotDir = "../../../webots-youbot-mcp/data/screenshots/";
const char* kStatusPath = "../../../webots-youbot-mcp/data/status.json";

std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream f(path, std::ios::trunc);
    f << content;
}

std::string jsonStringValue(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    size_t firstQuote = json.find('"', json.find(':', pos) + 1);
    if (firstQuote == std::string::npos) return "";
    size_t secondQuote = json.find('"', firstQuote + 1);
    if (secondQuote == std::string::npos) return "";
    return json.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

// Publie la pose réelle (position + cap) du robot, lue directement via
// l'API Supervisor -> permet de mesurer une dérive de trajectoire avec des
// chiffres plutôt qu'en comparant des captures d'écran à l'oeil.
void publishStatus(Node* selfNode, double simTime) {
    const double kPi = 3.14159265358979323846;
    const double* pos = selfNode->getPosition();
    const double* rot = selfNode->getOrientation();  // 3x3 row-major
    double yawDeg = atan2(rot[3], rot[0]) * 180.0 / kPi;
    std::ostringstream json;
    json << "{\"pos\": [" << pos[0] << ", " << pos[1] << ", " << pos[2] << "], "
         << "\"yaw_deg\": " << yawDeg << ", "
         << "\"sim_time\": " << simTime << "}";
    writeFile(kStatusPath, json.str());
}

void handleMcpCommands(Supervisor* robot, std::string& lastTimestamp) {
    std::string cmdJson = readFile(kCommandsPath);
    if (cmdJson.empty()) return;

    std::string ts = jsonStringValue(cmdJson, "timestamp");
    if (ts.empty() || ts == lastTimestamp || ts == "0") return;
    lastTimestamp = ts;

    // Effacer avant traitement : un "reload" relance le contrôleur, qui
    // relirait sinon indéfiniment la même commande.
    writeFile(kCommandsPath, "{\"action\": \"none\", \"command\": \"cleared\", \"timestamp\": \"0\"}");

    std::string action = jsonStringValue(cmdJson, "action");
    if (action == "screenshot") {
        std::string filename = jsonStringValue(cmdJson, "filename");
        robot->exportImage(std::string(kScreenshotDir) + filename + ".png", 100);
    } else if (action == "simulation") {
        std::string command = jsonStringValue(cmdJson, "command");
        if (command == "pause") robot->simulationSetMode(Supervisor::SIMULATION_MODE_PAUSE);
        else if (command == "resume") robot->simulationSetMode(Supervisor::SIMULATION_MODE_REAL_TIME);
        else if (command == "reset") robot->simulationReset();
        else if (command == "reload") robot->worldReload();
    }
}
}  // namespace

// This is the main program of your controller.
// It creates an instance of your Robot instance, launches its
// function(s) and destroys it at the end of the execution.
// Note that only one instance of Robot should be created in
// a controller program.
// The arguments of the main function can be specified by the
// "controllerArgs" field of the Robot node
int main(int argc, char **argv) {
  // create the Robot instance (Supervisor : requis pour exportImage() et le
  // pilotage de simulation depuis le pont MCP).
  Supervisor *robot = new Supervisor();

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

  std::string lastMcpTimestamp;
  Node* selfNode = robot->getSelf();
  int stepCounter = 0;

  // Main loop:
  // - perform simulation steps until Webots is stopping the controller
  while (robot->step(timeStep) != -1) {
    handleMcpCommands(robot, lastMcpTimestamp);
    if (++stepCounter % 15 == 0) {
      publishStatus(selfNode, robot->getTime());
    }
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
        double kneeAngle = gait.hipNeutralDeg + gait.legMirror[i] * kneeDeviation;
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
