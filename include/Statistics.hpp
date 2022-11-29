//
// Created by luca on 09/03/2022.
//

#ifndef QUSAT_STATISTICS_H
#define QUSAT_STATISTICS_H
#include <nlohmann/json.hpp>

using json = nlohmann::json;
struct Statistics {
  std::size_t                   nrOfGates            = 0U;
  std::size_t                   nrOfQubits           = 0U;
  std::size_t                   nrOfSatVars          = 0U;
  std::size_t                   nrOfGenerators       = 0U;
  std::size_t                   nrOfFunctionalConstr = 0U;
  std::size_t                   circuitDepth         = 0U;
  std::size_t                   nrOfDiffInputStates  = 0U;
  std::map<std::string, double> z3StatsMap;
  bool                          equal               = false;
  bool                          satisfiable         = false;
  std::size_t                   preprocTime         = 0U;
  std::size_t                   solvingTime         = 0U;
  std::size_t                   satConstructionTime = 0U;

  [[nodiscard]] json to_json() const {
    return json{{"numGates", nrOfGates},
                {"nrOfQubits", nrOfQubits},
                {"numSatVarsCreated", nrOfSatVars},
                {"numGenerators", nrOfGenerators},
                {"numFuncConstr", nrOfFunctionalConstr},
                {"circDepth", circuitDepth},
                {"numInputs", nrOfDiffInputStates},
                {"equivalent", equal},
                {"satisfiable", satisfiable},
                {"preprocTime", preprocTime},
                {"solvingTime", solvingTime},
                {"satConstructionTime", satConstructionTime},
                {"z3map", z3StatsMap}

    };
  }

  void from_json(const json& j) {
    j.at("numGates").get_to(nrOfGates);
    j.at("nrOfQubits").get_to(nrOfQubits);
    j.at("numSatVarsCreated").get_to(nrOfSatVars);
    j.at("numGenerators").get_to(nrOfGenerators);
    j.at("numFuncConstr").get_to(nrOfFunctionalConstr);
    j.at("circDepth").get_to(circuitDepth);
    j.at("numInputs").get_to(nrOfDiffInputStates);
    j.at("equivalent").get_to(equal);
    j.at("satisfiable").get_to(satisfiable);
    j.at("preprocTime").get_to(preprocTime);
    j.at("solvingTime").get_to(solvingTime);
    j.at("satConstructionTime").get_to(satConstructionTime);
    j.at("z3map").get_to(z3StatsMap);
  }

  [[nodiscard]] std::string toString() const {
    std::stringstream ss{};
    return this->to_json().dump(2);
  }
};
#endif // QUSAT_STATISTICS_H
