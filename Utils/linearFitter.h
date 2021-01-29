
#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace fitter
{
template <typename T>
void estimate_coef(std::vector<T> indep_var, std::vector<T> dep_var, float& B_1, float& B_0)
{
    float sumX = std::accumulate(indep_var.begin(), indep_var.end(), 0);

    float X_mean = sumX / (indep_var.size());
    float sumY   = std::accumulate(dep_var.begin(), dep_var.end(), 0);
    float Y_mean = sumY / (dep_var.size());
    float SS_xy  = 0;
    float SS_xx  = 0;
    int   n      = indep_var.size();
    {
        SS_xy = std::inner_product(indep_var.begin(), indep_var.end(), dep_var.begin(), 0);
        SS_xy = SS_xy - n * X_mean * Y_mean;
    }
    {
        SS_xx = std::inner_product(indep_var.begin(), indep_var.end(), indep_var.begin(), 0);
        SS_xx = SS_xx - n * X_mean * X_mean;
    }
    // std::cout << "SS_xy : " << SS_xy << std::endl;
    // std::cout << "SS_xx : " << SS_xx << std::endl;
    // std::cout << "X_mean : " << X_mean << std::endl;
    // std::cout << "Y_mean : " << Y_mean << std::endl;
    B_1 = SS_xy / SS_xx;
    B_0 = Y_mean - B_1 * X_mean;
}

template <typename T>
class Linear_Regression
{
  private:
  public:
    float b_0;
    float b_1;

    void fit(std::vector<T> datasetX, std::vector<T> datasetY) { estimate_coef<T>(datasetX, datasetY, b_1, b_0); }
};
} // namespace fitter