#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

struct Point {
  int64_t x;
  int64_t y;
  int id;
};

struct Edge {
  bool operator<(const Edge &other) const {
    if (w != other.w)
      return w < other.w;
    if (u != other.u)
      return u < other.u;

    return v < other.v;
  }

  int64_t w;
  int u;
  int v;
};

struct QuadEdge {
  Point &Org() { return origin; }
  Point &Dest() { return Rev()->origin; }

  QuadEdge *Rev() { return rot->rot; }
  QuadEdge *Lnext() { return rot->Rev()->onext->rot; }
  QuadEdge *Oprev() { return rot->onext->rot; }

  Point origin{};
  QuadEdge *rot = nullptr;
  QuadEdge *onext = nullptr;
  bool used = false;
};

struct Dsu {
  explicit Dsu(int n) {
    p.resize(n + 1);
    sz.assign(n + 1, 1);

    std::iota(p.begin(), p.end(), 0);
  }

  int Find(int v) { return p[v] == v ? v : p[v] = Find(p[v]); }

  bool Unite(int a, int b) {
    a = Find(a);
    b = Find(b);

    if (a == b)
      return false;

    if (sz[a] < sz[b])
      std::swap(a, b);

    p[b] = a;
    sz[a] += sz[b];
    return true;
  }

  std::vector<int> p;
  std::vector<int> sz;
};

class DelaunayTriangulator {
private:
  std::deque<QuadEdge> pool;
  QuadEdge *free_list = nullptr;
  std::vector<Point> pts;

  QuadEdge *NewQuadEdge() {
    if (free_list != nullptr) {
      QuadEdge *e = free_list;
      free_list = free_list->rot;
      e->rot = nullptr;
      e->onext = nullptr;
      e->used = false;
      e->origin = Point{};
      return e;
    }
    pool.emplace_back();
    return &pool.back();
  }

  void FreeEdge(QuadEdge *e) {
    e->rot = free_list;
    free_list = e;
  }

  QuadEdge *MakeEdge(const Point &from, const Point &to) {
    QuadEdge *e1 = NewQuadEdge();
    QuadEdge *e2 = NewQuadEdge();
    QuadEdge *e3 = NewQuadEdge();
    QuadEdge *e4 = NewQuadEdge();

    e1->origin = from;
    e3->origin = to;

    e1->rot = e2;
    e2->rot = e3;
    e3->rot = e4;
    e4->rot = e1;

    e1->onext = e1;
    e2->onext = e4;
    e3->onext = e3;
    e4->onext = e2;

    return e1;
  }

  void Splice(QuadEdge *a, QuadEdge *b) {
    QuadEdge *alpha = a->onext->rot;
    QuadEdge *beta = b->onext->rot;
    std::swap(a->onext, b->onext);
    std::swap(alpha->onext, beta->onext);
  }

  QuadEdge *Connect(QuadEdge *a, QuadEdge *b) {
    QuadEdge *e = MakeEdge(a->Dest(), b->Org());
    Splice(e, a->Lnext());
    Splice(e->Rev(), b);
    return e;
  }

  void DeleteEdge(QuadEdge *e) {
    Splice(e, e->Oprev());
    Splice(e->Rev(), e->Rev()->Oprev());

    QuadEdge *p1 = e;
    QuadEdge *p2 = e->rot;
    QuadEdge *p3 = e->rot->rot;
    QuadEdge *p4 = e->rot->rot->rot;

    FreeEdge(p1);
    FreeEdge(p2);
    FreeEdge(p3);
    FreeEdge(p4);
  }

  static int64_t CrossProduct(const Point &a, const Point &b, const Point &c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
  }

  static bool InCircle(const Point &a, const Point &b, const Point &c,
                       const Point &d) {
    long double ax = static_cast<long double>(a.x) - d.x;
    long double ay = static_cast<long double>(a.y) - d.y;
    long double bx = static_cast<long double>(b.x) - d.x;
    long double by = static_cast<long double>(b.y) - d.y;
    long double cx = static_cast<long double>(c.x) - d.x;
    long double cy = static_cast<long double>(c.y) - d.y;

    long double a2 = ax * ax + ay * ay;
    long double b2 = bx * bx + by * by;
    long double c2 = cx * cx + cy * cy;

    long double det = a2 * (bx * cy - by * cx) - b2 * (ax * cy - ay * cx) +
                      c2 * (ax * by - ay * bx);

    return det > 0.0;
  }

  static bool IsLeftOf(const Point &p, QuadEdge *e) {
    return CrossProduct(e->Org(), e->Dest(), p) > 0;
  }

  static bool IsRightOf(const Point &p, QuadEdge *e) {
    return CrossProduct(e->Org(), e->Dest(), p) < 0;
  }

  std::pair<QuadEdge *, QuadEdge *> BuildDelaunayRec(int l, int r) {
    if (r - l + 1 == 2) {
      QuadEdge *a = MakeEdge(pts[l], pts[r]);
      return {a, a->Rev()};
    }

    if (r - l + 1 == 3) {
      QuadEdge *a = MakeEdge(pts[l], pts[l + 1]);
      QuadEdge *b = MakeEdge(pts[l + 1], pts[r]);
      Splice(a->Rev(), b);

      int64_t s = CrossProduct(pts[l], pts[l + 1], pts[r]);
      if (s == 0)
        return {a, b->Rev()};

      QuadEdge *c = Connect(b, a);
      if (s > 0)
        return {a, b->Rev()};
      return {c->Rev(), c};
    }

    int m = l + (r - l) / 2;
    std::pair<QuadEdge *, QuadEdge *> left_res = BuildDelaunayRec(l, m);
    std::pair<QuadEdge *, QuadEdge *> right_res = BuildDelaunayRec(m + 1, r);

    QuadEdge *ldo = left_res.first;
    QuadEdge *ldi = left_res.second;
    QuadEdge *rdi = right_res.first;
    QuadEdge *rdo = right_res.second;

    while (true) {
      if (IsLeftOf(rdi->Org(), ldi)) {
        ldi = ldi->Lnext();
      } else if (IsRightOf(ldi->Org(), rdi)) {
        rdi = rdi->Rev()->onext;
      } else {
        break;
      }
    }

    QuadEdge *basel = Connect(rdi->Rev(), ldi);
    if (ldi->Org().id == ldo->Org().id)
      ldo = basel->Rev();
    if (rdi->Org().id == rdo->Org().id)
      rdo = basel;

    auto valid = [&](QuadEdge *e) { return IsRightOf(e->Dest(), basel); };

    while (true) {
      QuadEdge *lcand = basel->Rev()->onext;
      if (valid(lcand)) {
        while (InCircle(basel->Dest(), basel->Org(), lcand->Dest(),
                        lcand->onext->Dest())) {
          QuadEdge *t = lcand->onext;
          DeleteEdge(lcand);
          lcand = t;
        }
      }

      QuadEdge *rcand = basel->Oprev();
      if (valid(rcand)) {
        while (InCircle(basel->Dest(), basel->Org(), rcand->Dest(),
                        rcand->Oprev()->Dest())) {
          QuadEdge *t = rcand->Oprev();
          DeleteEdge(rcand);
          rcand = t;
        }
      }

      bool lv = valid(lcand);
      bool rv = valid(rcand);

      if (!lv && !rv)
        break;

      if (!lv || (rv && InCircle(lcand->Dest(), lcand->Org(), rcand->Org(),
                                 rcand->Dest()))) {
        basel = Connect(rcand, basel->Rev());
      } else {
        basel = Connect(basel->Rev(), lcand->Rev());
      }
    }

    return {ldo, rdo};
  }

  std::vector<std::pair<int, int>> ExtractEdges(QuadEdge *start) {
    std::vector<std::pair<int, int>> res;
    std::vector<QuadEdge *> st;
    st.push_back(start);

    while (!st.empty()) {
      QuadEdge *e = st.back();
      st.pop_back();
      if (e->used)
        continue;

      e->used = true;
      e->Rev()->used = true;

      int u = e->Org().id;
      int v = e->Dest().id;
      if (u != v) {
        if (u > v)
          std::swap(u, v);
        res.push_back({u, v});
      }

      st.push_back(e->onext);
      st.push_back(e->Rev()->onext);
      st.push_back(e->Lnext());
    }

    std::sort(res.begin(), res.end());
    res.erase(std::unique(res.begin(), res.end()), res.end());
    return res;
  }

public:
  explicit DelaunayTriangulator(const std::vector<Point> &points)
      : pts(points) {}

  std::vector<std::pair<int, int>> Build() {
    if (pts.empty()) {
      return {};
    }
    std::pair<QuadEdge *, QuadEdge *> hull =
        BuildDelaunayRec(0, pts.size() - 1);
    return ExtractEdges(hull.first);
  }
};

int64_t SquaredDistance(const Point &a, const Point &b) {
  int64_t dx = a.x - b.x;
  int64_t dy = a.y - b.y;
  return dx * dx + dy * dy;
}

int64_t ParseScaled(const std::string &s) {
  int sign = 1;
  size_t pos = 0;
  if (s[pos] == '-') {
    sign = -1;
    ++pos;
  }

  int64_t whole = 0;
  while (pos < s.size() && s[pos] != '.') {
    whole = whole * 10 + (s[pos] - '0');
    ++pos;
  }

  int64_t frac = 0;
  int64_t pw = 100;
  if (pos < s.size() && s[pos] == '.') {
    ++pos;
    int cnt = 0;
    while (pos < s.size() && cnt < 3) {
      frac += (s[pos] - '0') * pw;
      pw /= 10;
      ++pos;
      ++cnt;
    }
  }

  return sign * (whole * 1000 + frac);
}

int main() {
  int n = 0;
  std::cin >> n;

  std::vector<Point> input(n + 1);
  for (int i = 1; i <= n; ++i) {
    std::string sx;
    std::string sy;
    std::cin >> sx >> sy;
    input[i] = {ParseScaled(sx), ParseScaled(sy), i};
  }

  if (n == 1) {
    std::cout << std::fixed << std::setprecision(15) << 0.0 << std::endl;
    return 0;
  }

  std::vector<Point> pts;
  for (int i = 1; i <= n; ++i) {
    pts.push_back(input[i]);
  }

  std::sort(pts.begin(), pts.end(), [](const Point &a, const Point &b) {
    if (a.x != b.x)
      return a.x < b.x;
    if (a.y != b.y)
      return a.y < b.y;
    return a.id < b.id;
  });

  DelaunayTriangulator triangulator(pts);
  std::vector<std::pair<int, int>> delaunay_edges = triangulator.Build();

  std::vector<Edge> edges;
  for (const std::pair<int, int> &p : delaunay_edges) {
    int u = p.first;
    int v = p.second;
    edges.push_back({SquaredDistance(input[u], input[v]), u, v});
  }

  std::sort(edges.begin(), edges.end());

  Dsu dsu(n);
  long double total = 0.0L;
  std::vector<std::pair<int, int>> ans;

  for (const Edge &e : edges) {
    if (dsu.Unite(e.u, e.v)) {
      total += std::sqrt(e.w * 1.0L) / 1000.0L;
      ans.push_back({e.u, e.v});
      if (ans.size() + 2 == input.size())
        break;
    }
  }

  std::cout << std::fixed << std::setprecision(15) << total << std::endl;
  for (const auto &p : ans) {
    std::cout << p.first << ' ' << p.second << std::endl;
  }

  return 0;
}
