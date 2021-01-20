
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace fitter
{
void estimate_coef(std::vector<float> indep_var, std::vector<float> dep_var, float& B_1, float& B_0);
void estimate_coef(std::vector<int> indep_var, std::vector<int> dep_var, float& B_1, float& B_0);

class Linear_Regression
{
  private:
  public:
    float b_0;
    float b_1;
    Linear_Regression();
    void fit(std::vector<std::vector<float>> dataset);

    void fit(std::vector<std::vector<int>> dataset);
};
} // namespace fitter
