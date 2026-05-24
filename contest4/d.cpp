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
    if (y != other.y) return y < other.y;
    return x < other.x;
  }

  long long x;
  long long y;
};

enum class HalfPlane { kUpper = 0, kLower = 1 };

HalfPlane GetHalfPlane(const Point& direction) {
  if (direction.y != 0) {
    return (direction.y > 0) ? HalfPlane::kUpper : HalfPlane::kLower;
  }
  return (direction.x > 0) ? HalfPlane::kUpper : HalfPlane::kLower;
}

bool CompareByAngle(const Point& a, const Point& b) {
  HalfPlane half_a = GetHalfPlane(a);
  HalfPlane half_b = GetHalfPlane(b);

  if (half_a != half_b) return half_a < half_b;

  long long cross_product = a.Cross(b);
  return cross_product > 0;
}

double GetPointSegmentDistance(double point_x, double point_y, double start_x,
                               double start_y, double end_x, double end_y) {
  double segment_dx = end_x - start_x;
  double segment_dy = end_y - start_y;

  double denom = segment_dx * segment_dx + segment_dy * segment_dy;
  double projection_factor =
      ((point_x - start_x) * segment_dx + (point_y - start_y) * segment_dy) /
      denom;

  if (projection_factor < 0.0) projection_factor = 0.0;
  if (projection_factor > 1.0) projection_factor = 1.0;

  double closest_dx = (start_x + projection_factor * segment_dx) - point_x;
  double closest_dy = (start_y + projection_factor * segment_dy) - point_y;

  return std::sqrt(closest_dx * closest_dx + closest_dy * closest_dy);
}

static void ReadInput(std::vector<Point>& airport, std::vector<Point>& cloud) {
  int n, m;
  std::cin >> n >> m;
  airport.resize(n);
  cloud.resize(m);

  for (int i = 0; i < n; ++i) std::cin >> airport[i].x >> airport[i].y;
  for (int i = 0; i < m; ++i) std::cin >> cloud[i].x >> cloud[i].y;
}

static std::vector<Point> InvertPolygon(const std::vector<Point>& poly) {
  std::vector<Point> inverted(poly.size());
  for (size_t i = 0; i < poly.size(); ++i) {
    inverted[i] = Point(-poly[i].x, -poly[i].y);
  }
  return inverted;
}

static std::vector<Point> CollectEdges(const std::vector<Point>& poly) {
  std::vector<Point> edges;
  edges.reserve(poly.size());
  for (size_t i = 0; i < poly.size(); ++i) {
    edges.push_back(poly[(i + 1) % poly.size()] - poly[i]);
  }
  return edges;
}

static std::vector<Point> BuildMinkowskiSum(
    const std::vector<Point>& airport,
    const std::vector<Point>& inverted_cloud) {
  std::vector<Point> edges;
  auto a_edges = CollectEdges(airport);
  auto c_edges = CollectEdges(inverted_cloud);
  edges.reserve(a_edges.size() + c_edges.size());

  edges.insert(edges.end(), a_edges.begin(), a_edges.end());
  edges.insert(edges.end(), c_edges.begin(), c_edges.end());

  std::sort(edges.begin(), edges.end(), CompareByAngle);

  Point start_airport = *std::min_element(airport.begin(), airport.end());
  Point start_inverted_cloud =
      *std::min_element(inverted_cloud.begin(), inverted_cloud.end());

  std::vector<Point> minkowski_sum;
  minkowski_sum.reserve(edges.size() + 1);

  Point current = start_airport + start_inverted_cloud;
  minkowski_sum.push_back(current);

  for (const auto& edge : edges) {
    current = current + edge;
    minkowski_sum.push_back(current);
  }

  minkowski_sum.pop_back();
  return minkowski_sum;
}

static double MinDistanceToOrigin(const std::vector<Point>& polygon) {
  const size_t size = polygon.size();
  double min_distance = 1e18;

  for (size_t i = 0; i < size; ++i) {
    const Point& u = polygon[i];
    const Point& v = polygon[(i + 1) % size];
    double d = GetPointSegmentDistance(0.0, 0.0, u.x, u.y, v.x, v.y);
    if (d < min_distance) min_distance = d;
  }
  return min_distance;
}

static double ComputeAnswer(double min_distance) {
  double min_time_available = min_distance - 60.0;
  if (min_time_available < 0.0) min_time_available = 0.0;
  return min_time_available;
}

int main() {
  std::vector<Point> airport, cloud;
  ReadInput(airport, cloud);

  std::vector<Point> inverted_cloud = InvertPolygon(cloud);
  std::vector<Point> minkowski_sum = BuildMinkowskiSum(airport, inverted_cloud);

  double min_distance = MinDistanceToOrigin(minkowski_sum);
  double answer = ComputeAnswer(min_distance);

  std::cout << std::fixed << std::setprecision(10) << answer << '\n';
  return 0;
}
