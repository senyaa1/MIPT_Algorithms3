#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

class Point {
 public:
  Point(long long initial_x = 0, long long initial_y = 0)
      : x(initial_x), y(initial_y) {}

  Point operator-(const Point& other) const {
    return {x - other.x, y - other.y};
  }

  Point operator+(const Point& other) const {
    return {x + other.x, y + other.y};
  }

  long long Cross(const Point& other) const {
    return x * other.y - y * other.x;
  }

  bool operator<(const Point& other) const {
    if (y != other.y) {
      return y < other.y;
    }
    return x < other.x;
  }

  long long x;
  long long y;
};

int GetHalfPlane(Point direction) {
  if (direction.y != 0) {
    if (direction.y > 0) {
      return 0;
    } else {
      return 1;
    }
  }
  if (direction.x > 0) {
    return 0;
  } else {
    return 1;
  }
}

bool CompareByAngle(const Point& a, const Point& b) {
  int half_a = GetHalfPlane(a);
  int half_b = GetHalfPlane(b);
  if (half_a != half_b) {
    return half_a < half_b;
  }
  long long cross_product = a.Cross(b);
  return cross_product > 0;
}

double GetPointSegmentDistance(double point_x, double point_y, double start_x,
                               double start_y, double end_x, double end_y) {
  double diff_x = end_x - start_x;
  double diff_y = end_y - start_y;
  double t = ((point_x - start_x) * diff_x + (point_y - start_y) * diff_y) /
             (diff_x * diff_x + diff_y * diff_y);
  if (t < 0.0) {
    t = 0.0;
  }
  if (t > 1.0) {
    t = 1.0;
  }
  double closest_x = start_x + t * diff_x - point_x;
  double closest_y = start_y + t * diff_y - point_y;
  return std::sqrt(closest_x * closest_x + closest_y * closest_y);
}

int main() {
  int n, m;
  std::cin >> n >> m;

  std::vector<Point> airport(n);
  std::vector<Point> cloud(m);
  for (int i = 0; i < n; ++i) {
    std::cin >> airport[i].x >> airport[i].y;
  }
  for (int i = 0; i < m; ++i) {
    std::cin >> cloud[i].x >> cloud[i].y;
  }

  std::vector<Point> inverted_cloud(m);
  for (int i = 0; i < m; ++i) {
    inverted_cloud[i] = Point(-cloud[i].x, -cloud[i].y);
  }

  std::vector<Point> edges;
  for (int i = 0; i < n; ++i) {
    edges.push_back(airport[(i + 1) % n] - airport[i]);
  }
  for (int i = 0; i < m; ++i) {
    edges.push_back(inverted_cloud[(i + 1) % m] - inverted_cloud[i]);
  }

  std::sort(edges.begin(), edges.end(), CompareByAngle);

  Point start_airport = *std::min_element(airport.begin(), airport.end());
  Point start_inverted_cloud =
      *std::min_element(inverted_cloud.begin(), inverted_cloud.end());

  std::vector<Point> minkowski_sum;
  Point current = start_airport + start_inverted_cloud;

  minkowski_sum.push_back(current);
  for (const auto& edge : edges) {
    current = current + edge;
    minkowski_sum.push_back(current);
  }
  minkowski_sum.pop_back();

  size_t size = minkowski_sum.size();
  double min_distance = 1e18;

  for (size_t i = 0; i < size; ++i) {
    const Point& u = minkowski_sum[i];
    const Point& v = minkowski_sum[(i + 1) % size];
    double distance = GetPointSegmentDistance(0.0, 0.0, u.x, u.y, v.x, v.y);
    if (distance < min_distance) {
      min_distance = distance;
    }
  }

  double answer = min_distance - 60.0;
  if (answer < 0.0) {
    answer = 0.0;
  }

  std::cout << std::fixed << std::setprecision(10) << answer << std::endl;
  return 0;
}
