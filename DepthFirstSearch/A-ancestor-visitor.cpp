#include <iostream>
#include <numeric>
#include <unordered_map>
#include <vector>

template <class Iterator>
class IteratorRange {
  public:
    IteratorRange(Iterator begin, Iterator end) : begin_(begin), end_(end) {}

    Iterator begin() const {  // NOLINT
      return begin_;
    }

    Iterator end() const {  // NOLINT
      return end_;
    }

    size_t size() const {  // NOLINT
      return std::distance(begin_, end_);
    }

  private:
    Iterator begin_, end_;
};

template <class Vertex>
struct IEdge {
  Vertex source, destination;
};

template <class Vertex, class Edge>
class IGraph {
  public:
    using VertexIterator = typename std::vector<Vertex>::const_iterator;
    virtual IteratorRange<VertexIterator> GetVertices() const = 0;

    using EdgeIterator = typename std::vector<Edge>::const_iterator;
    virtual IteratorRange<EdgeIterator> GetOutgoingEdges(Vertex /* vertex */) const = 0;
};

template <class Vertex, class Edge>
class IDFSVisitor {
  public:
    virtual void InitializeVertex(Vertex /* vertex */) {}
    virtual void DiscoverVertex(Vertex /* vertex */) {}
    virtual void StartVertex(Vertex /* vertex */) {}
    virtual void ExamineEdge(const Edge& /* edge */) {}
    virtual void TreeEdge(const Edge& /* edge */) {}
    virtual void BackEdge(const Edge& /* edge */) {}
    virtual void ForwardOrCrossEdge(const Edge& /* edge */) {}
    virtual void FinishEdge(const Edge& /* edge */) {}
    virtual void FinishVertex(Vertex /* vertex */) {}
};

enum class Color {
  kWhite = 0,
  kGray = 1,
  kBlack = 2
};

template <class Vertex>
class IColormap {
  public:
    virtual Color GetColor(Vertex /* vertex */) const = 0;
    virtual void SetColor(Vertex /* vertex */, Color /* color */) = 0;
};

template <class Vertex, class Graph, class Colormap, class DFSVisitor>
void DepthFirstSearch(const Graph& graph, DFSVisitor& visitor, Vertex start = -1);

template <class Vertex, class Graph, class Colormap, class DFSVisitor>
void DepthFirstSearchVisit(const Graph& graph, Colormap& colormap, DFSVisitor& visitor, Vertex vertex);

using Vertex = int;

using Edge = IEdge<Vertex>;

class Graph : IGraph<Vertex, Edge> {
  public:
    explicit Graph(int vertex_count) {
      vertices_.resize(vertex_count);
      std::iota(vertices_.begin(), vertices_.end(), 0);
      outgoing_edges_.resize(vertex_count);
    }

    void AddEdge(const Edge& edge);

    using VertexIterator = typename std::vector<Vertex>::const_iterator;
    IteratorRange<VertexIterator> GetVertices() const override;

    using EdgeIterator = typename std::vector<Edge>::const_iterator;
    IteratorRange<EdgeIterator> GetOutgoingEdges(Vertex vertex) const override;

  private:
    std::vector<Vertex> vertices_;
    std::vector<std::vector<Edge>> outgoing_edges_;
};

class Colormap : IColormap<Vertex> {
  public:
    explicit Colormap(int vertex_count) {
      colormap_.resize(vertex_count, Color::kWhite);
    }

    Color GetColor(Vertex vertex) const override;
    void SetColor(Vertex vertex, Color color) override;

  private:
    std::vector<Color> colormap_;
};

struct TimeDFSVisitor : IDFSVisitor<Vertex, Edge> {
  explicit TimeDFSVisitor(int vertex_count) {
    time_in.resize(vertex_count);
    time_out.resize(vertex_count);
  }

  void DiscoverVertex(Vertex vertex) override;
  void FinishVertex(Vertex vertex) override;

  int timer = 0;
  std::vector<int> time_in, time_out;
};

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);

  std::freopen("ancestor.in", "r", stdin);
  std::freopen("ancestor.out", "w", stdout);

  int vertex_count;
  std::cin >> vertex_count;

  Graph graph(vertex_count);
  Vertex start = -1;
  for (Vertex vertex = 0, parent; vertex < vertex_count; ++vertex) {
    std::cin >> parent;
    if (!parent) {
      start = vertex;
    } else {
      --parent;
      graph.AddEdge({parent, vertex});
    }
  }

  TimeDFSVisitor time_dfs_visitor(vertex_count);
  DepthFirstSearch<Vertex, Graph, Colormap, TimeDFSVisitor>(graph, time_dfs_visitor, start);

  int query_count;
  std::cin >> query_count;

  while (query_count --> 0) {
    int ancestor, vertex;
    std::cin >> ancestor >> vertex;
    --ancestor, --vertex;

    if (time_dfs_visitor.time_in[ancestor] <= time_dfs_visitor.time_in[vertex] &&
        time_dfs_visitor.time_out[vertex] <= time_dfs_visitor.time_out[ancestor]) {
      std::cout << 1;
    } else {
      std::cout << 0;
    }
    std::cout << "\n";
  }

  return 0;
}

template <class Vertex, class Graph, class Colormap, class DFSVisitor>
void DepthFirstSearch(const Graph& graph, DFSVisitor& visitor, Vertex start) {
  Colormap colormap(graph.GetVertices().size());
  for (const auto& vertex : graph.GetVertices()) {
    colormap.SetColor(vertex, Color::kWhite);
    visitor.InitializeVertex(vertex);
  }
  if (start != -1) {
    visitor.StartVertex(start);
    DepthFirstSearchVisit(graph, colormap, visitor, start);
  }
  for (const auto& vertex : graph.GetVertices()) {
    if (colormap.GetColor(vertex) == Color::kWhite) {
      visitor.StartVertex(vertex);
      DepthFirstSearchVisit(graph, colormap, visitor, vertex);
    }
  }
}

template <class Vertex, class Graph, class Colormap, class DFSVisitor>
void DepthFirstSearchVisit(const Graph& graph, Colormap& colormap, DFSVisitor& visitor, Vertex vertex) {
  colormap.SetColor(vertex, Color::kGray);
  visitor.DiscoverVertex(vertex);
  for (const auto& outgoing_edge : graph.GetOutgoingEdges(vertex)) {
    visitor.ExamineEdge(outgoing_edge);
    const auto destination = outgoing_edge.destination;
    if (colormap.GetColor(destination) == Color::kWhite) {
      DepthFirstSearchVisit(graph, colormap, visitor, destination);
      visitor.TreeEdge(outgoing_edge);
    } else if (colormap.GetColor(destination) == Color::kGray) {
      visitor.BackEdge(outgoing_edge);
    } else {
      visitor.ForwardOrCrossEdge(outgoing_edge);
    }
    visitor.FinishEdge(outgoing_edge);
  }
  colormap.SetColor(vertex, Color::kBlack);
  visitor.FinishVertex(vertex);
}

void Graph::AddEdge(const Edge& edge) {
  outgoing_edges_[edge.source].push_back(edge);
}

using VertexIterator = typename std::vector<Vertex>::const_iterator;
IteratorRange<VertexIterator> Graph::GetVertices() const {
  return {vertices_.begin(), vertices_.end()};
}

using EdgeIterator = typename std::vector<Edge>::const_iterator;
IteratorRange<EdgeIterator> Graph::GetOutgoingEdges(Vertex vertex) const {
  return {outgoing_edges_[vertex].begin(), outgoing_edges_[vertex].end()};
}

Color Colormap::GetColor(Vertex vertex) const {
  return colormap_[vertex];
}

void Colormap::SetColor(Vertex vertex, Color color) {
  colormap_[vertex] = color;
}

void TimeDFSVisitor::DiscoverVertex(Vertex vertex) {
  time_in[vertex] = timer++;
}

void TimeDFSVisitor::FinishVertex(Vertex vertex) {
  time_out[vertex] = timer++;
}
