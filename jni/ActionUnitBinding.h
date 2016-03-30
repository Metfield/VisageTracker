#ifndef ACTIONUNITBINDING_H_
#define ACTIONUNITBINDING_H_

#include <string>
#include <vector>

class ActionUnitBinding {

public:
    // Constructor
    ActionUnitBinding(  std::string name_,
						std::string actionUnitName_,
						bool inverted_,
						float minLimit_,
						float maxLimit_,
						float weight_,
						float filterConstant_,
    					int filterWindowSize_);

    float GetValue();

private:
    std::string name;
    std::string actionUnitName;
    bool inverted;
    float minLimit;
	float maxLimit;
    float weight;
    float value;
    float filterConstant;
    int filterWindowSize;
    std::vector<float> normalizedValueHistory;

    float FilterValue(float value);

};


#endif /* ACTIONUNITBINDING_H_ */
