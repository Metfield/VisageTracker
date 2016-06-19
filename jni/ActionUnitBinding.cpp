#include <ActionUnitBinding.h>
#include <cmath>
#include <Logging.h>

// Constructor
ActionUnitBinding::ActionUnitBinding(
	std::string name_,
	std::string actionUnitName_,
	bool inverted_,
	float minLimit_,
	float maxLimit_,
	float weight_,
	float filterConstant_,
	int filterWindowSize_):

	name(name_),
	actionUnitName(actionUnitName_),
	inverted(inverted_),
	minLimit(minLimit_),
	maxLimit(maxLimit_),
	weight(weight_),
	filterConstant(filterConstant_),
	filterWindowSize(filterWindowSize_),
	value(0)
	{}

// Returns a normalized and filtered value for this ActionUnitBinding
float ActionUnitBinding::GetValue() {
	// Normalize value

	float normalizedValue = (value - minLimit) / (maxLimit - minLimit);
	normalizedValue = fmaxf(0, fminf(normalizedValue, 1));

	if (inverted)
	{
		normalizedValue = 1.0f - normalizedValue;
	}

	if(normalizedValueHistory.size() < filterWindowSize)
	{
		normalizedValueHistory.resize(filterWindowSize);

		for (int i = 1; i < filterWindowSize; i++)
		{
			normalizedValueHistory.push_back(normalizedValue);
		}
	}
	else
	{
		// Push back normalized history
		for (int i = 1; i < filterWindowSize; i++)
		{
			normalizedValueHistory[i - 1] = normalizedValueHistory[i];;
		}
	}

	// Add normalized value to history
	normalizedValueHistory[filterWindowSize - 1] = normalizedValue;

	// Filter value
	float filteredValue = FilterValue(normalizedValue);

	return filteredValue * weight;
}

// Returns a filtered float value
float ActionUnitBinding::FilterValue(float value) {
	// Get maximum variation
	float maxVariation = 0;
	for (int i = 0; i < filterWindowSize - 1; i++) {
		maxVariation = fmaxf(maxVariation, fabs(value - normalizedValueHistory[i]));
	}

	// Get weights
	float weights[filterWindowSize];
	for (int i = 0; i < filterWindowSize; i++) {
		weights[i] = expf(-i * filterConstant * maxVariation);
	}

	// Get sum of weights
	float weightSum = 0;
	for (int i = 0; i < filterWindowSize; i++) {
		weightSum += weights[i];
	}

	// Filter value
	float filteredValue = 0;
	for (int i = 0; i < filterWindowSize; i++) {
		filteredValue += weights[i] * normalizedValueHistory[i] / weightSum;
	}

	return filteredValue;
}

void ActionUnitBinding::UpdateValue(float newValue) {
	value = newValue;
}
