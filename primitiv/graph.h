#ifndef PRIMITIV_GRAPH_H_
#define PRIMITIV_GRAPH_H_

#include <memory>
#include <vector>
#include <primitiv/function.h>
#include <primitiv/node.h>

namespace primitiv {

/**
 * Computation graph.
 */
class Graph {
  Graph(const Graph &) = delete;
  Graph(Graph &&) = delete;
  Graph &operator=(const Graph &) = delete;
  Graph &operator=(Graph &&) = delete;

public:
  Graph() = default;
  ~Graph() = default;

  /**
   * Adds a function subgraph.
   * @param func Interface of the new function.
   * @param args List of arguments. Each node should point a node in the same
   *        computation graph.
   * @return A new Node object of the resulting value.
   */
  Node add_function(
      std::unique_ptr<Function> &&func,
      const std::vector<Node> &args);

  /**
   * Dump internal graphs.
   */
  void dump() const;

private:
  struct ValueNode {
    unsigned src_func_id;
    std::vector<unsigned> sink_func_ids;
  };

  struct FunctionNode {
    std::unique_ptr<Function> func;
    std::vector<unsigned> arg_val_ids;
    unsigned ret_val_id;
  };

  std::vector<ValueNode> vals_;
  std::vector<FunctionNode> funcs_;
};

}  // namespace primitiv

#endif  // PRIMITIV_GRAPH_H_
