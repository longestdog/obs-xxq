#include "LicenseCheck.h"

LicenseChecker::LicenseChecker() {
}

LicenseChecker::~LicenseChecker() {
}

bef_effect_result_t LicenseChecker::checkLicenseStatus(std::string path) {
    bef_effect_result_t ret = bef_effect_ai_check_license_base(path.c_str(), true);
    return ret;
}
