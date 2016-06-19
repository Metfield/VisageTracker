#ifndef ACTIONUNITBINDING_H_
#define ACTIONUNITBINDING_H_

#include <string>
#include <vector>

class ActionUnitBinding {

public:
    std::string name;
    std::string actionUnitName;

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
    void UpdateValue(float newValue);
    float value;
private:
    bool inverted;
    float minLimit;
	float maxLimit;
    float weight;

    float filterConstant;
    int filterWindowSize;
    std::vector<float> normalizedValueHistory;

    float FilterValue(float value);

};


#endif /* ACTIONUNITBINDING_H_ */
