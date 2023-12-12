#ifndef LICENSE_CHECK_H
#define LICENSE_CHECK_H

#include "bef_effect_ai_api.h"
#include <string>

class LicenseChecker {
public:
	LicenseChecker();
	~LicenseChecker();

public:
	bef_effect_result_t checkLicenseStatus(std::string path);
};

#endif
