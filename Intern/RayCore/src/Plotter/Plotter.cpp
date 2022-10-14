#include <Plotter/Plotter.h>

#include <algorithm>
namespace RAYX {
bool comp(Ray const& lhs, Ray const& rhs) {
    return lhs.m_extraParam < rhs.m_extraParam;
}
bool abs_comp(double const& lhs, double const& rhs) {
    return abs(lhs) < abs(rhs);
}
inline bool intclose(double x, double y) {
    return abs(x - y) < std::numeric_limits<double>::epsilon();
}
/**
 * @brief Plot the
 *
 * @param plotType Plot type, please check ENUM
 * @param plotName Plot Name
 * @param RayList Data to be plotted
 */
void Plotter::plot(int plotType, std::string plotName,
                   const std::vector<Ray>& RayList) {
    RAYX_LOG << "Plotting...";

    // Sort Data for plotting
    std::vector<Ray> correctOrderRayList;
    std::vector<double> Xpos, Ypos;
    correctOrderRayList.reserve(RayList.size());
    Xpos.reserve(RayList.size());
    Ypos.reserve(RayList.size());
    auto max = std::max_element(RayList.begin(), RayList.end(), comp);
    auto max_param = max->m_extraParam;

    // Create new RayList with right order
    for (auto r : RayList) {
        if (intclose(r.m_extraParam, max_param)) {
            correctOrderRayList.push_back(r);
            Xpos.push_back(r.m_position.x);
            Ypos.push_back(r.m_position.y);
        }
    }

    auto maxX = *(std::max_element(Xpos.begin(), Xpos.end()));
    auto maxY = *(std::max_element(Ypos.begin(), Ypos.end()));
    
    if (plotType == plotTypes::LikeRAYUI) {  // RAY-UI
        // Start plot
        matplotlibcpp::figure_size(1300, 1000);
        matplotlibcpp::suptitle(plotName);

        matplotlibcpp::subplot2grid(4, 4, 0, 0, 1, 3);
        auto bin_amount_freedman = getBinAmount(Xpos);
        matplotlibcpp::hist(Xpos, bin_amount_freedman, "#0062c3", 0.65, false,
                            {{"density", "False"}});
        // matplotlibcpp::subplots_adjust()
        matplotlibcpp::subplot2grid(4, 4, 1, 0, 3, 3);
        matplotlibcpp::scatter(Xpos, Ypos, 1,
                               {{"color", "#62c300"}, {"label", "Ray"}});
        matplotlibcpp::text(-maxX, -maxY, "Generated by RAY-X.");

        matplotlibcpp::xlabel("x / mm");
        matplotlibcpp::ylabel("y / mm");

        matplotlibcpp::legend();
        matplotlibcpp::grid(true);

        matplotlibcpp::subplot2grid(4, 4, 1, 3, 3, 1);
        bin_amount_freedman = getBinAmount(Ypos);
        matplotlibcpp::hist(
            Ypos, bin_amount_freedman, "#0062c3", 0.65, false,
            {{"density", "False"}, {"orientation", "horizontal"}});
        matplotlibcpp::title("Intensity");

        matplotlibcpp::show();
    } else
        RAYX_D_ERR << "Plot Type not supported";
}
/**
 * @brief Get the amount of bins for the histogram
 * FREEDMAN is a different method to get the "optimized" amount of bins
 *
 * @param vec Input Data Vector
 * @return int Bin Amount
 */
int Plotter::getBinAmount(std::vector<double>& vec) {
#ifdef FREEDMAN
    std::vector<double>::iterator b = vec.begin();
    std::vector<double>::iterator e = vec.end();
    std::vector<double>::iterator n3 = b;
    std::vector<double>::iterator n1 = b;
    const std::size_t q1 = 0.25 * std::distance(b, e);
    const std::size_t q3 = 0.75 * std::distance(b, e);
    std::advance(n1, q1);
    std::advance(n3, q3);

    double width = 2 * (*n3 - *n1) / std::pow(vec.size(), 1.0 / 3);
    return (int)std::ceil((*(std::max_element(vec.begin(), vec.end())) -
                           *(std::min_element(vec.begin(), vec.end()))) /
                          width);
#endif

    const double binwidth = 0.00125;
    auto xymax =
        std::max(*(std::max_element(vec.begin(), vec.end())),
                 *(std::min_element(vec.begin(), vec.end())), abs_comp);
    return (int)(xymax / binwidth) + 1;
}

}  // namespace RAYX
