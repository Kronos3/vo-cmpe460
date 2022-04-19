#include "NavAlgorithm.h"
#include "Assert.hpp"
#include "opencv2/imgproc.hpp"

namespace Rpi
{
    NavSimple::NavSimple(CarController* car)
            : NavAlgorithm(car)
    {
    }

    bool NavSimple::process(CamFrame* frame, cv::Mat &image)
    {
        F32 left, right;
        EdgesFound edges;

//        EdgesFound edges = find_edges(image, left, right,
//                                      m_car->params().simple.row,
//                                      m_car->params().simple.cutoff);

        F32 turning = 0;
        char edge_mnemonic = '?';

        switch (edges)
        {
            case NONE_FOUND:
                // Stop the car
                return false;
            case LEFT_FOUND:
                turning = 1 * (1 - left);
                edge_mnemonic = 'L';
                break;
            case RIGHT_FOUND:
                turning = -1 * (1 - right);
                edge_mnemonic = 'R';
                break;
            case BOTH_FOUND:
                edge_mnemonic = 'B';
                if (left < right)
                {
                    turning = 1 * (1 - left);
                }
                else
                {
                    turning = -1 * (1 - right);
                }
                break;
            default:
                FW_ASSERT(0 && "Invalid edge case found", edges);
        }


//        F32 turning_factor = m_car->params().simple.turning * turning;
        F32 turning_factor = 0 * turning;

        std::ostringstream ss;
        ss << "Edges: " << edge_mnemonic << "\n"
           << "Left: " << left << "\n"
           << "Right: " << right << "\n"
           << "Turn: " << turning_factor;

        draw_information(image, ss.str());

        // Send the commands to the car
        m_car->steer(turning_factor);
        m_car->throttle(0, 0); // TODO(tumbar) remove

        return true;
    }

    NavSimplePid::NavSimplePid(CarController* car)
            : NavPid(car)
    {
    }


    void draw_full_line(cv::Mat& img,
                        cv::Point2f p1,
                        cv::Point2f p2,
                        const cv::Scalar &color)
    {
        cv::Point2f p, q;
        // Check if the line is a vertical line because vertical lines don't have slope
        if (p1.x != p2.x)
        {
            p.x = 0;
            q.x = (F32)img.cols;

            // Slope equation (y1 - y2) / (x1 - x2)
            F32 m = (p1.y - p2.y) / (p1.x - p2.x);

            // Line equation:  y = mx + b
            F32 b = p1.y - (m * p1.x);
            p.y = m * p.x + b;
            q.y = m * q.x + b;
        }
        else
        {
            p.x = q.x = p2.x;
            p.y = 0;
            q.y = (F32)img.rows;
        }

        cv::line(img, p, q, color, 1);
    }


    struct ParametricSpline
    {
        bool set(const std::vector<cv::Point> &points)
        {
            F64 last_t = 0;
            for (size_t i = 1; i < points.size(); i++)
            {
                F64 dx = points[i].x - points[i - 1].x;
                F64 dy = points[i].y - points[i - 1].y;

                // dt = ds
                // parameter is length of curve
//                F64 gradient = std::sqrt((dx * dx) + (dy * dy));
//                if (gradient < 0.01)
//                {
//                    // Points are too close together
//                    continue;
//                }

                t.push_back(last_t + 1);
                x.push_back(points[i].x);
                y.push_back(points[i].y);
                last_t += 1;
            }

            if (t.size() <= 3)
            {
                return false;
            }

            // setup splines for x and y coordinate
            sx.set_points(t, x);
            sy.set_points(t, y);

            return true;
        }

        bool get_center(F64 t_,
                        const std::vector<cv::Point> &other_track,
                        cv::Point &center,
                        cv::Mat &image_diagnostic) const
        {
            // Pick a line

            // Find p0 and p1
            // We are solving for a line perpendicular to the tangent
            cv::Point2d p0(sx(t_), sy(t_));
            cv::Point2d p1 = p0 + cv::Point2d(sy.deriv(1, t_), sx.deriv(1, t_));

            draw_full_line(image_diagnostic, p0, p1, cv::Scalar(255, 255, 255));

            // Move through every pair on the other_track
            // and find the line segment that will intersect the
            for (U32 i = 0; i < other_track.size() - 1; i++)
            {
                cv::Point2d solution;
                if (linear_solver(p0, p1,
                                  other_track.at(i),
                                  other_track.at(i + 1),
                                  solution))
                {
                    // Now the midpoint between this solution and p0
                    // is the center of the track
                    center = 0.5 * (p0 + solution);
                    return true;
                }
            }

            return false;
        }

        const tk::spline& getx() const { return sx;  }
        const tk::spline& gety() const { return sy;  }

        cv::Point operator()(F32 t_)
        {
            return {(I32)sx(t_), (I32)sy(t_)};
        }

        F64 dx(F32 t_)
        {
            return sx.deriv(1, t_);
        }

        static bool linear_solver(
                cv::Point2d A, cv::Point2d B,
                cv::Point2d C, cv::Point2d D,
                cv::Point2d &solution)
        {
            F64 a = B.y - A.y;
            F64 b = A.x - B.x;
            F64 c = a * (A.x) + b * (A.y);

            // Line CD represented as a2x + b2y = c2
            F64 a1 = D.y - C.y;
            F64 b1 = C.x - D.x;
            F64 c1 = a1 * (C.x) + b1 * (C.y);
            F64 det = a * b1 - a1 * b;
            if (det == 0)
            {
                // Lines are parallel
                return false;
            }
            else
            {
                F64 x_sol = (b1 * c - b * c1) / det;
                F64 y_sol = (a * c1 - a1 * c) / det;
                solution = cv::Point2d(x_sol, y_sol);
                return true;
            }
        }

        std::vector<F64> t;
        std::vector<F64> x;
        std::vector<F64> y;
        tk::spline sx;
        tk::spline sy;
    };

    bool NavSimplePid::process(CamFrame* frame, cv::Mat &image)
    {
        // Nav doesn't care about the dimensions of the image,
        // we can operate on the Vision pipeline's size
        cv::resize(image, m_resized,
                   cv::Size(image.cols / 8, image.rows / 8),
                   0, 0, cv::INTER_NEAREST // same as Vis
        );

        F32 row = m_car->params().scan.row;
        I32 center = m_resized.cols / 2;
        I32 left, right;
        I32 row_pix = (I32) (row * (F32) m_resized.rows);
        cv::Point center_pnt = cv::Point(center, row_pix);
        EdgesFound edges = find_edges(m_resized, center_pnt, left, right);

        F64 actual;
        switch(edges)
        {
            case NONE_FOUND:
                right = m_resized.cols;
                left = 0;
                break;
            case LEFT_FOUND:
                right = m_resized.cols;
                cv::drawMarker(m_resized, cv::Point(left, row_pix),
                               cv::Scalar(155, 155, 155), cv::MARKER_CROSS, 5, 1);
                break;
            case RIGHT_FOUND:
                cv::drawMarker(m_resized, cv::Point(right, row_pix),
                               cv::Scalar(155, 155, 155), cv::MARKER_CROSS, 5, 1);
                left = 0;
                break;
            case BOTH_FOUND:
                cv::drawMarker(m_resized, cv::Point(left, row_pix),
                               cv::Scalar(155, 155, 155), cv::MARKER_CROSS, 5, 1);
                cv::drawMarker(m_resized, cv::Point(right, row_pix),
                               cv::Scalar(155, 155, 155), cv::MARKER_CROSS, 5, 1);
                break;
        }

        actual = (right + left) / 2.0;

#if 0
        std::vector<cv::Point> center_line;
        for (U32 i = 0; i < left_line.size(); i++)
        {
            center_line.push_back(0.5 * (left_line.at(i) + right_line.at(i)));
        }

        // Low pass the center line
        for (U32 i = 1; i < center_line.size() - 1; i++)
        {
            center_line[i] =
                    0.25 * center_line[i - 1] +
                    0.5 * center_line[i] +
                    0.25 * center_line[i + 1];

            cv::drawMarker(m_resized, center_line[i], cv::Scalar(255, 255, 255),
                           cv::MARKER_SQUARE, 5);
        }

        cv::Point steering_point = center_line.at(2);
        cv::drawMarker(m_resized, steering_point, cv::Scalar(255, 255, 255),
                       cv::MARKER_TRIANGLE_UP, 10);
        F64 actual = (F64)steering_point.x / m_resized.cols;
#endif

#if 0
        std::vector<cv::Point> center_points;
        for (U32 i = 0; i < FW_MIN(left_line.size(), right_line.size()); i++)
        {
            cv::Point center_point = 0.5 * (left_line[i] + right_line[i]);
            center_points.push_back(center_point);
            cv::drawMarker(m_resized, center_point, cv::Scalar(255, 255, 255),
                           cv::MARKER_CROSS, 5);
        }

        F64 actual;

        ParametricSpline center_spline;
        if (center_spline.set(center_points))
        {
            cv::Point steering_point = center_spline(m_car->params().pid.steering_t);

            cv::drawMarker(m_resized, steering_point,
                           cv::Scalar(255, 255, 255),
                           cv::MARKER_TRIANGLE_UP);

//            center_spline.draw_heading(m_car->params().pid.steering_t, m_resized);
//            actual = center_spline.dx(m_car->params().pid.steering_t);
            steering_point = center
        }
        else
        {
            actual = 0.0;
        }
#endif

        F64 desired = m_resized.cols / 2.0;
        F64 error = desired - actual;

        F64 steering = pid(error);
        F64 throttle = (1.0 - error) * m_car->params().pid.t;

        std::ostringstream ss;
        ss << "Desired: " << desired << "\n"
           << "Actual: " << actual << "\n"
           << "Error: " << error << "\n"
           << "Turn: " << steering << "\n"
           << "Throttle: " << throttle << "\n";

        cv::resize(m_resized, image,
                   cv::Size(image.cols, image.rows),
                   0, 0, cv::INTER_NEAREST // same as Vis
        );

        draw_information(image, ss.str());

        m_car->steer((F32) steering);
//        m_car->throttle((F32) throttle,
//                        (F32) throttle);
        // TODO(tumbar) Dynamic throttle
        m_car->throttle(m_car->params().pid.t,
                        m_car->params().pid.t);

        return true;
    }

    template<typename T>
    void smooth_bad(std::vector<T> &line, U32 iter_n)
    {
        // Continuously smooth the line
        std::vector<T> buffer = line;

        std::vector<T>* current = &line;
        std::vector<T>* average = &buffer;

        for (U32 iter = 0; iter < iter_n; iter++)
        {
            for (U32 i = 1; i < line.size(); i++)
            {
                T p1 = current->at(i);
                T p0 = current->at(i);
                (*average)[i] = 0.5 * (p1 + p0);
            }

            // Swap the two buffers
            std::vector<T>* tmp = current;
            current = average;
            average = tmp;
        }

        // line should not have bad points anymore
    }

    void remove_bad(std::vector<cv::Point> &line,
                    F32 derivative_tolerance)
    {
        // Discontinuities are defined as points whose derivatives
        // don't match approaching from both sides
        // Matching is defined as under some threshold

        U32 size_last;
        do
        {
            size_last = line.size();
            for (U32 i = 1; i < line.size() - 1; i++)
            {
                // We are only concerned with dx because
                // we are scanning at a constant dy
                F64 top_d = cv::norm(line[i + 1] - line[i]);
                F64 bottom_d = cv::norm(line[i - 1] - line[i]);

                if (top_d > derivative_tolerance
                    && bottom_d > derivative_tolerance)
                {
                    // The current point is a bad point
                    line.erase(line.begin() + i);
                }
            }
        } while (line.size() != size_last && line.size() > 3);

        // First and last points are usually bad
        if (line.size() >= 3)
        {
            line.erase(line.begin());
            line.erase(line.end() - 1);
        }
    }

    void refine_edges(
            cv::Mat &image,
            std::vector<cv::Point> &line)
    {
        for (auto &p: line)
        {
            // Search around this point for the nearest edge
            I32 i;
            for (i = 0; i < 30; i++)
            {
                U8 left_search = image.at<U8>(p.y, p.x - i);
                U8 right_search = image.at<U8>(p.y, p.x + i);
                if (left_search > 150)
                {
                    p = cv::Point(p.x - i, p.y);
                    break;
                }
                else if (right_search > 150)
                {
                    p = cv::Point(p.x + i, p.y);
                    break;
                }
            }


        }
    }

    void NavSimplePid::find_track_edges(cv::Mat &image,
                                        std::vector<cv::Point> &left_line,
                                        std::vector<cv::Point> &right_line) const
    {
        // Scan the image by moving up along the frame and scanning for edges
        U32 max_pts = 30;
        left_line.clear();
        right_line.clear();
        left_line.reserve(max_pts);
        right_line.reserve(max_pts);

        // Find the track
        F32 row = m_car->params().scan.row;
        I32 center = image.cols / 2;
        I32 left, right;
        I32 row_pix = (I32) (row * (F32) image.rows);
        cv::Point center_pnt = cv::Point(center, row_pix);
        EdgesFound edges = find_edges(image, center_pnt, left, right);

        if (edges != BOTH_FOUND)
        {
            return;
        }

        // Draw the track lines
        cv::Point ls(left, row_pix);
        cv::Point rs(right, row_pix);
        cv::Point lp;
        cv::Point rp;
        U32 i = 0;
        I32 rc = THETA_DIVISIONS / 4;
        I32 lc = THETA_DIVISIONS / 4;
        while (trace_line(image, ls,
                          lp, m_car->params().scan.step,
                          lc) &&
               trace_line(image, rs,
                          rp, m_car->params().scan.step,
                          rc) &&
               i++ < max_pts)
        {
            left_line.push_back(lp);
            right_line.push_back(rp);
            ls = lp;
            rs = rp;
        }

        for (const auto &l: left_line)
        {
            cv::drawMarker(image, l, cv::Scalar(155, 155, 155),
                           cv::MARKER_CROSS, 5);
        }
        for (const auto &r: right_line)
        {
            cv::drawMarker(image, r, cv::Scalar(155, 155, 155),
                           cv::MARKER_DIAMOND, 5);
        }
    }
}